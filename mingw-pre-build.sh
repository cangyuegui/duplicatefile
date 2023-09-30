!/bin/bash

cd /d %~dp0
del /q help_en.h
bin2c help_en.txt --name help > help_en.h


