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

./configure --host=$TARGET --prefix=$SYSROOT --with-sysroot=$SYSROOT --with-png=yes --with-zlib=yes ZLIB_CFLAGS="-I$SYSROOT/include" ZLIB_LIBS="-L$SYSROOT/lib -lz" LIBPNG_CFLAGS="-I$SYSROOT/include" LIBPNG_LIBS="-L$SYSROOT/lib -lpng" --enable-shared=no

make
make install
