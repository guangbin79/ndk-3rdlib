ARCH=android-arm
SYSROOT=$PWD/../build/armeabi

export ANDROID_NDK_HOME=`pwd`/../android-toolchain
export PATH=$ANDROID_NDK_HOME/bin:$PATH

./Configure zlib --prefix=$SYSROOT --openssldir=$PWD no-shared ${ARCH}
make
make install
