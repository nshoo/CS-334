# Module 2: Interactive Machines
This module was broken into two tasks. The code for Task 1, where we began experimenting with various forms of analog input, is contained in the file `mod2.py`. The larger project, Task 2, can be found in `TuneCombined.ino`.


## Task 1 Description
Task one had the requirement of having multiple "modes" that could be switched through with some kind of input. We decided that we wanted the modes to modify the tone and content of a message that would be printed to the display. We ended up making the switch control whether a greeting or a goodbye would be shown, and used the button to cycle between different tones.


## Task 2: TunePlayer (generative sequencer in a box)
The software side of the generative sequencer is relatively easy to set up. All of the code is contained in one Arduino file called `TuneCombined.ino`. It can be loaded on the ESP32 as long as the BLEMidi library is installed. The bluetooth midi device created when the device is running will be called "TunePlayer".
