@echo off
set /p "id=Reletive path to your build: "
waf.bat configure -8 -T release --prefix=%id% --build-games=sourcebox