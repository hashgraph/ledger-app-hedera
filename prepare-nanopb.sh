NANOPB=../ledger-nanopb

git submodule update --init

pushd proto
protoc --plugin=protoc-gen-nanopb=${NANOPB}/generator/protoc-gen-nanopb --nanopb_out=../src/nanopb_stubs  -I. -I${NANOPB}/generator/proto/ tx.proto
protoc --python_out=../py3_tests  -I. -I${NANOPB}/generator/proto/ tx.proto
popd

if ! [ "$(ls src/nanopb)" ]; then 
	cd src/nanopb
	cp -s ../../ledger-nanopb/*.c .
	cp -s ../../ledger-nanopb/*.h .
	cd ..
fi