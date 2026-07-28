# Windows Derleme Rehberi

## Seçenek 1: Visual Studio Build Tools

Visual Studio Installer üzerinden aşağıdaki iş yükünü kurun:

```text
Desktop development with C++
```

Ardından proje ana klasöründeki dosyayı çalıştırın:

```bat
BUILD_EXE.bat
```

Doğrudan MSVC scripti de çalıştırılabilir:

```bat
scripts\build_msvc.bat
```

Derleme çıktısı:

```text
release\PcPosNetworkManager.exe
```

## Seçenek 2: MSYS2 UCRT64 / MinGW-w64

MSYS2 UCRT64 ortamında GCC ve binutils paketlerini kurup UCRT64 `bin` klasörünü Windows PATH değişkenine ekleyin. Sonra:

```bat
scripts\build_mingw.bat
```

Script hem `x86_64-w64-mingw32-gcc` hem de standart `gcc` komut adlarını destekler.

## Kaynak ve Resource Derleme

Windows resource dosyası aşağıdaki öğeleri EXE içine gömer:

- Uygulama ikonu
- Dosya ve ürün sürümü
- Yönetici yetkisi manifesti
- Common Controls v6 teması
- Per-Monitor DPI farkındalığı

## Temiz Derleme

`build` ve `release` klasörlerinin içeriği silinebilir. Kaynak kod etkilenmez.
