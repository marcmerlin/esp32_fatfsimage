# FATFS on ESP32

This page is to help you setup a ffat image on your computer and upload it to your ESP32  
Blog post: http://marc.merlins.org/perso/arduino/post_2019-03-30_Using-FatFS-FFat-on-ESP32-Flash-With-Arduino.html
If you're happy formatting FFAT and creating files directly from ESP32 code, you can look at those pages too:
- https://www.dfrobot.com/blog-1179.html
- https://techtutorialsx.com/2018/10/06/esp32-arduino-fat-file-system/


## FatFS on ESP32 Howto:
ESP32 allows you to use part of the flash to store a filesystem. Initially, it
has been used for SPIFFS (a compressed read only filesystem). You can read more about it here:
https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/spiffs.html

Spiffs has issues though, on top of being read only of course, it's pretty bad at doing
seeks across large files (it's slow), and another filesystem like Fat, works better.
This is where FFat(Flash Fat) comes in.  
As of 2020 there is now also LittleFS. I have not used it, but I assume it's a better version
of SPIFFS that doesn't take as much RAM as FatFS

You can see how to use FatFS from this example:
https://github.com/espressif/arduino-esp32/blob/master/libraries/FFat/examples/FFat_Test/FFat_Test.ino
but it assumes that you are creating your files on the device you can begin(true) to format
the filesystem.

If you want to create the filesystem on your computer and send it to your ESP32 (in
my case I have a big collection of Animated Gifs I want to serve), you follow these steps:


## This code is mostly obsolete Use arduino-esp32fs-plugin instead

If you are on linux, you may still be happy with the instructions below, but after I wrote all this, 
a better solution came up: https://github.com/lorol/arduino-esp32fs-plugin
I recommend you install this instead and then grab a mkfatfs binary (it's a bit of a pain to build) from:
- https://github.com/labplus-cn/mkfatfs/releases/tag/v2.0.1
- https://github.com/labplus-cn/mkfatfs/releases/tag/v1.0  
and grab esp32fs.jar from https://github.com/lorol/arduino-esp32fs-plugin/releases

But if you'd like to create your FatFS image from a makefile and upload it via esptool, then my 
branch does this, read on.


### Create a FFAT partition on ESP32
reformat your ESP32 to have a FatFS partition. You'll probably need this in your tree: 
https://github.com/espressif/arduino-esp32/pull/2623/files
or you can simply git clone https://github.com/marcmerlin/arduino-esp32 which has the change you need

You will need to pay attention to the last line for both the offset (which you copy as is 
in esptool) and the size, which you need to convert to decimal and divide by 1024 for KB: https://github.com/espressif/arduino-esp32/blob/170d204566bbee414f4059db99168974c69d166e/tools/partitions/noota_3gffat.csv
```
ffat,     data, fat,     0x111000,0x2EF000,
```
actually the eeprom partition was removed in newer versions of esp32-arduino, so now it should be:
```
ffat,     data, fat,     0x110000,0x2F0000
```

### Create a FatFS image
To create the fatfs image on linux (or you have to make the code here work and build for your OS).
Thanks to lbernstone for building a binary:
```
# replace 3004 with the size of your partition. In the 1/3MB split, the fatfs partition is 
# 0x2EF000 = 3076096 .  3076096/1024 = 3004
fatfsimage -l5 img.ffat 3004 datadir/
# with newer esp32-arduino, it's now 0x2F0000 = 3080192. 3080192/1024 = 3008
fatfsimage -l5 img.ffat 3008 datadir/
```

### Upload the FatFS image to your ESP32
To upload the image at the right offset (0x111000 for the 1/3MB split)
```
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash  0x111000 img.ffat
or with newer esp32-arduino:
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash  0x110000 img.ffat
```

### FFat test code.
upload and run the arduino/ffat code to verify the partition list and get a file listing.
https://github.com/marcmerlin/esp32_fatfsimage/blob/master/arduino/ffat/ffat.ino
```
partition addr: 0x010000; size: 0x100000; label: app0
partition addr: 0x009000; size: 0x005000; label: nvs
partition addr: 0x00e000; size: 0x002000; label: otadata
partition addr: 0x110000; size: 0x001000; label: eeprom
partition addr: 0x111000; size: 0x2ef000; label: ffat

Trying to mount ffat partition if present
File system mounted
Total space:    3018752
Free space:     1429504
Listing directory: /gifs64
  FILE: /gifs64/ani-bman-BW.gif	SIZE: 4061
  FILE: /gifs64/087_net.gif	SIZE: 46200
(...)
```

### Memory use
The FFAT module uses 8KB plus 4KB per concurrent file that can be opened. By default, it allows 10 files to be opened, which means it uses 48KB. IF you want to reduce its memory use, you can tell it to only support one file, and you will save 36KB, leaving you with only 12KB used.
```
if (!FFat.begin(0, "", 1)) die("Fat FS mount failed. Not enough RAM?");
```


Original project README listed below in case you need to (re)build fatfsimage

## fatfsimage
A utility to create and populate FATFS image files on the host that can then
be flashed to the ESP32.

All you need to do is add it to your components directory and
run "make menuconfig" to set the required parameters (look for
FATFSIMAGE Configuration).  Once done, "fatfsimage" will be built
the next time you build your project.

When ready, you can then do "make fat" to create the image or
"make fat-flash" to flash it.

The required settings are:

#### Source directory
This is the path to the directory containing any files or other directories
you want to copy to the image.

#### Disk image size in KB
You specify the size of the disk image in KB.  Make sure this matches the
size of your partition.

#### Image name
The filename for the image file.

#### FATFS Partition offset
Specify the start of the partition.  Make sure it matches the actual
partition.

### Usage

You may also run the utility manually if you like:

```
Usage: build/fatfsimage/fatfsimage [-h] [-l <level>] <image> <KB> <paths> [<paths>]...
Create and load a FATFS disk image.

  -h, --help                display this help and exit
  -l, --log=<level>         log level (0-5, 3 is default)
  <image>                   image file name
  <KB>                      disk size in KB
  <paths>                   directories/files to load
```
