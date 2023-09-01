@echo off
set /p "id=Reletive path to your build: "
waf.bat configure -T release --prefix=%id% --build-games=sourcebox