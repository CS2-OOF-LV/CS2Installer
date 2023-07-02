@echo off
if exist "CS2Installer.exe" (
  start "" "CS2Installer.exe" disablemanifest
) else (
  echo CS2Installer.exe not found
  pause
  exit
)