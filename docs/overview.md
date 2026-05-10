# Dartt Dashboard Overview
This software uses [dartt](https://github.com/ocanath/dartt-protocol) to communicate with peripheral devices in real-time. It works with any dartt device. Layouts are defined in an unambiguous format via .json. This software can also build a .json defining the dartt layout using a .elf file with DWARF debugging information (i.e. debug builds of embedded software). This allows for automatable cross-platform compatibility of dartt layouts.


## Dartt Memory Layout Definition

Dartt memory layouts are defined via .json files. Dartt dashboard injects associated plot and display information into the .json to save persistent settings. Dartt json files can be generated easily via .elf files using DWARF debugging information. Alternatively, dartt json files can be generated 

In order to generate a .json from a Dartt .elf binary:

- Build the embedded device firmware. The peripheral DARTT backing memory must be defined as a `typedef struct`, and the peripheral must route dartt misc messages to a single instance of that `typedef struct`. 