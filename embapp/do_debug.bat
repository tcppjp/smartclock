call ../../do_path.bat
make ocd
make db
taskkill /F /IM openocd.exe
