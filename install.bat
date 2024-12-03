cd "%~dp0"

call .\scripts\build.bat

copy ".\scripts\register.bat" ".\woloff\register.bat"

set WOLOFF_INSTALL_PATH=C:\Program Files\woloff

mkdir "%WOLOFF_INSTALL_PATH%"
copy ".\woloff\register.bat" "%WOLOFF_INSTALL_PATH%\register.bat"
copy ".\woloff\woloff.exe" "%WOLOFF_INSTALL_PATH%\woloff.exe"

set WOLOFF_REGISTER_SCRIPT=%WOLOFF_INSTALL_PATH%\register.bat

call "%WOLOFF_REGISTER_SCRIPT%"
