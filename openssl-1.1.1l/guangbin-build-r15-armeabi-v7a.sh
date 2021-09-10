ARCH=android-arm
SYSROOT=$PWD/../build/armeabi-v7a

export ANDROID_NDK_HOME=`pwd`/../android-toolchain
export PATH=$ANDROID_NDK_HOME/bin:$PATH

./Configure zlib --prefix=$SYSROOT --openssldir=$PWD -march=armv7-a no-shared ${ARCH}
make
make install
