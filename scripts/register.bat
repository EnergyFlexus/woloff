cd "%~dp0"
REG ADD "HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run" /V "woloff" /t REG_SZ /F /D "%~dp0woloff.exe"
