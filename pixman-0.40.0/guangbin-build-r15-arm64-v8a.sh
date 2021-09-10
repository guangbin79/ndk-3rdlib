SYSROOT=$PWD/../build/arm64-v8a
target_host=aarch64-linux-android

export PATH=$PATH:`pwd`/../android-toolchain/bin
export AR=$target_host-ar
export CC=$target_host-gcc
export AS=$target_host-as
export CXX=$target_host-g++
export LD=$target_host-ld
export RANLIB=$target_host-ranlib
export STRIP=$target_host-strip

export CFLAGS="-fPIE -fPIC"
export LDFLAGS="-pie"

autoreconf -ivf
./configure --host=$target_host --prefix=$SYSROOT --with-sysroot=$SYSROOT --enable-libpng=yes --disable-arm-simd --disable-arm-neon --disable-arm-iwmmxt --disable-arm-iwmmxt2 PNG_CFLAGS="-I$SYSROOT/include" PNG_LIBS="-L$SYSROOT/lib -lpng" --enable-shared=no
make
make install
