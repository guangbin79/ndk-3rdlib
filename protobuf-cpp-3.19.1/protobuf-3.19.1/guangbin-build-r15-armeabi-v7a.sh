SYSROOT=$PWD/../../build/armeabi-v7a
target_host=arm-linux-androideabi

export PATH=$PATH:`pwd`/../../android-toolchain/bin
export AR=$target_host-ar
export CC=$target_host-gcc
export AS=$target_host-as
export CXX=$target_host-g++
export LD=$target_host-ld
export RANLIB=$target_host-ranlib
export STRIP=$target_host-strip

export CFLAGS="-fPIE -fPIC -march=armv7-a -std=c++11"
export LDFLAGS="-pie -march=armv7-a"

autoreconf -ivf
./configure --host=$target_host --prefix=$SYSROOT --with-sysroot=$SYSROOT --with-zlib-include=$SYSROOT/include --with-zlib-lib=$SYSROOT/lib --enable-shared=no
make
make install
