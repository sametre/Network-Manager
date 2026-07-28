#include <winsock2.h>
#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "config.h"
#include "network.h"
#include "resource.h"
#include "utils.h"
#include "device.h"
#include "license.h"
#include "network_report.h"

#ifdef _MSC_VER
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Shell32.lib")
#endif

#define APP_CLASS_NAME L"PcPosNetworkManagerWindow"
#define APP_TITLE L"PC • POS Network Manager"
#define APP_VERSION L"1.0.0"

#define WINDOW_WIDTH 900
#define WINDOW_HEIGHT 720

#define IDC_TITLE 1001
#define IDC_SUBTITLE 1002
#define IDC_ADAPTER 1010
#define IDC_REFRESH 1011
#define IDC_ADAPTER_INFO 1012
#define IDC_PC_IP 1020
#define IDC_POS_IP 1021
#define IDC_MASK 1022
#define IDC_GATEWAY 1023
#define IDC_DNS1 1024
#define IDC_DNS2 1025
#define IDC_AUTO_TEST 1026
#define IDC_APPLY_STATIC 1030
#define IDC_ENABLE_DHCP 1031
#define IDC_TEST_POS 1032
#define IDC_NETWORK_SETTINGS 1033
#define IDC_STATUS 1040
#define IDC_LOG 1041
#define IDC_OPEN_LOG 1042
#define IDC_FOOTER 1050

#define WM_APP_TASK_DONE (WM_APP + 10)

typedef enum TaskAction {
    TASK_APPLY_STATIC = 1,
    TASK_ENABLE_DHCP = 2,
    TASK_PING_POS = 3
} TaskAction;

typedef struct AppState {
    HWND window;
    HWND adapter_combo;
    HWND adapter_info;
    HWND pc_ip;
    HWND pos_ip;
    HWND mask;
    HWND gateway;
    HWND dns1;
    HWND dns2;
    HWND auto_test;
    HWND apply_button;
    HWND dhcp_button;
    HWND ping_button;
    HWND refresh_button;
    HWND status;
    HWND status_bar;
    HWND scroll_bar;
    HWND log_box;
    HWND licenses_text;
    HFONT font_normal;
    HFONT font_title;
    HFONT font_small;
    HBRUSH background_brush;
    HBRUSH panel_brush;
    AppConfig config;
    DeviceInfo device_info;
    LicenseInfo license_info;
    AdapterInfo adapters[MAX_ADAPTERS];
    size_t adapter_count;
    wchar_t app_directory[MAX_PATH];
    wchar_t config_path[MAX_PATH];
    wchar_t logs_directory[MAX_PATH];
    wchar_t log_path[MAX_PATH];
    BOOL task_running;
    int scroll_position;
} AppState;

typedef struct TaskContext {
    HWND target_window;
    TaskAction action;
    AppConfig config;
} TaskContext;

typedef struct TaskFinished {
    TaskAction action;
    NetworkOperationResult result;
    BOOL auto_test_performed;
    BOOL auto_test_success;
} TaskFinished;

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param);

static HWND create_label(HWND parent, int id, const wchar_t *text,
                         int x, int y, int width, int height, DWORD style)
{
    return CreateWindowExW(
        0, L"STATIC", text,
        WS_CHILD | WS_VISIBLE | style,
        x, y, width, height,
        parent, (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL
    );
}

static HWND create_edit(HWND parent, int id, const wchar_t *text,
                        int x, int y, int width, int height)
{
    HWND edit = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", text,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
        x, y, width, height,
        parent, (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL
    );
    SendMessageW(edit, EM_SETLIMITTEXT, CONFIG_VALUE_LEN - 1, 0);
    return edit;
}

static HWND create_button(HWND parent, int id, const wchar_t *text,
                          int x, int y, int width, int height)
{
    return CreateWindowExW(
        0, L"BUTTON", text,
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
        x, y, width, height,
        parent, (HMENU)(INT_PTR)id, GetModuleHandleW(NULL), NULL
    );
}

static void set_control_font(HWND control, HFONT font)
{
    if (control != NULL && font != NULL) {
        SendMessageW(control, WM_SETFONT, (WPARAM)font, TRUE);
    }
}

static void append_ui_log(AppState *state, const wchar_t *text)
{
    SYSTEMTIME time_value;
    wchar_t prefix[64];
    wchar_t message[OPERATION_DETAILS_LEN + 1024];
    int length;

    if (state == NULL || state->log_box == NULL || text == NULL) {
        return;
    }

    GetLocalTime(&time_value);
    swprintf(prefix, sizeof(prefix) / sizeof(prefix[0]),
             L"[%02u:%02u:%02u] ",
             time_value.wHour, time_value.wMinute, time_value.wSecond);

    message[0] = L'\0';
    util_append_text(message, sizeof(message) / sizeof(message[0]), prefix);
    util_append_text(message, sizeof(message) / sizeof(message[0]), text);
    util_append_text(message, sizeof(message) / sizeof(message[0]), L"\r\n");

    length = GetWindowTextLengthW(state->log_box);
    SendMessageW(state->log_box, EM_SETSEL, length, length);
    SendMessageW(state->log_box, EM_REPLACESEL, FALSE, (LPARAM)message);
    SendMessageW(state->log_box, EM_SCROLLCARET, 0, 0);
}

static void set_status(AppState *state, const wchar_t *text)
{
    if (state != NULL && state->status != NULL) {
        SetWindowTextW(state->status, text != NULL ? text : L"");
        InvalidateRect(state->status, NULL, TRUE);
    }
}

static void enable_action_controls(AppState *state, BOOL enabled)
{
    if (state == NULL) {
        return;
    }

    EnableWindow(state->apply_button, enabled);
    EnableWindow(state->dhcp_button, enabled);
    EnableWindow(state->ping_button, enabled);
    EnableWindow(state->refresh_button, enabled);
    EnableWindow(state->adapter_combo, enabled);
    state->task_running = !enabled;
}

static void update_adapter_info(AppState *state)
{
    int selected;
    LRESULT item_data;
    AdapterInfo *adapter;
    wchar_t text[ADAPTER_INFO_TEXT_LEN];

    if (state == NULL || state->adapter_combo == NULL) {
        return;
    }

    selected = (int)SendMessageW(state->adapter_combo, CB_GETCURSEL, 0, 0);
    if (selected == CB_ERR) {
        SetWindowTextW(state->adapter_info, L"Ağ adaptörü seçilmedi.");
        return;
    }

    item_data = SendMessageW(state->adapter_combo, CB_GETITEMDATA, selected, 0);
    if (item_data == CB_ERR || (size_t)item_data >= state->adapter_count) {
        SetWindowTextW(state->adapter_info, L"Adaptör bilgisi okunamadı.");
        return;
    }

    adapter = &state->adapters[(size_t)item_data];
    swprintf(text, sizeof(text) / sizeof(text[0]),
             L"Durum: %ls    IPv4: %ls    MAC: %ls",
             adapter->is_up ? L"Bağlı" : L"Bağlı değil",
             adapter->ipv4,
             adapter->mac);
    SetWindowTextW(state->adapter_info, text);
}

static void refresh_adapters(AppState *state)
{
    size_t index;
    int selected_index = -1;
    int first_connected = -1;

    if (state == NULL || state->adapter_combo == NULL) {
        return;
    }

    SendMessageW(state->adapter_combo, CB_RESETCONTENT, 0, 0);
    state->adapter_count = network_enumerate_adapters(state->adapters, MAX_ADAPTERS);

    for (index = 0; index < state->adapter_count; ++index) {
        wchar_t display[512];
        int combo_index;

        swprintf(display, sizeof(display) / sizeof(display[0]),
                 L"%ls  —  %ls%ls",
                 state->adapters[index].friendly_name,
                 state->adapters[index].ipv4,
                 state->adapters[index].is_up ? L"" : L"  (Bağlı değil)");

        combo_index = (int)SendMessageW(state->adapter_combo, CB_ADDSTRING, 0, (LPARAM)display);
        if (combo_index != CB_ERR && combo_index != CB_ERRSPACE) {
            SendMessageW(state->adapter_combo, CB_SETITEMDATA, combo_index, (LPARAM)index);

            if (state->adapters[index].is_up && first_connected < 0) {
                first_connected = combo_index;
            }
            if (state->config.adapter[0] != L'\0' &&
                wcscmp(state->adapters[index].friendly_name, state->config.adapter) == 0) {
                selected_index = combo_index;
            }
        }
    }

    if (selected_index < 0) {
        selected_index = first_connected >= 0 ? first_connected : (state->adapter_count > 0 ? 0 : -1);
    }

    if (selected_index >= 0) {
        SendMessageW(state->adapter_combo, CB_SETCURSEL, selected_index, 0);
        update_adapter_info(state);
        set_status(state, L"Ağ adaptörleri hazır.");
    } else {
        SetWindowTextW(state->adapter_info, L"Kullanılabilir Ethernet veya Wi-Fi adaptörü bulunamadı.");
        set_status(state, L"Ağ adaptörü bulunamadı.");
    }
}

static BOOL collect_config(AppState *state, AppConfig *config)
{
    int selected;
    LRESULT item_data;

    if (state == NULL || config == NULL) {
        return FALSE;
    }

    ZeroMemory(config, sizeof(*config));

    selected = (int)SendMessageW(state->adapter_combo, CB_GETCURSEL, 0, 0);
    if (selected == CB_ERR) {
        MessageBoxW(state->window, L"Önce bir ağ adaptörü seçmelisin.", APP_TITLE,
                    MB_OK | MB_ICONWARNING);
        return FALSE;
    }

    item_data = SendMessageW(state->adapter_combo, CB_GETITEMDATA, selected, 0);
    if (item_data == CB_ERR || (size_t)item_data >= state->adapter_count) {
        MessageBoxW(state->window, L"Seçilen ağ adaptörü okunamadı.", APP_TITLE,
                    MB_OK | MB_ICONERROR);
        return FALSE;
    }

    util_copy_text(config->adapter, ADAPTER_NAME_LEN,
                   state->adapters[(size_t)item_data].friendly_name);
    GetWindowTextW(state->pc_ip, config->pc_ip, CONFIG_VALUE_LEN);
    GetWindowTextW(state->pos_ip, config->pos_ip, CONFIG_VALUE_LEN);
    GetWindowTextW(state->mask, config->subnet_mask, CONFIG_VALUE_LEN);
    GetWindowTextW(state->gateway, config->gateway, CONFIG_VALUE_LEN);
    GetWindowTextW(state->dns1, config->dns_primary, CONFIG_VALUE_LEN);
    GetWindowTextW(state->dns2, config->dns_secondary, CONFIG_VALUE_LEN);
    config->auto_test = SendMessageW(state->auto_test, BM_GETCHECK, 0, 0) == BST_CHECKED;

    if (!util_is_safe_adapter_name(config->adapter)) {
        MessageBoxW(state->window, L"Ağ adaptörü adı güvenli biçimde işlenemiyor.", APP_TITLE,
                    MB_OK | MB_ICONERROR);
        return FALSE;
    }

    if (!util_is_valid_ipv4(config->pc_ip) ||
        !util_is_valid_ipv4(config->pos_ip) ||
        !util_is_valid_ipv4(config->subnet_mask) ||
        !util_is_valid_ipv4(config->gateway) ||
        !util_is_valid_ipv4(config->dns_primary) ||
        (config->dns_secondary[0] != L'\0' && !util_is_valid_ipv4(config->dns_secondary))) {
        MessageBoxW(state->window,
                    L"IP profilindeki alanlardan biri geçerli bir IPv4 adresi değil.",
                    APP_TITLE, MB_OK | MB_ICONWARNING);
        return FALSE;
    }

    if (wcscmp(config->pc_ip, config->pos_ip) == 0) {
        MessageBoxW(state->window,
                    L"Bilgisayar ve POS cihazı aynı IP adresini kullanamaz.",
                    APP_TITLE, MB_OK | MB_ICONWARNING);
        return FALSE;
    }

    if (!util_same_subnet(config->pc_ip, config->gateway, config->subnet_mask)) {
        MessageBoxW(state->window,
                    L"Bilgisayar IP adresi ile modem ağ geçidi aynı alt ağda değil.",
                    APP_TITLE, MB_OK | MB_ICONWARNING);
        return FALSE;
    }

    if (!util_same_subnet(config->pc_ip, config->pos_ip, config->subnet_mask)) {
        MessageBoxW(state->window,
                    L"Bilgisayar ve POS IP adresleri aynı alt ağda değil.",
                    APP_TITLE, MB_OK | MB_ICONWARNING);
        return FALSE;
    }

    return TRUE;
}

static DWORD WINAPI task_thread_proc(LPVOID parameter)
{
    TaskContext *context = (TaskContext *)parameter;
    TaskFinished *finished;

    if (context == NULL) {
        return 1;
    }

    finished = (TaskFinished *)calloc(1, sizeof(*finished));
    if (finished == NULL) {
        free(context);
        return 1;
    }

    finished->action = context->action;

    switch (context->action) {
        case TASK_APPLY_STATIC:
            network_apply_static(&context->config, &finished->result);
            if (finished->result.success && context->config.auto_test) {
                NetworkOperationResult ping_result;
                finished->auto_test_performed = TRUE;
                finished->auto_test_success = network_ping_pos(&context->config, &ping_result);
                util_append_text(finished->result.details, OPERATION_DETAILS_LEN,
                                 L"\r\n=== Otomatik POS testi ===\r\n");
                util_append_text(finished->result.details, OPERATION_DETAILS_LEN,
                                 ping_result.details);
                util_append_text(finished->result.details, OPERATION_DETAILS_LEN,
                                 ping_result.summary);
                util_append_text(finished->result.details, OPERATION_DETAILS_LEN, L"\r\n");

                if (!finished->auto_test_success) {
                    util_append_text(finished->result.summary,
                                     sizeof(finished->result.summary) /
                                         sizeof(finished->result.summary[0]),
                                     L" POS cihazı henüz yanıt vermedi.");
                }
            }
            break;
        case TASK_ENABLE_DHCP:
            network_enable_dhcp(&context->config, &finished->result);
            break;
        case TASK_PING_POS:
            network_ping_pos(&context->config, &finished->result);
            break;
        default:
            util_copy_text(finished->result.summary,
                           sizeof(finished->result.summary) /
                               sizeof(finished->result.summary[0]),
                           L"Bilinmeyen işlem.");
            break;
    }

    PostMessageW(context->target_window, WM_APP_TASK_DONE, 0, (LPARAM)finished);
    free(context);
    return 0;
}

static BOOL start_task(AppState *state, TaskAction action)
{
    AppConfig config;
    TaskContext *context;
    HANDLE thread;

    if (state == NULL || state->task_running) {
        return FALSE;
    }

    if (!collect_config(state, &config)) {
        return FALSE;
    }

    state->config = config;
    config_save(&state->config, state->config_path);

    context = (TaskContext *)calloc(1, sizeof(*context));
    if (context == NULL) {
        MessageBoxW(state->window, L"İşlem için bellek ayrılamadı.", APP_TITLE,
                    MB_OK | MB_ICONERROR);
        return FALSE;
    }

    context->target_window = state->window;
    context->action = action;
    context->config = config;

    enable_action_controls(state, FALSE);
    set_status(state, L"İşlem yürütülüyor…");

    switch (action) {
        case TASK_APPLY_STATIC:
            append_ui_log(state, L"Sabit IP profili uygulanıyor.");
            break;
        case TASK_ENABLE_DHCP:
            append_ui_log(state, L"Ağ adaptörü DHCP moduna alınıyor.");
            break;
        case TASK_PING_POS:
            append_ui_log(state, L"POS bağlantı testi başlatıldı.");
            break;
        default:
            break;
    }

    thread = CreateThread(NULL, 0, task_thread_proc, context, 0, NULL);
    if (thread == NULL) {
        free(context);
        enable_action_controls(state, TRUE);
        set_status(state, L"İşlem başlatılamadı.");
        MessageBoxW(state->window, L"Arka plan işlemi başlatılamadı.", APP_TITLE,
                    MB_OK | MB_ICONERROR);
        return FALSE;
    }

    CloseHandle(thread);
    return TRUE;
}

static void handle_task_finished(AppState *state, TaskFinished *finished)
{
    wchar_t log_entry[OPERATION_DETAILS_LEN + 1024];

    if (state == NULL || finished == NULL) {
        free(finished);
        return;
    }

    enable_action_controls(state, TRUE);
    set_status(state, finished->result.summary);

    log_entry[0] = L'\0';
    util_append_text(log_entry, sizeof(log_entry) / sizeof(log_entry[0]),
                     finished->result.success ? L"BAŞARILI: " : L"HATA: ");
    util_append_text(log_entry, sizeof(log_entry) / sizeof(log_entry[0]),
                     finished->result.summary);
    append_ui_log(state, log_entry);

    if (finished->result.details[0] != L'\0') {
        append_ui_log(state, finished->result.details);
    }

    util_append_log_file(state->log_path, log_entry);
    if (finished->result.details[0] != L'\0') {
        util_append_log_file(state->log_path, finished->result.details);
    }

    if (finished->result.success) {
        MessageBoxW(state->window, finished->result.summary, APP_TITLE,
                    MB_OK | MB_ICONINFORMATION);
    } else {
        MessageBoxW(state->window, finished->result.summary, APP_TITLE,
                    MB_OK | MB_ICONERROR);
    }

    refresh_adapters(state);
    free(finished);
}

static void open_network_settings(HWND owner)
{
    HINSTANCE result = ShellExecuteW(owner, L"open", L"ms-settings:network-status",
                                     NULL, NULL, SW_SHOWNORMAL);
    if ((INT_PTR)result <= 32) {
        ShellExecuteW(owner, L"open", L"control.exe", L"ncpa.cpl", NULL, SW_SHOWNORMAL);
    }
}

static void create_application_controls(AppState *state)
{
    HWND control;
    int left = 28;
    int right_column = 458;
    int label_width = 120;
    int edit_width = 245;
    int row1 = 222;
    int row2 = 270;
    int row3 = 318;

    control = create_label(state->window, IDC_TITLE, APP_TITLE,
                           28, 20, 620, 38, SS_LEFT);
    set_control_font(control, state->font_title);

    control = create_label(state->window, IDC_SUBTITLE,
                           L"Bilgisayar ve POS cihazı için güvenli, kalıcı IPv4 profil yönetimi",
                           30, 61, 760, 24, SS_LEFT);
    set_control_font(control, state->font_small);

    control = create_label(state->window, 0, L"Ağ adaptörü",
                           30, 112, 120, 24, SS_LEFT);
    set_control_font(control, state->font_normal);

    state->adapter_combo = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
        150, 106, 590, 300,
        state->window, (HMENU)(INT_PTR)IDC_ADAPTER, GetModuleHandleW(NULL), NULL
    );
    set_control_font(state->adapter_combo, state->font_normal);

    state->refresh_button = create_button(state->window, IDC_REFRESH, L"Yenile",
                                          756, 105, 110, 32);
    set_control_font(state->refresh_button, state->font_normal);

    state->adapter_info = create_label(state->window, IDC_ADAPTER_INFO,
                                       L"Adaptör bilgileri yükleniyor…",
                                       150, 148, 716, 26, SS_LEFT);
    set_control_font(state->adapter_info, state->font_small);

    control = create_label(state->window, 0, L"IP PROFİLİ",
                           30, 184, 300, 25, SS_LEFT);
    set_control_font(control, state->font_normal);

    control = create_label(state->window, 0, L"Bilgisayar IP",
                           left, row1 + 5, label_width, 24, SS_LEFT);
    set_control_font(control, state->font_normal);
    state->pc_ip = create_edit(state->window, IDC_PC_IP, state->config.pc_ip,
                               left + label_width, row1, edit_width, 31);
    set_control_font(state->pc_ip, state->font_normal);

    control = create_label(state->window, 0, L"POS IP",
                           right_column, row1 + 5, label_width, 24, SS_LEFT);
    set_control_font(control, state->font_normal);
    state->pos_ip = create_edit(state->window, IDC_POS_IP, state->config.pos_ip,
                                right_column + label_width, row1, edit_width, 31);
    set_control_font(state->pos_ip, state->font_normal);

    control = create_label(state->window, 0, L"Alt ağ maskesi",
                           left, row2 + 5, label_width, 24, SS_LEFT);
    set_control_font(control, state->font_normal);
    state->mask = create_edit(state->window, IDC_MASK, state->config.subnet_mask,
                              left + label_width, row2, edit_width, 31);
    set_control_font(state->mask, state->font_normal);

    control = create_label(state->window, 0, L"Modem / Gateway",
                           right_column, row2 + 5, label_width, 24, SS_LEFT);
    set_control_font(control, state->font_normal);
    state->gateway = create_edit(state->window, IDC_GATEWAY, state->config.gateway,
                                 right_column + label_width, row2, edit_width, 31);
    set_control_font(state->gateway, state->font_normal);

    control = create_label(state->window, 0, L"Birincil DNS",
                           left, row3 + 5, label_width, 24, SS_LEFT);
    set_control_font(control, state->font_normal);
    state->dns1 = create_edit(state->window, IDC_DNS1, state->config.dns_primary,
                              left + label_width, row3, edit_width, 31);
    set_control_font(state->dns1, state->font_normal);

    control = create_label(state->window, 0, L"İkincil DNS",
                           right_column, row3 + 5, label_width, 24, SS_LEFT);
    set_control_font(control, state->font_normal);
    state->dns2 = create_edit(state->window, IDC_DNS2, state->config.dns_secondary,
                              right_column + label_width, row3, edit_width, 31);
    set_control_font(state->dns2, state->font_normal);

    state->auto_test = CreateWindowExW(
        0, L"BUTTON", L"Sabit IP uygulandıktan sonra POS bağlantısını otomatik test et",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTOCHECKBOX,
        148, 366, 520, 28,
        state->window, (HMENU)(INT_PTR)IDC_AUTO_TEST, GetModuleHandleW(NULL), NULL
    );
    set_control_font(state->auto_test, state->font_small);
    SendMessageW(state->auto_test, BM_SETCHECK,
                 state->config.auto_test ? BST_CHECKED : BST_UNCHECKED, 0);

    state->apply_button = create_button(state->window, IDC_APPLY_STATIC,
                                        L"Sabit IP Uygula", 28, 410, 205, 42);
    state->dhcp_button = create_button(state->window, IDC_ENABLE_DHCP,
                                       L"DHCP'ye Geri Dön", 245, 410, 205, 42);
    state->ping_button = create_button(state->window, IDC_TEST_POS,
                                       L"POS Bağlantısını Test Et", 462, 410, 230, 42);
    control = create_button(state->window, IDC_NETWORK_SETTINGS,
                            L"Windows Ağ Ayarları", 704, 410, 162, 42);

    set_control_font(state->apply_button, state->font_normal);
    set_control_font(state->dhcp_button, state->font_normal);
    set_control_font(state->ping_button, state->font_normal);
    set_control_font(control, state->font_small);

    state->status = create_label(state->window, IDC_STATUS, L"Hazır.",
                                 30, 470, 720, 28, SS_LEFT);
    set_control_font(state->status, state->font_normal);

    control = create_label(state->window, 0, L"İŞLEM GÜNLÜĞÜ",
                           30, 510, 260, 24, SS_LEFT);
    set_control_font(control, state->font_normal);

    control = create_button(state->window, IDC_OPEN_LOG, L"Log Dosyasını Aç",
                            706, 503, 160, 30);
    set_control_font(control, state->font_small);

    state->log_box = CreateWindowExW(
        WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL |
            ES_READONLY | ES_LEFT,
        28, 540, 838, 112,
        state->window, (HMENU)(INT_PTR)IDC_LOG, GetModuleHandleW(NULL), NULL
    );
    set_control_font(state->log_box, state->font_small);

    control = create_label(state->window, IDC_FOOTER,
                           L"PC • POS Network Manager v" APP_VERSION
                           L"   |   POS IP'sini cihazdan veya modem DHCP rezervasyonundan ayrıca sabitleyin.",
                           30, 670, 836, 22, SS_LEFT);
    set_control_font(control, state->font_small);
}

static BOOL initialize_state(HWND window, AppState *state)
{
    if (state == NULL) {
        return FALSE;
    }

    ZeroMemory(state, sizeof(*state));
    state->window = window;
    state->background_brush = CreateSolidBrush(RGB(247, 249, 252));
    state->panel_brush = CreateSolidBrush(RGB(255, 255, 255));
    state->font_normal = CreateFontW(
        -17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );
    state->font_title = CreateFontW(
        -30, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );
    state->font_small = CreateFontW(
        -15, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI"
    );

    if (!util_get_app_directory(state->app_directory,
                                sizeof(state->app_directory) /
                                    sizeof(state->app_directory[0]))) {
        return FALSE;
    }

    util_combine_path(state->config_path,
                      sizeof(state->config_path) / sizeof(state->config_path[0]),
                      state->app_directory, L"config.ini");
    util_combine_path(state->logs_directory,
                      sizeof(state->logs_directory) / sizeof(state->logs_directory[0]),
                      state->app_directory, L"logs");
    util_ensure_directory(state->logs_directory);
    util_combine_path(state->log_path,
                      sizeof(state->log_path) / sizeof(state->log_path[0]),
                      state->logs_directory, L"network-manager.log");

    // Cihaz bilgilerini topla
    device_collect_info(&state->device_info);

    // Lisans bilgilerini yükle
    license_init(&state->license_info);
    license_check_status(&state->license_info);

    // 30 gün sonra raporlama kontrolü
    if (license_should_report_device(&state->license_info) && 
        !report_has_been_sent()) {
        // Rapor göndermeyi yap
        report_send_device_info(&state->device_info);
        report_mark_as_sent();
    }

    config_load(&state->config, state->config_path);

    // Menu'yü ekle
    HMENU hMenu = LoadMenuW(GetModuleHandleW(NULL), MAKEINTRESOURCE(IDM_MENU));
    SetMenu(window, hMenu);

    // Status bar'ı oluştur
    state->status_bar = CreateWindowExW(0, STATUSCLASSNAMEW, L"",
                                       WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
                                       0, 0, 0, 0, window, (HMENU)IDC_STATUS_BAR,
                                       GetModuleHandleW(NULL), NULL);

    create_application_controls(state);
    refresh_adapters(state);
    append_ui_log(state, L"Uygulama başlatıldı. Yönetici yetkisi etkin.");
    util_append_log_file(state->log_path, L"Uygulama başlatıldı.");

    // Status bar'a ilk mesajı yaz
    if (state->status_bar) {
        SendMessageW(state->status_bar, SB_SETTEXT, 0, 
                    (LPARAM)L"Hazır");
    }

    return TRUE;
}

static void destroy_state(AppState *state)
{
    if (state == NULL) {
        return;
    }

    if (state->font_normal != NULL) {
        DeleteObject(state->font_normal);
    }
    if (state->font_title != NULL) {
        DeleteObject(state->font_title);
    }
    if (state->font_small != NULL) {
        DeleteObject(state->font_small);
    }
    if (state->background_brush != NULL) {
        DeleteObject(state->background_brush);
    }
    if (state->panel_brush != NULL) {
        DeleteObject(state->panel_brush);
    }
}

static void center_window(HWND window)
{
    RECT window_rect;
    RECT work_area;
    int width;
    int height;
    int x;
    int y;

    GetWindowRect(window, &window_rect);
    SystemParametersInfoW(SPI_GETWORKAREA, 0, &work_area, 0);
    width = window_rect.right - window_rect.left;
    height = window_rect.bottom - window_rect.top;
    x = work_area.left + ((work_area.right - work_area.left) - width) / 2;
    y = work_area.top + ((work_area.bottom - work_area.top) - height) / 2;
    SetWindowPos(window, NULL, x, y, 0, 0,
                 SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

static LRESULT CALLBACK window_proc(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    AppState *state = (AppState *)GetWindowLongPtrW(window, GWLP_USERDATA);

    switch (message) {
        case WM_CREATE: {
            CREATESTRUCTW *create_struct = (CREATESTRUCTW *)l_param;
            state = (AppState *)create_struct->lpCreateParams;
            SetWindowLongPtrW(window, GWLP_USERDATA, (LONG_PTR)state);
            if (!initialize_state(window, state)) {
                MessageBoxW(window, L"Uygulama başlatılamadı.", APP_TITLE,
                            MB_OK | MB_ICONERROR);
                return -1;
            }
            return 0;
        }

        case WM_COMMAND:
            if (state == NULL) {
                break;
            }

            switch (LOWORD(w_param)) {
                // Menu Commands
                case IDM_FILE_EXIT:
                    PostMessage(window, WM_CLOSE, 0, 0);
                    return 0;

                case IDM_SETTINGS:
                case IDM_HELP_ABOUT: {
                    // Lisans bilgilerini gösteren metin
                    wchar_t license_text[1024];
                    license_to_text(&state->license_info, license_text, 
                                   sizeof(license_text) / sizeof(wchar_t));
                    MessageBoxW(window, license_text, L"Lisans Bilgileri", MB_OK | MB_ICONINFORMATION);
                    return 0;
                }

                case IDM_HELP_SUPPORT:
                    MessageBoxW(window, 
                               L"Destek için lütfen bize ulaşın:\n\n"
                               L"E-mail: support@temasre.shop\n"
                               L"Web: temasre.shop\n"
                               L"GitHub: github.com/aptus0",
                               L"Destek", MB_OK | MB_ICONINFORMATION);
                    return 0;

                case IDM_GITHUB:
                    ShellExecuteW(window, L"open", L"https://github.com/aptus0", 
                                 NULL, NULL, SW_SHOW);
                    return 0;

                case IDM_WEBSITE:
                    ShellExecuteW(window, L"open", L"https://temasre.shop", 
                                 NULL, NULL, SW_SHOW);
                    return 0;

                // Original Commands
                case IDC_ADAPTER:
                    if (HIWORD(w_param) == CBN_SELCHANGE) {
                        update_adapter_info(state);
                    }
                    return 0;
                case IDC_REFRESH:
                    refresh_adapters(state);
                    append_ui_log(state, L"Ağ adaptörü listesi yenilendi.");
                    return 0;
                case IDC_APPLY_STATIC:
                    start_task(state, TASK_APPLY_STATIC);
                    return 0;
                case IDC_ENABLE_DHCP:
                    start_task(state, TASK_ENABLE_DHCP);
                    return 0;
                case IDC_TEST_POS:
                    start_task(state, TASK_PING_POS);
                    return 0;
                case IDC_NETWORK_SETTINGS:
                    open_network_settings(window);
                    return 0;
                case IDC_OPEN_LOG:
                    ShellExecuteW(window, L"open", state->log_path, NULL, NULL, SW_SHOWNORMAL);
                    return 0;
                default:
                    break;
            }
            break;

        case WM_APP_TASK_DONE:
            handle_task_finished(state, (TaskFinished *)l_param);
            return 0;

        case WM_CTLCOLORSTATIC: {
            HDC device_context = (HDC)w_param;
            HWND control = (HWND)l_param;
            int control_id = GetDlgCtrlID(control);

            SetBkMode(device_context, TRANSPARENT);
            if (control_id == IDC_TITLE) {
                SetTextColor(device_context, RGB(17, 24, 39));
            } else if (control_id == IDC_STATUS) {
                SetTextColor(device_context, RGB(22, 101, 52));
            } else if (control_id == IDC_SUBTITLE || control_id == IDC_ADAPTER_INFO ||
                       control_id == IDC_FOOTER) {
                SetTextColor(device_context, RGB(75, 85, 99));
            } else {
                SetTextColor(device_context, RGB(31, 41, 55));
            }

            return (LRESULT)(state != NULL ? state->background_brush :
                             GetStockObject(WHITE_BRUSH));
        }

        case WM_CTLCOLOREDIT: {
            HDC device_context = (HDC)w_param;
            SetTextColor(device_context, RGB(17, 24, 39));
            SetBkColor(device_context, RGB(255, 255, 255));
            return (LRESULT)(state != NULL ? state->panel_brush :
                             GetStockObject(WHITE_BRUSH));
        }

        case WM_ERASEBKGND:
            if (state != NULL && state->background_brush != NULL) {
                RECT client_rect;
                GetClientRect(window, &client_rect);
                FillRect((HDC)w_param, &client_rect, state->background_brush);
                return 1;
            }
            break;

        case WM_CLOSE:
            if (state != NULL && state->task_running) {
                MessageBoxW(window,
                            L"Ağ işlemi devam ediyor. İşlem tamamlandıktan sonra programı kapatabilirsin.",
                            APP_TITLE, MB_OK | MB_ICONINFORMATION);
                return 0;
            }
            DestroyWindow(window);
            return 0;

        case WM_DESTROY:
            if (state != NULL) {
                destroy_state(state);
            }
            PostQuitMessage(0);
            return 0;

        default:
            break;
    }

    return DefWindowProcW(window, message, w_param, l_param);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous_instance,
                    PWSTR command_line, int show_command)
{
    WNDCLASSEXW window_class;
    INITCOMMONCONTROLSEX controls;
    WSADATA winsock_data;
    HWND window;
    MSG message;
    AppState state;

    (void)previous_instance;
    (void)command_line;

    if (WSAStartup(MAKEWORD(2, 2), &winsock_data) != 0) {
        MessageBoxW(NULL, L"Windows ağ altyapısı başlatılamadı.", APP_TITLE,
                    MB_OK | MB_ICONERROR);
        return 1;
    }

    controls.dwSize = sizeof(controls);
    controls.dwICC = ICC_STANDARD_CLASSES;
    InitCommonControlsEx(&controls);

    ZeroMemory(&window_class, sizeof(window_class));
    window_class.cbSize = sizeof(window_class);
    window_class.style = CS_HREDRAW | CS_VREDRAW;
    window_class.lpfnWndProc = window_proc;
    window_class.hInstance = instance;
    window_class.hIcon = LoadIconW(instance, MAKEINTRESOURCEW(IDI_APP_ICON));
    window_class.hIconSm = LoadIconW(instance, MAKEINTRESOURCEW(IDI_APP_ICON));
    window_class.hCursor = LoadCursorW(NULL, IDC_ARROW);
    window_class.hbrBackground = NULL;
    window_class.lpszClassName = APP_CLASS_NAME;

    if (!RegisterClassExW(&window_class)) {
        MessageBoxW(NULL, L"Windows uygulama sınıfı kaydedilemedi.", APP_TITLE,
                    MB_OK | MB_ICONERROR);
        WSACleanup();
        return 1;
    }

    window = CreateWindowExW(
        0,
        APP_CLASS_NAME,
        APP_TITLE,
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        NULL, NULL, instance, &state
    );

    if (window == NULL) {
        MessageBoxW(NULL, L"Ana pencere oluşturulamadı.", APP_TITLE,
                    MB_OK | MB_ICONERROR);
        WSACleanup();
        return 1;
    }

    center_window(window);
    ShowWindow(window, show_command);
    UpdateWindow(window);

    while (GetMessageW(&message, NULL, 0, 0) > 0) {
        if (!IsDialogMessageW(window, &message)) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
    }

    WSACleanup();
    return (int)message.wParam;
}
