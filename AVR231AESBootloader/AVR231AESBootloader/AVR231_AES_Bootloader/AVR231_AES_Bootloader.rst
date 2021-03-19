======================
AVR231: AES Bootloader
======================

This application note provides an example of AES Bootloader on ATmega328PB.
NOTICE: If Atmel Studio encounters the error "Invalid zip package. Index was outside the bounds of the array" when importing this example, please shorten the name of the .atzip file.
This applicaiton requires the ATmega_DFP (released with Atmel Studio) with the version 1.0.106 or later. 
If not, please update to the latest Atmel Studio from http://www.atmel.com/tools/atmelstudio.aspx?tab=overview 

For a more detailed description, please see the applicaiton note available from:

- `AVR231: AES Bootloader <http://www.microchip.com/wwwappnotes/appnotes.aspx?appnote=en591242>`_

========================
SUPPORTED EVALUATION KIT
========================

- `ATmega328PB XPLAINED MINI <https://www.microchip.com/DevelopmentTools/ProductDetails.aspx?PartNO=atmega328pb-xmini>`_

=====
FUSES
===== 
- Enable boot reset vector 
- Set Low Fuse Byte to use External Clock @ 16MHz
- In Atmel Studio 7 this can be done by selecting Tools->Device Programming->Fuses
- CPU runs at 16 MHz

================
RUNNING THE DEMO
================
1. Download the example .atzip file from Atmel|START.
2. If with Atmel Studio 7 (or later), import .atzip file into Atmel Studio 7, File->Import->Atmel Start Project.
   If with IAR 6.80(or later), unzip the .atzip file to locate .eww/.ewp files, and open with IAR IDE.
3. Change settings according to the section "Atmel Studio and IAR Settings" in the application note.
4. Build and flash into supported evaluation board.
5. Run (debug) the code.
