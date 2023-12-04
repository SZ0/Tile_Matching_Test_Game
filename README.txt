CREDITS:

IMAGES:
Background image edited, but original by: <a href="https://pixabay.com/users/popcorn_pix-7815162/?utm_source=link-attribution&utm_medium=referral&utm_campaign=image&utm_content=4874436">Paula Wood</a> from <a href="https://pixabay.com//?utm_source=link-attribution&utm_medium=referral&utm_campaign=image&utm_content=4874436">Pixabay</a>
Pinecone image edited, but original by: <a href="https://www.flaticon.com/free-icons/pinecone" title="pinecone icons">Pinecone icons created by Freepik - Flaticon</a>
Font by https://www.fontspace.com/ggbotnet



I use CMake to support building on different platforms, I tested it working on: 
- VISUAL STUDIO ON WINDOWS,
- LINUX MINT 18.3 and
- DEBIAN 9.4

(NOTE: If you will try different ways to build, I recommend deleting the 'build' folder betwen them.)


################## TO BUILD WITH VISUAL STUDIO ON WINDOWS: ##################
LAZY WAY:
- Have Visual Studio 2017 installed,
- double click the file: 'MakeMeAGameVisualStudio2017.bat' and 
- press 'F5' (after Visual Studio loads).
The game should now be served.
If it worked, this happened: bat ran CMake creating a Visual Studio 2017 solution and opened.
(NOTE: I only had access to Visual Studio 2017, but I added (DISCLAMER: and did NOT test) 'MakeMeAGameVisualStudio2012.bat' and 'MakeMeAGame.bat' that you can try for giggles to see if you can avoid installing Visual Studio 2017.)

THE WAY INTENDED:
- Download latest graphical CMake from here: https://cmake.org/download/
- Run CMake.
- Point the 'Where is the source code' to the folder containing the top level CMakeLists.txt (the 'SOURCE' folder).
- Point to 'Where to build the binaries' to some empty folder (eg. 'SOURCE/build') folder.
- Press configure. A window will pop up.
- In the new window for 'Specify the generator for this project' pick a 32 bit Visual Studio version (tested working on Visual Studio 15 2017).
- Leave the 'Use default native compilers' radio button clicked and press the Finish button.
- Wait until 'Configuring done' is prompted in the output panel below the Configure button.
- Repeat pressing Configure after 'Configuring done' is prompted until there are no more red options in the central panel. (Only one more time should do.)
- Press Generate and wait for 'Generating done' to be prompted below.
- Press the Open Project button (or you can find the generated Visual Studio solution in the build folder you specified).
- The Visual Studio solution will contain three projects, build the 'TestGame' project and run it. This should start the game.


################## TO BUILD WITH MAKE ON LINUX MINT 18.3 or DEBIAN 9.4: ##################
LAZY WAY:
- Make sure you have graphics card drivers installed and up to date.
- Open a command-line interface and 'cd' to inside the 'SOURCE' directory.
- Run './linuxMakeMeAGame.sh' (without quotation marks). (If you can not run this file, run 'chmod +x ./linuxMakeMeAGame.sh' first.)
- Type in your password and press ENTER to allow the script installing cmake, g++, various libraries and their dependencies.

INFORMED WAY (if you don't want to mess up your Linux installation):
- Make sure you have graphics card drivers installed and up to date.
- Run the following command or make sure the mentioned packages are installed: 'sudo apt-get install libglew2.0 libglew-dev libsdl2-2.0-0 libsdl2-dev libsdl2-image-dev libsdl2-image-2.0-0 libglm-dev libglm-doc libsdl2-ttf-2.0-0 libsdl2-ttf-dev -y'
  (NOTE: Some of these packages may be available under your distribution only with different version: eg. libglew2.0 is available on Debian 9.4, but on Linux Mint 18.3 it is not. However, the version available is a dependency of libglew-dev and will be installed with it by your package manager.)
  (Note: if you install sdl both via source-code-cmake and your package manager, you may get redifinition errors when adding SDL_image.h, keep only one installation at the time.)
- 'cd' to 'SOURCE' directory.
- Run: 'cmake . -Bbuild'
- Run: 'make -C build'
- Run the game with: './build/TestGame'
(NOTE: You can also use the graphical cmake: cmake-gui, if not installed yet, use: "sudo apt-get install cmake-gui", then follow the same steps as for Windows, but use the default generator instead of picking Visual Studio 2017 and run make in the build directory.)
