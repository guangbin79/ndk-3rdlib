TARGET=armv7a-linux-androideabi
API=19
TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64
SYSROOT=$PWD/../build/armeabi-v7a

export AR=$TOOLCHAIN/bin/llvm-ar
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export AS=$CC
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip

./configure --host=$TARGET --prefix=$SYSROOT --with-sysroot=$SYSROOT --enable-png --enable-gobject=no FREETYPE_CFLAGS="-I$SYSROOT/include/freetype2" FREETYPE_LIBS="-L$SYSROOT.lib -lfreetype" png_REQUIRES="libpng" png_CFLAGS="-I$SYSROOT/include" png_LIBS="-L$SYSROOT/lib -lpng" pixman_CFLAGS="-I$SYSROOT/include/pixman-1" pixman_LIBS="-L$SYSROOT/lib -lpixman-1"

make
make install
