#include "network.h"

#pragma warning(disable: 4115)  // ICMP SDK: named type definition in parentheses

#include <iphlpapi.h>
#include <icmpapi.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#include "utils.h"

#define PROCESS_OUTPUT_LEN 8192

static void result_reset(NetworkOperationResult *result)
{
    if (result != NULL) {
        ZeroMemory(result, sizeof(*result));
    }
}

static void result_append(NetworkOperationResult *result, const wchar_t *text)
{
    if (result != NULL && text != NULL) {
        util_append_text(result->details, OPERATION_DETAILS_LEN, text);
    }
}

static void append_command_header(NetworkOperationResult *result, const wchar_t *title)
{
    result_append(result, L"\r\n--- ");
    result_append(result, title);
    result_append(result, L" ---\r\n");
}

static BOOL run_process_capture(const wchar_t *command_line_source,
                                wchar_t *output,
                                size_t output_count,
                                DWORD *exit_code)
{
    SECURITY_ATTRIBUTES security_attributes;
    STARTUPINFOW startup_info;
    PROCESS_INFORMATION process_info;
    HANDLE read_pipe = NULL;
    HANDLE write_pipe = NULL;
    wchar_t command_line[2048];
    char byte_buffer[1024];
    DWORD bytes_read;
    DWORD process_exit_code = ERROR_GEN_FAILURE;
    size_t output_used = 0;
    BOOL success = FALSE;

    if (command_line_source == NULL || output == NULL || output_count == 0) {
        return FALSE;
    }

    output[0] = L'\0';
    util_copy_text(command_line, sizeof(command_line) / sizeof(command_line[0]), command_line_source);

    ZeroMemory(&security_attributes, sizeof(security_attributes));
    security_attributes.nLength = sizeof(security_attributes);
    security_attributes.bInheritHandle = TRUE;

    if (!CreatePipe(&read_pipe, &write_pipe, &security_attributes, 0)) {
        return FALSE;
    }

    SetHandleInformation(read_pipe, HANDLE_FLAG_INHERIT, 0);

    ZeroMemory(&startup_info, sizeof(startup_info));
    startup_info.cb = sizeof(startup_info);
    startup_info.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    startup_info.wShowWindow = SW_HIDE;
    startup_info.hStdOutput = write_pipe;
    startup_info.hStdError = write_pipe;
    startup_info.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    ZeroMemory(&process_info, sizeof(process_info));

    if (!CreateProcessW(
            NULL,
            command_line,
            NULL,
            NULL,
            TRUE,
            CREATE_NO_WINDOW,
            NULL,
            NULL,
            &startup_info,
            &process_info)) {
        CloseHandle(read_pipe);
        CloseHandle(write_pipe);
        return FALSE;
    }

    CloseHandle(write_pipe);
    write_pipe = NULL;

    while (ReadFile(read_pipe, byte_buffer, sizeof(byte_buffer), &bytes_read, NULL) && bytes_read > 0) {
        int required;
        int converted;

        if (output_used >= output_count - 1) {
            continue;
        }

        required = MultiByteToWideChar(CP_OEMCP, 0, byte_buffer, (int)bytes_read, NULL, 0);
        if (required <= 0) {
            continue;
        }

        if ((size_t)required > output_count - output_used - 1) {
            required = (int)(output_count - output_used - 1);
        }

        converted = MultiByteToWideChar(CP_OEMCP, 0, byte_buffer, (int)bytes_read,
                                        output + output_used, required);
        if (converted > 0) {
            output_used += (size_t)converted;
            output[output_used] = L'\0';
        }
    }

    WaitForSingleObject(process_info.hProcess, INFINITE);
    if (GetExitCodeProcess(process_info.hProcess, &process_exit_code)) {
        success = process_exit_code == 0;
    }

    CloseHandle(read_pipe);
    CloseHandle(process_info.hThread);
    CloseHandle(process_info.hProcess);

    if (exit_code != NULL) {
        *exit_code = process_exit_code;
    }

    return success;
}

static BOOL execute_step(NetworkOperationResult *result,
                         const wchar_t *step_title,
                         const wchar_t *command_line)
{
    wchar_t output[PROCESS_OUTPUT_LEN];
    wchar_t error_message[512];
    wchar_t exit_message[128];
    DWORD exit_code = ERROR_GEN_FAILURE;
    BOOL ok;

    append_command_header(result, step_title);
    ok = run_process_capture(command_line, output,
                             sizeof(output) / sizeof(output[0]), &exit_code);

    if (output[0] != L'\0') {
        result_append(result, output);
        if (output[wcslen(output) - 1] != L'\n') {
            result_append(result, L"\r\n");
        }
    }

    if (!ok) {
        if (exit_code == ERROR_GEN_FAILURE) {
            util_format_system_error(GetLastError(), error_message,
                                     sizeof(error_message) / sizeof(error_message[0]));
            result_append(result, L"İşlem başlatılamadı: ");
            result_append(result, error_message);
            result_append(result, L"\r\n");
        } else {
            swprintf(exit_message, sizeof(exit_message) / sizeof(exit_message[0]),
                     L"Komut başarısız oldu. Çıkış kodu: %lu\r\n",
                     (unsigned long)exit_code);
            result_append(result, exit_message);
        }
        result->error_code = exit_code;
    }

    return ok;
}

size_t network_enumerate_adapters(AdapterInfo *items, size_t capacity)
{
    ULONG buffer_size = 16 * 1024;
    ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST |
                  GAA_FLAG_SKIP_DNS_SERVER;
    IP_ADAPTER_ADDRESSES *addresses = NULL;
    IP_ADAPTER_ADDRESSES *adapter;
    DWORD status;
    size_t count = 0;

    if (items == NULL || capacity == 0) {
        return 0;
    }

    addresses = (IP_ADAPTER_ADDRESSES *)malloc(buffer_size);
    if (addresses == NULL) {
        return 0;
    }

    status = GetAdaptersAddresses(AF_INET, flags, NULL, addresses, &buffer_size);
    if (status == ERROR_BUFFER_OVERFLOW) {
        IP_ADAPTER_ADDRESSES *larger = (IP_ADAPTER_ADDRESSES *)realloc(addresses, buffer_size);
        if (larger == NULL) {
            free(addresses);
            return 0;
        }
        addresses = larger;
        status = GetAdaptersAddresses(AF_INET, flags, NULL, addresses, &buffer_size);
    }

    if (status != NO_ERROR) {
        free(addresses);
        return 0;
    }

    for (adapter = addresses; adapter != NULL && count < capacity; adapter = adapter->Next) {
        AdapterInfo *item;
        IP_ADAPTER_UNICAST_ADDRESS *unicast;
        size_t index;
        wchar_t byte_text[8];

        if (adapter->IfType == IF_TYPE_SOFTWARE_LOOPBACK ||
            adapter->IfType == IF_TYPE_TUNNEL ||
            adapter->FriendlyName == NULL) {
            continue;
        }

        item = &items[count];
        ZeroMemory(item, sizeof(*item));
        util_copy_text(item->friendly_name, ADAPTER_NAME_LEN, adapter->FriendlyName);
        util_copy_text(item->description, ADAPTER_NAME_LEN,
                       adapter->Description != NULL ? adapter->Description : L"");
        item->is_up = adapter->OperStatus == IfOperStatusUp;
        item->if_type = adapter->IfType;
        util_copy_text(item->ipv4, CONFIG_VALUE_LEN, L"IPv4 yok");

        for (unicast = adapter->FirstUnicastAddress; unicast != NULL; unicast = unicast->Next) {
            SOCKADDR_IN *ipv4;
            if (unicast->Address.lpSockaddr == NULL ||
                unicast->Address.lpSockaddr->sa_family != AF_INET) {
                continue;
            }

            ipv4 = (SOCKADDR_IN *)unicast->Address.lpSockaddr;
            if (InetNtopW(AF_INET, &ipv4->sin_addr, item->ipv4, CONFIG_VALUE_LEN) != NULL) {
                break;
            }
        }

        item->mac[0] = L'\0';
        for (index = 0; index < adapter->PhysicalAddressLength; ++index) {
            swprintf(byte_text, sizeof(byte_text) / sizeof(byte_text[0]),
                     index == 0 ? L"%02X" : L"-%02X",
                     adapter->PhysicalAddress[index]);
            util_append_text(item->mac, sizeof(item->mac) / sizeof(item->mac[0]), byte_text);
        }
        if (item->mac[0] == L'\0') {
            util_copy_text(item->mac, sizeof(item->mac) / sizeof(item->mac[0]), L"Bulunamadı");
        }

        ++count;
    }

    free(addresses);
    return count;
}

BOOL network_apply_static(const AppConfig *config, NetworkOperationResult *result)
{
    wchar_t command[2048];
    BOOL ok;

    result_reset(result);
    if (config == NULL || result == NULL) {
        return FALSE;
    }

    swprintf(command, sizeof(command) / sizeof(command[0]),
             L"netsh.exe interface ipv4 set address name=\"%ls\" source=static "
             L"address=%ls mask=%ls gateway=%ls gwmetric=1 store=persistent",
             config->adapter, config->pc_ip, config->subnet_mask, config->gateway);

    ok = execute_step(result, L"Sabit IPv4 adresi", command);
    if (!ok) {
        util_copy_text(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
                       L"Sabit IP uygulanamadı.");
        return FALSE;
    }

    swprintf(command, sizeof(command) / sizeof(command[0]),
             L"netsh.exe interface ipv4 set dnsservers name=\"%ls\" source=static "
             L"address=%ls register=primary validate=no",
             config->adapter, config->dns_primary);

    ok = execute_step(result, L"Birincil DNS", command);
    if (!ok) {
        util_copy_text(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
                       L"IP uygulandı ancak DNS ayarlanamadı.");
        return FALSE;
    }

    if (config->dns_secondary[0] != L'\0') {
        swprintf(command, sizeof(command) / sizeof(command[0]),
                 L"netsh.exe interface ipv4 add dnsservers name=\"%ls\" "
                 L"address=%ls index=2 validate=no",
                 config->adapter, config->dns_secondary);

        ok = execute_step(result, L"İkincil DNS", command);
        if (!ok) {
            util_copy_text(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
                           L"IP uygulandı ancak ikincil DNS ayarlanamadı.");
            return FALSE;
        }
    }

    execute_step(result, L"DNS önbelleğini temizleme", L"ipconfig.exe /flushdns");

    result->success = TRUE;
    swprintf(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
             L"%ls adaptörü %ls olarak sabitlendi.", config->adapter, config->pc_ip);
    return TRUE;
}

BOOL network_enable_dhcp(const AppConfig *config, NetworkOperationResult *result)
{
    wchar_t command[1024];
    BOOL ok;

    result_reset(result);
    if (config == NULL || result == NULL) {
        return FALSE;
    }

    swprintf(command, sizeof(command) / sizeof(command[0]),
             L"netsh.exe interface ipv4 set address name=\"%ls\" source=dhcp",
             config->adapter);
    ok = execute_step(result, L"IPv4 DHCP modu", command);
    if (!ok) {
        util_copy_text(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
                       L"Otomatik IP modu etkinleştirilemedi.");
        return FALSE;
    }

    swprintf(command, sizeof(command) / sizeof(command[0]),
             L"netsh.exe interface ipv4 set dnsservers name=\"%ls\" source=dhcp",
             config->adapter);
    ok = execute_step(result, L"DNS DHCP modu", command);
    if (!ok) {
        util_copy_text(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
                       L"IP DHCP oldu ancak DNS otomatik moda alınamadı.");
        return FALSE;
    }

    execute_step(result, L"IP adresini yenileme", L"ipconfig.exe /renew");
    execute_step(result, L"DNS önbelleğini temizleme", L"ipconfig.exe /flushdns");

    result->success = TRUE;
    swprintf(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
             L"%ls adaptörü otomatik IP (DHCP) moduna alındı.", config->adapter);
    return TRUE;
}

BOOL network_ping_pos(const AppConfig *config, NetworkOperationResult *result)
{
    HANDLE icmp_handle;
    IN_ADDR destination;
    const char payload[] = "PcPosNetworkManager";
    BYTE reply_buffer[sizeof(ICMP_ECHO_REPLY) + sizeof(payload) + 64];
    DWORD reply_count;
    int attempt;
    int successful = 0;
    unsigned long total_time = 0;
    wchar_t line[256];

    result_reset(result);
    if (config == NULL || result == NULL) {
        return FALSE;
    }

    if (InetPtonW(AF_INET, config->pos_ip, &destination) != 1) {
        util_copy_text(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
                       L"POS IP adresi geçersiz.");
        return FALSE;
    }

    icmp_handle = IcmpCreateFile();
    if (icmp_handle == INVALID_HANDLE_VALUE) {
        result->error_code = GetLastError();
        util_copy_text(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
                       L"Ping servisi başlatılamadı.");
        return FALSE;
    }

    append_command_header(result, L"POS bağlantı testi");

    for (attempt = 1; attempt <= 3; ++attempt) {
        PICMP_ECHO_REPLY reply;
        ZeroMemory(reply_buffer, sizeof(reply_buffer));

        reply_count = IcmpSendEcho(
            icmp_handle,
            destination.S_un.S_addr,
            (LPVOID)payload,
            (WORD)sizeof(payload),
            NULL,
            reply_buffer,
            (DWORD)sizeof(reply_buffer),
            1500
        );

        if (reply_count > 0) {
            reply = (PICMP_ECHO_REPLY)reply_buffer;
            if (reply->Status == IP_SUCCESS) {
                ++successful;
                total_time += reply->RoundTripTime;
                swprintf(line, sizeof(line) / sizeof(line[0]),
                         L"Deneme %d: Başarılı — %lu ms\r\n",
                         attempt, (unsigned long)reply->RoundTripTime);
            } else {
                swprintf(line, sizeof(line) / sizeof(line[0]),
                         L"Deneme %d: Yanıt durumu %lu\r\n",
                         attempt, (unsigned long)reply->Status);
            }
        } else {
            DWORD error_code = GetLastError();
            swprintf(line, sizeof(line) / sizeof(line[0]),
                     L"Deneme %d: Yanıt alınamadı — hata %lu\r\n",
                     attempt, (unsigned long)error_code);
            result->error_code = error_code;
        }

        result_append(result, line);
        if (attempt < 3) {
            Sleep(250);
        }
    }

    IcmpCloseHandle(icmp_handle);

    if (successful > 0) {
        result->success = TRUE;
        swprintf(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
                 L"POS cihazına erişildi: %ls — %d/3 yanıt, ortalama %lu ms.",
                 config->pos_ip,
                 successful,
                 (unsigned long)(total_time / (unsigned long)successful));
        return TRUE;
    }

    swprintf(result->summary, sizeof(result->summary) / sizeof(result->summary[0]),
             L"POS cihazından yanıt alınamadı: %ls.", config->pos_ip);
    return FALSE;
}
