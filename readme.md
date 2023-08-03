# StarForge

A WIP Map Editor for Super Mario Galaxy

![screenshot](preview.png)

## How to Use

Current StarForge requires a **Dolphin Root** (subject to change) or at the very least a root formatted in a similar way.

### Setting a Root

`Edit > Settings > Root Path Open`, Select your Dolphin Root's `DATA` folder containing `files` and `sys`. 

### Setting up the ObjectDB

Paste the URL for the ObjectDB json you wish to use in the settings menu's `ObjecDB` textbox and hit the update button. 


[Galaxy Database](https://github.com/SunakazeKun/galaxydatabase) is reccomended, and would use the following url:
`https://raw.githubusercontent.com/SunakazeKun/galaxydatabase/main/objectdb.json`


## Compiling

### Setup Repository

```
git clone https://github.com/Astral-C/StarForge.git

cd StarForge

git submodule update --init --recursive
```

**Install the Iconv and CURL libraries for your system**.

### Linux
```
cd StarForge
cmake -S. -Bbuild
cd build
make
```

### Windows

MinGW is reccomended for Windows builds. VisualStudio is untested.

#### Cross Compiling

Cross compiling is currently known to work properly.

Ensure all MinGW Packages for zlib, libpng, glfw, iconv, and curl are installed.
```
cd StarForge
cmake -S. -Bbuild -DCMAKE_TOOLCHAIN_FILE=mingw=w64-x86_64.cmake
cd build
make
```