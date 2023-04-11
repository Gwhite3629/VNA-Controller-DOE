# Introduction:
This is a program which periodically measures S-parameter data from an HP-Agilent HP-8753D VNA located next to the furnace in the MAL. The program is currently in a folder on the MAL14 computer or whichever is directly behind the two VNAs. The program operaties over virtual serial port and connects to the VNAs via a Prologix GPIB controller.

# Usage:
The program can be launched by opening the `main.exe` file. This is statically compiled and should work on any modern Windows computer. The program on launch prompts for a serial device. This is usually COMn where n is found in the Windows device manager. To find this use the search bar and look for "device manager" and look for COM devices, you can check which COM device the GPIB controller is by unplugging it and seeing which device disappears. The program also prompts for a profile. This is described in the section below. The BAUD rate can be set to `9600`. The program can be safely exited at any time by typing "quit" into the command window which is creating on execution.

# Configuring:
The program is configured in the `profiles_data` text file. Do not change the name or extension of this file. The top of the file displays a template which can be used to create and outline a profile. Profiles are created by enclosing the required parameters in a `#` symbol and a number. This number is then entered in the programs start prompt. The parameters are as follow:
|Parameter|Function|
|---------|--------|
|POINTS|This is the number of points the machine records. This should agree with the used recall state.|
|NUM_CHAN|This is the number of channels to record. Values between 1 and 4|
|CHAN n|This is the desired channel to measure for each measurement. This layout is obvious in the examples provided in the current config file.|
|TESTTYPE|This is the type of test. If `TIMED` is chosen then the `TGATHER` field is required. If `INDEF` is chosen then the program will run until the computer is shutdown or the user types "quit".|
|FGATHER|This is the time interval between measurements in minutes. Measurements can take a significant amount of time so this should not be too frequent.|
|TGATHER|This is the amount of time in minutes that the program will run for if the `TIMED` option is chosen.|
|CALKIT|This is the calibration kit used for the given recall state.|
|FSTART|This is the start frequency in MHZ|
|FSTOP|This is the stop frequency in MHZ|

NOTE:
In order to properly use a calibration and recall state the machine must be placed into `USER PRESET` mode. This is done by pressing the green `PRESET` key on the front of the VNA and selecting `USER PRESET` on the menu that pops up. To set your recall state as the user preset go to the `RECALL` menu and rename your state to `UPRESET`. There is a prompt at the bottom of the menu which describes this.

# Concepts:
This program is incredibly simple. It was designed so that automated measurements of samples in the furnace next to the VNAs could be done. This allows unsupervised measurement of the DET at temperature for long periods of time and with high measurement resolution. Beware that if the frequency of measurement is too high or the program is left on too long then there can be a large number of data files. This is because a new file is created each time a measurement is taken and for each channel in the measurement.

The codes function is very simple with a couple timers internally that keep track of the intervals and some simple serial control functions to send commands to the VNA.

# Code:

## main.c

This is the entrypoint and where the user selects device name and baud rate. Not much should be done in this file.

## serial.c

This file contains various serial device control abstractions. Not much should be changed here.

## GPIB_prof.c

This file has all the commands for the GPIB controller.

## commands.c

This file contains most of the commands that are sent to the VNA. There are comments in this file that describe specific operation.

## file.c

This file has eroneous functions and all file controls like writing data files and finding things in the profile file.

## selector.c

This file contains all the actual logic of the program and is responsible for controlling when things happen. There are some locks and threads in here that simplify code flow.

# Using Data:

The data file that is produced is fairly self explanatory. The name of the file contains the measured channel as well as the date and time. Inside the file is a one-line header followed by the magnitude, phase, and frequency of the measurement. Dealing with all these data files can be a hastle. If necessary Grady White can provide assistance with managing these files via a python script.

# Building:

The program can be built on any computer with POSIX compliant C libraries. This was done through MinGW-x64 and gcc. The main.exe file is statically compiled so there shouldn't be any issues. The provided makefile works with windows. There is an alternative version of this program for linux as well as a live-terminal where custom commands can be sent to the VNA. This was used for debugging and isn't very useful.

Single line compilation:
```
gcc -pthread -static -Wall -o main.exe main.c GPIB_prof.c serial.c selector.c file.c commands.c
```

# Acknowledgements:	

This work was supported by the U.S. Department of Energy, Office of Science, Office of Basic Energy Sciences Established Program to Stimulate Competitive Research (EPSCoR) under Award DE SC0020126.

# Disclaimer:  

This code was prepared as an account of work sponsored by an agency of the United States Government. Neither the United States Government nor any agency thereof, nor any of their employees, makes any warranty, express or implied, or assumes any legal liability or responsibility for the accuracy, completeness, or usefulness of any information, apparatus, product, or process disclosed, or represents that its use would not infringe privately owned rights. Reference herein to any specific commercial product, process, or service by trade name, trademark, manufacturer, or otherwise does not necessarily constitute or imply its endorsement, recommendation, or favoring by the United States Government or any agency thereof. The views and opinions of authors expressed herein do not necessarily state or reflect those of the United States Government or any agency thereof.