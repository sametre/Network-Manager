@echo off
setlocal EnableExtensions
chcp 65001 >nul

set "ROOT=%~dp0.."
set "RELEASE=%ROOT%\release"
set "PACKAGE=%ROOT%\PcPosNetworkManager-v1.0.0-Windows-x64.zip"

if not exist "%RELEASE%\PcPosNetworkManager.exe" (
    echo [HATA] Önce BUILD_EXE.bat ile EXE oluşturulmalıdır.
    pause
    exit /b 1
)

copy /Y "%ROOT%\config.example.ini" "%RELEASE%\config.ini" >nul
copy /Y "%ROOT%\README.md" "%RELEASE%\README.md" >nul
if not exist "%RELEASE%\logs" mkdir "%RELEASE%\logs"

powershell -NoProfile -ExecutionPolicy Bypass -Command ^
  "if (Test-Path '%PACKAGE%') { Remove-Item '%PACKAGE%' -Force }; Compress-Archive -Path '%RELEASE%\*' -DestinationPath '%PACKAGE%' -CompressionLevel Optimal"

if errorlevel 1 (
    echo [HATA] Release ZIP paketi oluşturulamadı.
    pause
    exit /b 1
)

echo [OK] Paket oluşturuldu:
echo %PACKAGE%
pause
