sudo apt-get update
sudo apt-get install cmake g++ libglew-dev libsdl2-2.0-0 libsdl2-dev libsdl2-image-dev libsdl2-image-2.0-0 libglm-dev libglm-doc libsdl2-ttf-2.0-0 libsdl2-ttf-dev -y
sudo apt-get install libglew2.0 -y
sudo apt-get install libglew1.13 -y
# TODO: find out how to build when package is not provided: libglew2.0 
cmake . -Bbuild
make -C build
./build/TestGame
