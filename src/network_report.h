#ifndef PCPOS_NETWORK_REPORT_H
#define PCPOS_NETWORK_REPORT_H

#include "device.h"
#include <wchar.h>

#define REPORT_SERVER_URL L"http://temasre.shop/ip-address"

typedef enum {
	REPORT_STATUS_PENDING,
	REPORT_STATUS_SENT,
	REPORT_STATUS_FAILED,
	REPORT_STATUS_ERROR
} ReportStatus;

// Cihaz bilgisini sunucuya gönder (HTTP POST)
// Döner: 0 başarılı, -1 başarısız
int report_send_device_info(const DeviceInfo *device_info);

// Raporu TXT formatında dosyaya kaydet (backup)
int report_save_to_file(const DeviceInfo *device_info, const wchar_t *file_path);

// Raporlama durumunu Registry'de güncelle
int report_mark_as_sent(void);

// Raporlanıp raporlanmadığını kontrol et
int report_has_been_sent(void);

#endif
