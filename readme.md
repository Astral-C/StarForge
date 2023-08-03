## StarForge

A WIP Map Editor for Super Mario Galaxy

![screenshot](preview.png)

### How to Use

Current StarForge requires a **Dolphin Root** (subject to change) or at the very least a root formatted in a similar way.

#### Setting a Root

`Edit > Settings > Root Path Open`, Select your Dolphin Root's `DATA` folder containing `files` and `sys`. 

#### Setting up the ObjectDB

Paste the URL for the ObjectDB you wish to use in the settings menu's `ObjecDB` textbox and hit the update button. 

[Galaxy Database](https://github.com/SunakazeKun/galaxydatabase) is reccomended.
For example, to use Galaxy Database, you would paste `https://raw.githubusercontent.com/SunakazeKun/galaxydatabase/main/objectdb.json` into the `ObjectDB` textbox.

### Compiling

Clone repository with `git clone https://github.com/Astral-C/StarForge.git`

Initialize submodules `git submodule update --init --recursive`

**Requires Iconv and CURL libraries**.


#### Linux
```
cd StarForge
cmake -S. -Bbuild
cd build
make
```

#### Windows

MinGW is reccomended for Windows builds, untested with VisualStudio (if you get that working, open an issue and let me know!).

For this the build process is the same as [Linux](#Linux)

Currently all Windows releases are cross compiled, this is the reccomended way to compile for Windows. 
Ensure all MinGW Packages for zlib, libpng, glfw, iconv, and curl are installed.

```
cd StarForge
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=mingw=w64-x86_64.cmake
cd build
make
```