TARGET=armv7a-linux-androideabi
API=21
TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64
SYSROOT=$PWD/../build/armeabi-v7a

export AR=$TOOLCHAIN/bin/llvm-ar
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export AS=$CC
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip

autoreconf -ivf
./configure --host=$TARGET --prefix=$SYSROOT --with-sysroot=$SYSROOT --enable-libpng=yes --disable-arm-simd --disable-arm-neon --disable-arm-iwmmxt --disable-arm-iwmmxt2 PNG_CFLAGS="-I$SYSROOT/include" PNG_LIBS="-L$SYSROOT/lib -lpng" --enable-shared=no

make
make install
