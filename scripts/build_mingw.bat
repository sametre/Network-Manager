@echo off
setlocal EnableExtensions
chcp 65001 >nul

set "ROOT=%~dp0.."
set "BUILD=%ROOT%\build"
set "RELEASE=%ROOT%\release"

if not exist "%BUILD%" mkdir "%BUILD%"
if not exist "%RELEASE%" mkdir "%RELEASE%"

set "CC="
set "RC="

where x86_64-w64-mingw32-gcc.exe >nul 2>nul
if not errorlevel 1 (
    set "CC=x86_64-w64-mingw32-gcc.exe"
    set "RC=x86_64-w64-mingw32-windres.exe"
) else (
    where gcc.exe >nul 2>nul
    if not errorlevel 1 (
        where windres.exe >nul 2>nul
        if not errorlevel 1 (
            set "CC=gcc.exe"
            set "RC=windres.exe"
        )
    )
)

if not defined CC (
    echo.
    echo [HATA] MinGW-w64 GCC ve windres bulunamadı.
    echo MSYS2 UCRT64 veya MinGW-w64 kurup bin klasörünü PATH'e ekleyin.
    echo Alternatif olarak scripts\build_msvc.bat dosyasını çalıştırın.
    echo.
    pause
    exit /b 1
)

pushd "%ROOT%\resources"
"%RC%" app.rc -O coff -o "%BUILD%\app_resources.o"
if errorlevel 1 goto :build_error
popd

pushd "%ROOT%"
"%CC%" ^
  -std=c11 -O2 -Wall -Wextra -Wpedantic ^
  -finput-charset=UTF-8 ^
  -DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX -D_WIN32_WINNT=0x0601 ^
  -Isrc ^
  src\main.c src\network.c src\config.c src\utils.c ^
  build\app_resources.o ^
  -o release\PcPosNetworkManager.exe ^
  -municode -mwindows -static-libgcc ^
  -liphlpapi -lws2_32 -lcomctl32 -lshell32 -ladvapi32 -luser32 -lgdi32
if errorlevel 1 goto :build_error_root
popd

echo.
echo [OK] EXE oluşturuldu:
echo %RELEASE%\PcPosNetworkManager.exe
echo.
pause
exit /b 0

:build_error
popd
echo [HATA] Kaynak dosyası derlenemedi.
pause
exit /b 1

:build_error_root
popd
echo [HATA] C kaynak kodu derlenemedi.
pause
exit /b 1
