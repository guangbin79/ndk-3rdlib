cd zlib-1.2.11 && ./guangbin-build.sh && cd ..
cd libpng-1.6.37 && ./guangbin-build.sh && cd ..
cd freetype-2.11.0 && ./guangbin-build.sh && cd .. && sed -i 's/ =[^|]*\/..\/build\/armeabi-v7a\/lib\/libpng.la/ =\/lib\/libpng.la/' build/armeabi-v7a/lib/libfreetype.la
cd jpegsrc.v9d/jpeg-9d && ./guangbin-build.sh && cd ../..
cd openssl-1.1.1l && ./guangbin-build.sh && cd ..
cd curl-7.78.0 && ./guangbin-build.sh && cd ..
cd pixman-0.40.0 && ./guangbin-build.sh && cd ..
cd libwebp-1.2.1 && ./guangbin-build.sh && cd ..
cd cairo-1.16.0 && ./guangbin-build.sh && cd ..
cd proj-5.1.0 && ./guangbin-build.sh && cd ..
cd gdal-2.3.1 && ./guangbin-build.sh && cd ..
