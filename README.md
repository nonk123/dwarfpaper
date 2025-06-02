# dwarfpaper

![Screenshot of a Windows 10 desktop with a vibrant ASCII-art live wallpaper](.github/assets/screenie-1.png)

A Live wallpaper app that renders classic Dwarf Fortress styled ASCII vomit all over your desktop.

Only works on Windows due to the amount of winapi hacks involved.

## Usage

Run the provided binary, making sure it can read [`9x16.png`](assets/9x16.png) from the working directory.

TODO: publish a binary.

TODO: make this app configurable rather than use hardcoded compile-time constants.

TODO: add multi-monitor support.

## Building

In short, run:

```sh
cmake -S . -B build -G Ninja
cmake --build build
cd build
./dwarfpaper.exe
```
