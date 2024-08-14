# dc32_badge custom firmware
Custom firmware built from scratch for the Defcon 32 badge. Work-in-progress.

## References

### Pinout

```c
#define PIN_RAM_NCS		0
#define PIN_TOUCHINT	1
#define PIN_SDA			2
#define PIN_SCL			3
#define PIN_WS2812		4

#define PIN_LCD_DnC		5
#define PIN_LCD_DO		6
#define PIN_LCD_SCK		8
#define PIN_LCD_CS		9
#define PIN_LCD_BL		10

#define PIN_SELF_PWR	11

#define PIN_SD_MISO		12
#define PIN_SD_NCS		13
#define PIN_SD_SCK		14
#define PIN_SD_MOSI		15

#define PIN_BTN_R		16
#define PIN_BTN_D		17
#define PIN_BTN_U		18
#define PIN_BTN_L		19
#define PIN_BTN_B		20
#define PIN_BTN_A		21
#define PIN_BTN_START	22
#define PIN_BTN_SEL		23
#define PIN_BTN_CENTER	24

#define PIN_IRDA_OUT	26		// 26 is the built in IR. 28 is for super-power-IR attachment
#define PIN_IRDA_IN		27
#define PIN_IRDA_SD		7

#define PIN_SPQR		25

// PWM use
#define BACKLITE_PWM_INDEX			5

// DMA USE
#define DISP_DMA_FIRST				0	// uses 2
#define SD_DMA_FIRST				2	// uses 2
#define WS2812_DMA					4	// uses 1
#define I2C_DMA_FIRST				5	// uses 2

// PIO use:
#define DISP_PIO_IDX				0
#define DISP_PIO_SM					0	// of 4
#define DISP_PIO_FIRST_USED_PC		0	// uses 2 instrs

#define I2C_PIO_IDX					1
#define I2C_PIO_SM					0	// of 4
#define I2C_PIO_INT_IRQ_IDX			0	// of 8
#define I2C_PIO_EXT_IRQ_IDX			0	// of 2
#define I2C_PIO_FIRST_USED_PC		0	// uses 22 instrs

#define SIR_PIO_IDX					1
#define SIR_PIO_SM					1	// of 4
#define SIR_PIO_EXT_IRQ_IDX			1	// of 2
#define SIR_PIO_FIRST_USED_PC		22	// uses 9 instrs

#define WS2812_PIO_IDX				1
#define WS2812_PIO_SM				2	// of 4
#define WS2812_PIO_FIRST_USED_PC	31	// uses 1 instr
```

## Tasks

### Upload-Release
Directory: ./build
Requires: build-release

Builds, then copies the firmware with speed optimisations to the RP2350.

```bash
export PICO_DIR=`findmnt -S LABEL=RP2350 -o TARGET -fn`
cp ./dc32_badge.uf2 $PICO_DIR
```

### Upload-Debug
Directory: ./build
Requires: build-debug

Builds, then copies the firmware with debug optimisations to the RP2350.

```bash
export PICO_DIR=`findmnt -S LABEL=RP2350 -o TARGET -fn`
cp ./dc32_badge.uf2 $PICO_DIR
```

### Build-Release
Directory: ./build

Builds the firmware optimized for size.

```bash
cmake -DCMAKE_BUILD_TYPE=Release -DPICO_PLATFORM=rp2350 ..
make -j4 dc32_badge
```

### Build-Debug
Directory: ./build

Builds the keyboard firmware with development outputs.

```bash
cmake -DCMAKE_BUILD_TYPE=Debug -DPICO_PLATFORM=rp2350 .. 
make -j4 dc32_badge
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
