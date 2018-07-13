########Required-Installations for linux##########
----------------------
QT installation:
----------------------
sudo apt-get install qtbase5-dev

----------------------
Sodium installation:
----------------------
download latest version from https://download.libsodium.org/libsodium/releases/
inside the extracted folder:
./configure
make && make check
sudo make install
###########Required-Installations for mac###########
----------------------
QT installation:
----------------------
brew install qt5
cmake -D CMAKE_PREFIX_PATH=/Users/<username>/.../Qt/.../lib/cmake ..

----------------------
Sodium installation:
----------------------
ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)" < /dev/null 2> /dev/null
brew install libsodium
##########Compile-Instructions##########
cd build
cmake ..
make
./IBR_COIN
################Usage######################
- Enklave Keys: At first usage start the programm and start mining for one block. This will start the enclave and thus creates a publicKey.txt in the build folder. Every User of the network (including yourself) has to copy this key into their keys.txt (if not exists create one), seperated by linebreakes.
- IPs: Every network participant has to add the ip-addresses of other users in their config/addresses.csv file (seperated by ",")
- Now you can restart the application and start using this app.

