# dwarfpaper

A live wallpaper app that renders classic Dwarf Fortress styled ASCII vomit all over your desktop. Only works on Windows due to the amount of winapi hacks involved. Below are some screenshots of this in action:

![16:9 screenshot of a Windows 10 desktop with a vibrant ASCII-art pipes-screensaver wallpaper](.github/assets/screenie-1.png)

![4:3 screenshot of a Windows 10 desktop with a vibrant ASCII-art pipes-screensaver wallpaper](.github/assets/screenie-2.png)

## Usage

Download [a prebuilt binary](https://github.com/nonk123/dwarfpaper/releases#latest) and run it. Use the following command-line options to customize the program's behavior:

- `-m`/`--mode`: Select one of the supported modes by name. Only `pipes` is available as of now.
- `-D`/`--debug`: Run the simulation in a separate window rather than as your actual wallpaper. Perfect for one-off tests.

## Building

I'll assume you're using [Visual Studio Code](https://code.visualstudio.com) as it's the go-to text editor for anything code-related nowadays.

You will need [a C compiler](https://winlibs.com/#download-release) and [CMake](https://cmake.org/download). Follow the hyperlinks for respective installation instructions. Don't forget to install [this extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools) to make your life easier, as we'll be using it to build the project.

Clone this repository (or download it as a ZIP & extract somewhere--I don't care) and open its folder in VSCode. On the status bar (at the bottom of the window), click "No toolchain selected" next to a different "CMake: Ready" line. Scan for available toolchains, and make sure the compiler bundle you just downloaded and extracted is discovered; then select it. CMake should now start configuring the build system and spit out a bunch of files inside the `build` subdirectory.

Once configuration is finished, you can press the "Build" button next to the one that took you into selecting a toolchain. You should see that it's completed in the logs panel that popped out the bottom of VSCode window. Navigate to the `build` directory of your project and run the resulting `dwarfpaper.exe` binary. Ta-da! All done.

Or just download [a binary release](https://github.com/nonk123/dwarfpaper/releases#latest) instead--no need to go through all this...
