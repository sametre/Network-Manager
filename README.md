# PC • POS Network Manager

Windows bilgisayar ile POS cihazı arasındaki yerel ağ iletişimini kararlı tutmak için hazırlanmış yerel bir IPv4 profil yöneticisidir.

Varsayılan ağ profili:

- Modem / Gateway: `192.168.1.1`
- Bilgisayar: `192.168.1.10`
- POS cihazı: `192.168.1.11`
- Alt ağ maskesi: `255.255.255.0`

## Özellikler

- Windows üzerindeki Ethernet ve Wi-Fi adaptörlerini otomatik bulur.
- Adaptör adı `Ethernet 2` gibi değişse bile kaynak kodu düzenlemeyi gerektirmez.
- Bilgisayarın IPv4, ağ maskesi, gateway ve DNS ayarlarını kalıcı statik profile geçirir.
- Seçilen adaptörü tekrar otomatik IP/DHCP moduna döndürür.
- POS cihazına üç denemeli ICMP bağlantı testi uygular.
- IP adreslerini ve aynı alt ağ koşulunu işlemden önce doğrular.
- Arayüzü dondurmadan arka planda ağ işlemlerini yürütür.
- Ayarları `config.ini`, işlem kayıtlarını `logs/network-manager.log` dosyasında tutar.
- Windows yönetici yetkisini manifest üzerinden otomatik ister.
- MinGW-w64, Visual Studio C++ Build Tools ve CMake yapılarını destekler.

## EXE Oluşturma

### En kolay yöntem

1. Proje klasörünü Windows bilgisayara çıkartın.
2. Visual Studio C++ Build Tools veya MinGW-w64 kurulu olduğundan emin olun.
3. Ana klasördeki `BUILD_EXE.bat` dosyasını çalıştırın.
4. Çıktı `release/PcPosNetworkManager.exe` yolunda oluşur.

`BUILD_EXE.bat`, önce mevcut derleyiciyi kontrol eder. Visual Studio bulunursa MSVC, MinGW-w64 bulunursa GCC ile derleme yapar.

### Visual Studio ile

```bat
scripts\build_msvc.bat
```

Visual Studio Installer içinden **Desktop development with C++** iş yükü kurulmalıdır. Script, Visual Studio kurulumunu `vswhere` üzerinden otomatik bulmayı dener.

### MinGW-w64 ile

```bat
scripts\build_mingw.bat
```

Gerekli komutlar:

```text
x86_64-w64-mingw32-gcc.exe
x86_64-w64-mingw32-windres.exe
```

Standart MSYS2/MinGW kurulumu kullanılıyorsa `gcc.exe` ve `windres.exe` de otomatik algılanır.

### CMake ile

```bat
cmake -S . -B build-cmake -G "Visual Studio 17 2022" -A x64
cmake --build build-cmake --config Release
```

### GitHub Actions ile otomatik EXE

Projeyi GitHub deposuna gönderdiğinizde `.github/workflows/windows-build.yml` iş akışı Windows x64 EXE dosyasını otomatik derler. GitHub üzerindeki **Actions > Windows x64 Build > Run workflow** adımlarıyla manuel olarak da çalıştırılabilir. Derleme tamamlandığında EXE, workflow artifact'i olarak sunulur.

## Kullanım

1. Programı açın. Windows yönetici izni isteyecektir.
2. PC ile modeme bağlı olan ağ adaptörünü seçin.
3. Bilgisayar IP alanını `192.168.1.10` olarak bırakın.
4. POS IP alanını `192.168.1.11` olarak bırakın.
5. `Sabit IP Uygula` düğmesine basın.
6. İşlem sonrası POS testi otomatik çalışır. Ayrı olarak `POS Bağlantısını Test Et` düğmesi de kullanılabilir.
7. Otomatik IP'ye dönmek gerektiğinde `DHCP'ye Geri Dön` düğmesine basın.

## Modemde Yapılması Gereken Ayar

Program bilgisayarın IP adresini sabitler. POS cihazının IP adresinin de değişmemesi için modem arayüzündeki DHCP rezervasyonu bölümünde MAC-IP eşlemesi yapılmalıdır:

```text
PC MAC  -> 192.168.1.10
POS MAC -> 192.168.1.11
```

Modemin otomatik dağıtım havuzunu mümkünse aşağıdaki gibi ayrı bir aralıkta tutun:

```text
192.168.1.100 - 192.168.1.250
```

Böylece modem `.10` ve `.11` adreslerini yanlışlıkla başka cihazlara vermez.

POS cihazında statik ağ ayarı bulunuyorsa:

```text
IP      : 192.168.1.11
Mask    : 255.255.255.0
Gateway : 192.168.1.1
DNS     : 192.168.1.1
```

## Proje Yapısı

```text
PcPosNetworkManager/
├── BUILD_EXE.bat
├── CMakeLists.txt
├── config.example.ini
├── README.md
├── docs/
│   ├── BUILD_WINDOWS.md
│   ├── NETWORK_SETUP.md
│   └── PROJECT_STRUCTURE.md
├── resources/
│   ├── app.ico
│   ├── app.manifest
│   ├── app-preview.png
│   └── app.rc
├── scripts/
│   ├── build_mingw.bat
│   ├── build_msvc.bat
│   └── package_release.bat
├── src/
│   ├── config.c
│   ├── config.h
│   ├── main.c
│   ├── network.c
│   ├── network.h
│   ├── resource.h
│   ├── utils.c
│   └── utils.h
├── build/
└── release/
```

## Güvenlik Notları

- Ağ ayarı değiştiren işlemler yönetici yetkisi gerektirir.
- Kullanıcıdan alınan IPv4 alanları işlem öncesinde doğrulanır.
- Ağ adaptörü seçim listesinden alınır; komut satırına kontrolsüz serbest metin aktarılmaz.
- Program yalnızca yerel Windows ağ yapılandırmasını değiştirir ve POS cihazına bağlantı testi uygular.

## Release Paketi

EXE derlendikten sonra:

```bat
scripts\package_release.bat
```

Bu işlem `PcPosNetworkManager-v1.0.0-Windows-x64.zip` paketini oluşturur.
