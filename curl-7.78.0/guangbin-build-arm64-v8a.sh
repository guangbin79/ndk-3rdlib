TARGET=aarch64-linux-android
API=21
TOOLCHAIN=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64
SYSROOT=$PWD/../build/arm64-v8a

export AR=$TOOLCHAIN/bin/llvm-ar
export CC=$TOOLCHAIN/bin/$TARGET$API-clang
export AS=$CC
export CXX=$TOOLCHAIN/bin/$TARGET$API-clang++
export LD=$TOOLCHAIN/bin/ld
export RANLIB=$TOOLCHAIN/bin/llvm-ranlib
export STRIP=$TOOLCHAIN/bin/llvm-strip

autoreconf -ivf
./configure --host=$TARGET --prefix=$SYSROOT --with-sysroot=$SYSROOT --with-openssl=$SYSROOT --with-zlib=$SYSROOT --enable-shared=no

make
make install
