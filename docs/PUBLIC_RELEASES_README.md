# Network Manager

PC ile POS cihazı arasındaki yerel ağ iletişimini kararlı tutan Windows ağ profil yöneticisi.

[![Latest Release](https://img.shields.io/github/v/release/aptus0/Network-Manager-Releases?label=Son%20Sürüm&style=for-the-badge)](https://github.com/aptus0/Network-Manager-Releases/releases/latest)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11%20x64-blue?style=for-the-badge)](https://github.com/aptus0/Network-Manager-Releases/releases/latest)

---

## 📥 İndirme Seçenekleri

Her yeni sürüm (Etiket/Tag) yayınlandığında aşağıdaki iki formatta dosya otomatik olarak oluşturulur:

1. **🚀 Kolay Kurulum (Setup - Önerilen)**
   * **[→ NetworkManager-Setup.exe İndir](https://github.com/aptus0/Network-Manager-Releases/releases/latest)**
   * Bilgisayarınıza otomatik kurulum yapar, Masaüstü ve Başlat menüsüne Türkçe kısayollar ekler.

2. **📦 Taşınabilir Sürüm (Portable ZIP)**
   * **[→ NetworkManager-v1.x.x-Windows-x64.zip İndir](https://github.com/aptus0/Network-Manager-Releases/releases/latest)**
   * Kurulum gerektirmez, doğrudan arşivden çıkarılıp çalıştırılabilir.

---

## 🏷️ Sürümler ve Versiyon Etiketleri (Tags)

Bu projedeki sürümler `v1.0.0`, `v1.1.0` şeklinde etiketlenerek yayınlanır.
* **`main` dalı (branch):** Her zaman en kararlı ana kaynak kodları barındırır.
* **Git Etiketleri (Tags):** Her `v*.*.*` etiketi basıldığında GitHub Actions otomatik olarak yeni sürümü derler, paketler ve buradaki Releases sayfasına yükler.

---

## ✅ Sistem Gereksinimleri

* Windows 10 veya Windows 11 (64-bit)
* Ağ ayarlarını değiştirmek için **Yönetici (Administrator)** yetkisi

---

## 🚀 Setup Kurulumu

1. `NetworkManager-Setup.exe` dosyasını indirin.
2. Çift tıklayarak çalıştırın (UAC/Yönetici uyarısına "Evet" deyin).
3. Kurulum adımlarını Türkçe yönergelerle tamamlayın.
4. Program kurulduktan sonra masaüstündeki kısayolu kullanarak başlatabilirsiniz.

---

## 🔧 Programın Kullanımı

1. Ağ listesinden PC ile modemin bağlı olduğu adaptörü (örn. Ethernet veya Wi-Fi) seçin.
2. Bilgisayar statik IP'sini (`192.168.1.10`) ve POS IP'sini (`192.168.1.11`) kontrol edin.
3. **Sabit IP Uygula** düğmesine tıklayın.
4. Ağ geçidi (Gateway) ve diğer ağ profili ayarlarınız otomatik olarak statik konfigürasyona geçirilecektir.
5. İşlem sonrasında POS bağlantı testi arka planda otomatik çalışır.
6. Tekrar DHCP (otomatik IP) moduna dönmek isterseniz **DHCP'ye Geri Dön** düğmesini kullanın.

---

## ⚙️ Varsayılan Ağ Profili Şeması

| Cihaz / Rol | IP Adresi | Açıklama |
|---|---|---|
| Modem / Ağ Geçidi | `192.168.1.1` | Varsayılan Gateway adresi |
| Bilgisayar (PC) | `192.168.1.10` | Sabitlenecek yerel IP adresi |
| POS Cihazı | `192.168.1.11` | Bağlantı kurulacak POS adresi |
| Alt Ağ Maskesi | `255.255.255.0` | Standart subnet maskesi |

---

## 🛡️ Güvenlik ve Gizlilik

Bu araç yalnızca yerel bilgisayarınızdaki ağ adaptörlerinin ayarlarını yönetir. Herhangi bir dış sunucuya kişisel veri göndermez. Sadece POS cihazınızın IP adresine yerel ICMP (ping) paketi göndererek testi gerçekleştirir.

---

> **Bilgilendirme:** Bu depo (Releases) sadece derlenmiş dağıtım paketlerini sunar. Kaynak kodları güvenlik amacıyla özel (private) bir depoda tutulmaktadır.
