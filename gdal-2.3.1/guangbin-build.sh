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

./configure --host=$TARGET --prefix=$SYSROOT --with-sysroot=$SYSROOT --with-png=$SYSROOT --with-jpeg=$SYSROOT --with-curl=$SYSROOT/bin/curl-config --with-webp=$SYSROOT --with-crypto=$SYSROOT

make
make install
