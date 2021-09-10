SYSROOT=$PWD/../build/armeabi
target_host=arm-linux-androideabi

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

./configure --host=$target_host --prefix=$SYSROOT --with-sysroot=$SYSROOT --with-png=yes --with-zlib=yes ZLIB_CFLAGS="-I$SYSROOT/include" ZLIB_LIBS="-L$SYSROOT/lib -lz" LIBPNG_CFLAGS="-I$SYSROOT/include" LIBPNG_LIBS="-L$SYSROOT/lib -lpng" --enable-shared=no
make
make install
