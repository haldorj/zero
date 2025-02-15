## Build

Start by cloning the repository with git clone ```--recursive https://github.com/haldorj/zero```.
If the repository was cloned non-recursively previously, use ```git submodule update --init``` to clone the necessary submodules.

1. Head into the Assimp submodule folder ```Zero\Vendor\Assimp```
2. Build with cmake (Open a command prompt and run the command ```cmake .```)
3. Open the Assimp.sln file
4. Build for Release x64
5. Copy the new .dll file from ```Assimp/bin``` and place it into the Zero project folder
