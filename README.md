# slab-firmware
Experimental firmware for the slab keyboard. Hardware under development, to be released later.

## Tasks

### Upload-debug
Directory: ./build
Requires: build-debug

Uploads the firmware to the keyboard with debug outputs.

```bash
cp ./slab.uf2 $(findmnt -S LABEL=RPI-RP2 -o TARGET -fn)
```

### Build
Directory: ./build

Builds the keyboard firmware.

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4 slab
```

### Build-debug
Directory: ./build

Builds the keyboard firmware with development outputs.

```bash
cmake -DCMAKE_BUILD_TYPE=Debug .. 
make -j4 slab
cp compile_commands.json ../ # Copies the autocomplete information for ccls.
```

### Clean
Cleans the build directory for a fresh build.

```bash
rm -rf ./build
mkdir build
```

### Init-submodules

Fetches submodules for use in the project.
```bash
git submodule update --init --recursive
```
