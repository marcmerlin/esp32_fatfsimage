# FATFS on ESP32

ESP32 allows you to use part of the flash to store a filesystem. Initially, it
has been used for SPIFFS (a compressed read only filesystem). You can read more about it here:
https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/storage/spiffs.html

Spiffs has issues though, on top of being read only of course, it's pretty bad at doing
seeks across large files (it's slow), and another filesystem like Fat, works better.
This is where FFat(Flash Fat) comes in.

You can see how to use FatFS from this example:
https://github.com/espressif/arduino-esp32/blob/master/libraries/FFat/examples/FFat_Test/FFat_Test.ino
but it assumes that you are creating your files on the device you can begin(true) to format
the filesystem.

If you want to create the filesystem on your computer and send it to your ESP32 (in
my case I have a big collection of Animated Gifs I want to serve), you follow these steps:

1) reformat your ESP32 to have a FatFS partition. You'll probably need this in your tree: 
https://github.com/espressif/arduino-esp32/pull/2623/files
or you can simply git clone https://github.com/marcmerlin/arduino-esp32 which has the change you need

2) create the fatfs image on linux (or you have to make the code here work and build for your OS).
Thanks to lbernstone for building a binary:
```
# replace 3004 with the size of your partition. In the 1/3MB split, the fatfs partition is 
# 0x2EF000 = 3076096 .  3076096/1024 = 3004
fatfsimage -l5 img.ffat 3004 datadir/
```

3) upload the image at the right offset (0x111000 for the 1/3MB split)
```
esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 921600 write_flash  0x111000 img.ffat
```

4) upload and run the arduiono/ffat code to verify the partition list and get a file listing.


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
