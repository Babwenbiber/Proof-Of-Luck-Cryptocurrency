cd build
shopt -s extglob 
rm -r -- !(enclave.signed.so|keys.txt|secretData.bin|publicKey.txt)
cd ..
rm keys/*

