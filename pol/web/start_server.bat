@echo off
chcp 65001 >nul
cd /d "%~dp0"

for /f "delims=" %%a in ('powershell -NoProfile -Command "(Get-NetIPAddress -AddressFamily IPv4 -ErrorAction SilentlyContinue | Where-Object { $_.InterfaceAlias -notmatch 'Loopback' -and $_.IPAddress -notmatch '^169\.' } | Select-Object -First 1).IPAddress" 2^>nul') do set IP=%%a
if not defined IP set IP=127.0.0.1

echo.
echo   ============================================
echo   На телефоне (в той же Wi-Fi) в браузере
echo   откройте адрес:
echo.
echo      http://%IP%:8080
echo.
echo   ============================================
echo.
echo   Сервер запущен. Закройте окно для остановки.
echo.

python -m http.server 8080 --bind 0.0.0.0 2>nul
if errorlevel 1 (
    echo Python не найден. Установите Python с python.org
    echo.
    echo ИЛИ используйте Node.js:
    echo npx serve . -p 8080 --listen 0.0.0.0:8080
    pause
) else (
    echo.
    echo Если на телефоне не открывается, проверьте:
    echo 1. ПК и телефон в одной Wi-Fi сети?
    echo 2. Файрвол Windows может блокировать порт 8080
    echo    (разрешите в настройках Windows Firewall)
    echo.
    pause
)
