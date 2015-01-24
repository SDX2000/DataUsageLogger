@echo on

set "DEST_DIR=%programfiles%\SDXTECH\DataUsageLogger"

if not exist "%DEST_DIR%\." (
    md "%DEST_DIR%"
)

copy "%~dp0x64\Release\DataUsageLogger.exe" "%DEST_DIR%\"
sc create "WiFi data usage logger" binPath= "%DEST_DIR%\DataUsageLogger.exe"