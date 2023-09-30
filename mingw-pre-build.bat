cd /d %~dp0
del /q help_en.h
bin2c help_en.txt --name help > help_en.h
g++ *.c *.cpp -Wall -static -O3 -std=c++17 -o duplicatefile.exe -fno-rtti -fno-exceptions
strip duplicatefile.exe
