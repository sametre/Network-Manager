@echo off
setlocal EnableExtensions
chcp 65001 >nul

echo ==============================================
echo        PC - POS NETWORK MANAGER BUILD
echo ==============================================
echo.

where cl.exe >nul 2>nul
if not errorlevel 1 (
    call "%~dp0scripts\build_msvc.bat"
    exit /b %errorlevel%
)

where x86_64-w64-mingw32-gcc.exe >nul 2>nul
if not errorlevel 1 (
    call "%~dp0scripts\build_mingw.bat"
    exit /b %errorlevel%
)

where gcc.exe >nul 2>nul
if not errorlevel 1 (
    where windres.exe >nul 2>nul
    if not errorlevel 1 (
        call "%~dp0scripts\build_mingw.bat"
        exit /b %errorlevel%
    )
)

rem build_msvc.bat, Visual Studio'yu vswhere ile otomatik bulmayı da dener.
call "%~dp0scripts\build_msvc.bat"
exit /b %errorlevel%
