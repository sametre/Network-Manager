# Network Manager

PC ile POS cihazı arasındaki yerel ağ iletişimini kararlı tutan Windows ağ profil yöneticisi.

[![Latest Release](https://img.shields.io/github/v/release/YOUR_USERNAME/Network-Manager-Releases?label=Son%20Sürüm&style=for-the-badge)](https://github.com/YOUR_USERNAME/Network-Manager-Releases/releases/latest)
[![Platform](https://img.shields.io/badge/Platform-Windows%2010%2F11%20x64-blue?style=for-the-badge)](https://github.com/YOUR_USERNAME/Network-Manager-Releases/releases/latest)

---

## 📥 İndir

**[→ Son Sürümü İndir](https://github.com/YOUR_USERNAME/Network-Manager-Releases/releases/latest)**

Sağ taraftaki **Releases** bölümünden `NetworkManager-vX.X.X-Windows-x64.zip` dosyasını indirin.

---

## ✅ Gereksinimler

- Windows 10 veya Windows 11 (64-bit)
- Yönetici (Administrator) yetkisi

---

## 🚀 Kurulum

1. ZIP dosyasını indirin ve istediğiniz bir klasöre çıkartın.
2. `NetworkManager.exe` dosyasına sağ tıklayın → **Yönetici olarak çalıştır**.
3. Windows UAC onayını verin.

---

## 🔧 Kullanım

1. Açılan pencereden PC ile modeme bağlı ağ adaptörünü seçin.
2. PC IP alanını `192.168.1.10` olarak bırakın.
3. POS IP alanını `192.168.1.11` olarak bırakın.
4. **Sabit IP Uygula** düğmesine basın.
5. İşlem tamamlandığında POS bağlantı testi otomatik çalışır.
6. Otomatik IP'ye dönmek gerektiğinde **DHCP'ye Geri Dön** düğmesini kullanın.

---

## ⚙️ Varsayılan Ağ Profili

| Cihaz | IP Adresi |
|---|---|
| Modem / Gateway | `192.168.1.1` |
| Bilgisayar (PC) | `192.168.1.10` |
| POS Cihazı | `192.168.1.11` |
| Alt Ağ Maskesi | `255.255.255.0` |

---

## 🛡️ Güvenlik

Bu program yalnızca yerel Windows ağ yapılandırmasını değiştirir ve POS cihazına bağlantı testi uygular. İnternet bağlantısı gerektirmez.

---

> **Not:** Bu repository yalnızca derlenmiş sürümleri içerir. Kaynak kodu paylaşılmamaktadır.
