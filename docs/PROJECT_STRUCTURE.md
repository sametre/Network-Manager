# Proje Mimarisi

## `src/main.c`

Win32 masaüstü arayüzü, adaptör seçimi, form doğrulaması, arka plan görevleri ve kullanıcı bildirimleri.

## `src/network.c`

Windows IP Helper API ile adaptör keşfi, `netsh` üzerinden kalıcı IPv4/DHCP yönetimi ve ICMP POS bağlantı testi.

## `src/config.c`

`config.ini` profilinin okunması ve kaydedilmesi.

## `src/utils.c`

IPv4 doğrulama, alt ağ kontrolü, güvenli metin/path işlemleri ve UTF-8 loglama yardımcıları.

## `resources`

Uygulama ikonu, Windows manifesti, sürüm bilgileri ve resource tanımları.

## İş Parçacığı Modeli

Ağ değişiklikleri ve ping işlemleri ayrı bir worker thread üzerinde yürür. Sonuç, `WM_APP_TASK_DONE` mesajıyla ana UI thread'ine aktarılır. Böylece uygulama işlem sırasında donmaz.

## Ayarların Yaşam Döngüsü

1. Uygulama açılırken `config.ini` okunur.
2. Dosya yoksa güvenli varsayılan profil yüklenir.
3. Kullanıcı bir işlem başlattığında profil doğrulanır.
4. Profil `config.ini` dosyasına kaydedilir.
5. İşlem sonucu arayüze ve `logs/network-manager.log` dosyasına yazılır.
