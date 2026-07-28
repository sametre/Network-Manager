@echo off
setlocal EnableExtensions
chcp 65001 >nul

set "ROOT=%~dp0.."
set "BUILD=%ROOT%\build"
set "RELEASE=%ROOT%\release"

if not exist "%BUILD%" mkdir "%BUILD%"
if not exist "%RELEASE%" mkdir "%RELEASE%"

where cl.exe >nul 2>nul
if errorlevel 1 (
    set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
    if exist "%VSWHERE%" (
        for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%i"
        if defined VSROOT call "%VSROOT%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64
    )
)

where cl.exe >nul 2>nul
if errorlevel 1 (
    echo.
    echo [HATA] Visual Studio C++ Build Tools bulunamadı.
    echo Visual Studio Installer üzerinden "Desktop development with C++" bileşenini kurun.
    echo.
    pause
    exit /b 1
)

pushd "%ROOT%\resources"
rc.exe /nologo /fo "%BUILD%\app_resources.res" app.rc
if errorlevel 1 goto :resource_error
popd

pushd "%BUILD%"
cl.exe /nologo /std:c11 /W4 /O2 /utf-8 ^
  /DUNICODE /D_UNICODE /DWIN32_LEAN_AND_MEAN /DNOMINMAX /D_WIN32_WINNT=0x0601 ^
  /I"%ROOT%\src" ^
  "%ROOT%\src\main.c" ^
  "%ROOT%\src\network.c" ^
  "%ROOT%\src\config.c" ^
  "%ROOT%\src\utils.c" ^
  "%BUILD%\app_resources.res" ^
  /Fe:"%RELEASE%\PcPosNetworkManager.exe" ^
  /link /SUBSYSTEM:WINDOWS /MACHINE:X64 ^
  iphlpapi.lib ws2_32.lib comctl32.lib shell32.lib advapi32.lib user32.lib gdi32.lib
if errorlevel 1 goto :compile_error
popd

echo.
echo [OK] EXE oluşturuldu:
echo %RELEASE%\PcPosNetworkManager.exe
echo.
pause
exit /b 0

:resource_error
popd
echo [HATA] Windows resource dosyası derlenemedi.
pause
exit /b 1

:compile_error
popd
echo [HATA] C kaynak kodu derlenemedi.
pause
exit /b 1
