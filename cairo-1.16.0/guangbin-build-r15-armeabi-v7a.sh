SYSROOT=$PWD/../build/armeabi-v7a
target_host=arm-linux-androideabi

export PATH=$PATH:`pwd`/../android-toolchain/bin
export AR=$target_host-ar
export CC=$target_host-gcc
export AS=$target_host-as
export CXX=$target_host-g++
export LD=$target_host-ld
export RANLIB=$target_host-ranlib
export STRIP=$target_host-strip

export CFLAGS="-fPIE -fPIC -march=armv7-a"
export LDFLAGS="-pie -march=armv7-a"

autoreconf -ivf
./configure --host=$target_host --prefix=$SYSROOT --with-sysroot=$SYSROOT --enable-png --enable-gobject=no FREETYPE_CFLAGS="-I$SYSROOT/include/freetype2" FREETYPE_LIBS="-L$SYSROOT.lib -lfreetype" png_REQUIRES="libpng" png_CFLAGS="-I$SYSROOT/include" png_LIBS="-L$SYSROOT/lib -lpng" pixman_CFLAGS="-I$SYSROOT/include/pixman-1" pixman_LIBS="-L$SYSROOT/lib -lpixman-1" --enable-shared=no
make
make install
