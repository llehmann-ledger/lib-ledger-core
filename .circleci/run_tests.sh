cd ../lib-ledger-core-build
export MOCK_HTTP_FOLDER=`pwd`/../lib-ledger-core/core/test/fixtures/mocked_http/
echo "mocked tests:"
ls $MOCK_HTTP_FOLDER  
~/cmake-3.16.5-Linux-x86_64/bin/ctest -VV -timeout 120
