@echo on

set "DEST_DIR=%programfiles%\SDXTECH\DataUsageLogger"

if not exist %DEST_DIR%\. (
    md "%DEST_DIR%"
)

copy x64\Release\DataUsageLogger.exe "%DEST_DIR%"
sc create "WiFi data usage logger2" binPath= "%DEST_DIR%\DataUsageLogger.exe"