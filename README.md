# slab-firmware
Firmware for the open-source slab keyboard. Work-in-progress.

## Tasks

### Upload-Release
Directory: ./build
Requires: build-release

Builds, then copies the firmware with debug optimisations to the RP2040.

```bash
export PICO_DIR=`findmnt -S LABEL=RPI-RP2 -o TARGET -fn`
cp ./slab.uf2 $PICO_DIR
```

### Upload-Debug
Directory: ./build
Requires: build-debug

Builds, then copies the firmware with speed optimisations to the RP2040.

```bash
export PICO_DIR=`findmnt -S LABEL=RPI-RP2 -o TARGET -fn`
cp ./slab.uf2 $PICO_DIR
```

### Build-Release
Directory: ./build

Builds the keyboard firmware optimized for size.

```bash
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j4 slab
```

### Build-Debug
Directory: ./build

Builds the keyboard firmware with development outputs.

```bash
cmake -DCMAKE_BUILD_TYPE=Debug .. 
make -j4 slab
cp compile_commands.json ../ # Copies the autocomplete information for ccls.
```

### Clean-Development

Resets development files for a clean build.

```bash
rm -rf ./build
mkdir build
```

### Initialize-Development

Fetches submodules and creates the build folder.

```bash
git submodule update --init --recursive
mkdir -p build
```

### Update-Submodules

Updates the submodules to the latest commit.

```bash
git submodule update --remote
```
