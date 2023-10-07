cd /d %~dp0
cd ..
mkdir libjpeg-turbo-cmakebuild
cd libjpeg-turbo-cmakebuild
cmake -G"MinGW Makefiles" -DENABLE_STATIC=ON -DENABLE_SHARED=OFF -DCMAKE_INSTALL_PREFIX=../libjpeg-turbo-lib ../libjpeg-turbo -B ./
mingw32-make.exe
mingw32-make.exe install