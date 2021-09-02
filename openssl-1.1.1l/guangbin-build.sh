ANDROID_API=19
ARCH=android-arm
SYSROOT=$PWD/../build/armeabi-v7a

export PATH=$ANDROID_NDK_HOME/toolchains/llvm/prebuilt/linux-x86_64/bin:$ANDROID_NDK_HOME/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64/bin:$PATH

./Configure zlib --prefix=$SYSROOT --openssldir=$PWD ${ARCH} -D__ANDROID_API__=$ANDROID_API
make
make install
