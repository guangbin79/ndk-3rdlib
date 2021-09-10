if [ -z "$1" ]; then
    echo e.g.: "./build.sh <path: ANDROID_NDK_HOME>"
    exit 1
fi
export ANDROID_NDK_HOME=$1

./standalone_toolchain-armeabi.sh
cd zlib-1.2.11 && ./guangbin-build-r15-armeabi.sh && cd ..
cd libiconv-1.16 && ./guangbin-build-r15-armeabi.sh && cd ..
cd sqlite-3.37.0 && ./guangbin-build-r15-armeabi.sh && cd ..
cd libpng-1.6.37 && ./guangbin-build-r15-armeabi.sh && cd ..
cd freetype-2.11.0 && ./guangbin-build-r15-armeabi.sh && cd .. && sed -i 's/ =[^|]*\/..\/build\/armeabi\/lib\/libpng.la/ =\/lib\/libpng.la/' build/armeabi/lib/libfreetype.la
cd jpegsrc.v9d/jpeg-9d && ./guangbin-build-r15-armeabi.sh && cd ../..
cd openssl-1.1.1l && ./guangbin-build-r15-armeabi.sh && cd ..
cd curl-7.78.0 && ./guangbin-build-r15-armeabi.sh && cd ..
cd pixman-0.40.0 && ./guangbin-build-r15-armeabi.sh && cd ..
cd libwebp-1.2.1 && ./guangbin-build-r15-armeabi.sh && cd ..
cd cairo-1.16.0 && ./guangbin-build-r15-armeabi.sh && cd ..
cd proj-5.1.0 && ./guangbin-build-r15-armeabi.sh && cd ..
cd gdal-2.3.1 && ./guangbin-build-r15-armeabi.sh && cd ..

rm -rf android-toolchain
