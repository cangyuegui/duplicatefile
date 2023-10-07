cd /d %~dp0
cd ..
IF EXIST "libjpeg-turbo-cmakebuild" (
    EXIT
)
mkdir libjpeg-turbo-cmakebuild
cd libjpeg-turbo-cmakebuild
cmake -G"MinGW Makefiles" -DENABLE_STATIC=ON -DENABLE_SHARED=OFF   -DCMAKE_DEBUG_POSTFIX=d -DCMAKE_INSTALL_PREFIX=../libjpeg-turbo-lib ../libjpeg-turbo -B ./ -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="-O3 -fno-rtti -fno-exceptions"
mingw32-make.exe -j4
mingw32-make.exe install