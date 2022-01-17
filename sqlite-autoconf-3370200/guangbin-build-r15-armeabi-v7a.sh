target_host=arm-linux-androideabi

export PATH=`pwd`/../android-toolchain/bin:$PATH
export AR=$target_host-ar
export CC=$target_host-gcc
export AS=$target_host-as
export CXX=$target_host-g++
export LD=$target_host-ld
export RANLIB=$target_host-ranlib
export STRIP=$target_host-strip

export CFLAGS="-fPIE -fPIC -march=armv7-a"
export LDFLAGS="-pie -march=armv7-a"

./configure --with-sysroot=`pwd`/../android-toolchain/sysroot --host=$target_host --prefix=$PWD/build/armeabi-v7a --enable-shared=no
make
make install
