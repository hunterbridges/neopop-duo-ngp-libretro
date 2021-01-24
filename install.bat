@echo off
cd %~dp0

@echo on
copy /Y neopop_duo_ngp_libretro.dll "%userprofile%\AppData\Roaming\RetroArch\cores"
copy /Y neopop_duo_ngp_libretro.info "%userprofile%\AppData\Roaming\RetroArch\info"

@echo off
pause