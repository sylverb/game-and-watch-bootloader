# Changelog

## What's New

### Version 1.0.1
Added an info screen with application version and some other info that is shown if :
- bank 2 content is incorrect
- booting with GAME button pressed

## Requirements
To install the bootloader on your Game & Watch, you need a JTAG adapter (such as an ST-Link v2).

You'll need the [gnwmanager](https://github.com/BrianPugh/gnwmanager) application to flash the bootloader onto your console.

## Flashing the bootloader
Download gnw_bootloader.bin
Flash the binary in the internal flash of the Game & Watch using [gnwmanager](https://github.com/BrianPugh/gnwmanager).
```
gnwmanager flash bank1 gnw_bootloader.bin -- start bank1
```
