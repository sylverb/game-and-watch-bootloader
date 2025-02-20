# Changelog

## What's New

### Version 1.0.3
- Update for dual boot (patched OFW) support
- Change diagnostic key to PAUSE/SET to prevent unwanted activation with dual boot keys ("GAME + Left")

## Requirements
To install the bootloader on your Game & Watch, you need a JTAG adapter (such as an ST-Link v2).

You'll need the [gnwmanager](https://github.com/BrianPugh/gnwmanager) application to flash the bootloader onto your console.

## Flashing the bootloader
Download gnw_bootloader.bin
Flash the binary in the internal flash of the Game & Watch using [gnwmanager](https://github.com/BrianPugh/gnwmanager).
```
gnwmanager flash bank1 gnw_bootloader.bin -- start bank1
```
