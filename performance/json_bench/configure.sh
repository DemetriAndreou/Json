set -x
cd ../makeHugeJson/
cmake .
make
makeHugeJson 
mv huge.json ../json_bench

cd -
mkdir Json
cp ../../Json.* Json
./clone.sh
cmake .
make -j4
