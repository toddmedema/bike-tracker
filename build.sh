mkdir temp_build

rm firmware.bin
rm temp_build/*.*
cp src/*.* temp_build

cp lib/*.h temp_build
cp lib/*.cpp temp_build

particle compile electron temp_build --saveTo firmware.bin
