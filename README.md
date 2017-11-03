# isoident
This module identifies participants and message on the ISOBUS network utilizing the address-claim procedure defined in ISO 11783-5. The detected participants and their messages will be saved in the configfile. Furthermore it can be an addition to the existing amcanlogger to adjust the CAN logging profile dynamically to the active devices.

TODO:
* ✓ Clean up (~~Memory Leaks~~, ~~CAN Filter~~, ~~Exceptions in datasets structure - utils_parse, line 408~~)
* ✓ J1939 Integration (~~PGN~~, ~~SPN~~, ~~Manufacturer~~)
* ✓ Add log file/ debug option
* ✓ Additional Data in isoident.xml (~~lastSA~~, ~~status~~, ~~lastSeen~~, ~~class and industry in isoident.xml~~)
* ✓ Identify messages from unknown sender (~~Restructure isoident.xml~~, ~~add method to handle unknown messages~~,~~isoident.xml generator~~)
* ✓ Optimize Canlogger.xml-handling (~~No overwriting, ...~~)
* ✓ Edit xml via web (~~Test nginx~~, ~~test Xonomy~~, ~~create basic docSpec~~ , ~~create web frame~~, ~~test on box~~) 
* ✓ Update config section
* Create database of signal details

## Build

The module uses the Mini-XML library which is included in the repo or can be found there: https://michaelrsweet.github.io/mxml/mxml.html

Compiliation can be done for the local machine, manually via:

`$ gcc -D _BSD_SOURCE -o isoident isoident.c utils_general.c utils_parse.c utils_xml.c -lmxml -pthread -lm -std=c99`

or

`$ gcc -D _BSD_SOURCE -o isoident isoident.c utils_general.c utils_parse.c utils_xml.c -lmxml -pthread -lm -std=c99 -fplan9-extensions -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable`

or via the included Makefile, executing:

`$ make`

Cross - Compiling for other Platforms, i.e. for an Cortex A7 system can be done utilizing the gcc-arm-linux-gnueabi toolchain, doing:

`$ arm-linux-gnueabi-gcc -march=armv7 -D _BSD_SOURCE -o isoident-armv7 isoident.c utils_general.c utils_parse.c utils_xml.c mxml-2.10/mxml-attr.c mxml-2.10/mxml-entity.c mxml-2.10/mxml-file.c mxml-2.10/mxml-get.c mxml-2.10/mxml-index.c mxml-2.10/mxml-node.c mxml-2.10/mxml-search.c mxml-2.10/mxml-private.c mxml-2.10/mxml-set.c mxml-2.10/mxml-string.c -pthread -lm -std=c99 -fplan9-extensions -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable -mthumb -static`

## Config

The config or library file includes signals, messages, devices and evaluation parameters. Everytime a device or message is seen on the bus, the software writes an entry to the config file.
The default name is isoident.xml.

The file consists out of 4 sections:

###<signallib/>
The signallib consists standalone signals, that have to be put in manually. If the "log" attribute is set to "1" the signal is passed to the log. This is an example for an entry:

`<signal spn="174" name="Engine Fuel Temperature 1" log="0" start="8" len="8" end="0" fac="0" offs="0" min="-1000" max="1000" type="1" unit="°C" ddi="" />`
All the attributes have to be set by hand or copied into the configfile.


`<messagelib/>`
Everytime the software detects a message, whose source address (the sending device) has not been identified, a new entry will be made in here. Again, with setting the "log" attribute of a signal in here will enable logging. The signals are added according to the standard that defines the PGN. The database of this can be found in datasets/ . An example entry is as followed:
`<message pgn="60928" name="Address Claimed Message" type="ISO11783"> <signal spn="2848" name="NAME of Controller Application (for address claimed)" log="0" start="0" len="64" end="0" fac="0" offs="0" min="-800000" max="800000" type="" unit="" ddi="" /></message>`

The "type" attribute of the message node describes the standard which the message is defined in. It can be J1939 or ISO11783.


`<devicellib/>`
The devicelib includes all identified devices with information about the last active date and manufacturer, function and class. As child nodes, all messages (and with that signals) that came from this device are entered.
`</device><device UUID="315891700" manufacturer="Fleetguard" function="Suspension Control - Drive Axle" class="POWERED AUXILIARY DEVICES" industry="Agriculture and Forestry Equipment" lastClaim="2017-11-03-21:29" lastSA="38" status="online"><message pgn="65198" name="Air Supply Pressure" type="J1939"><signal spn="46" name="Pneumatic Supply Pressure" log="0" start="0" len="8" end="0"
fac="0" offs="0" min="-800000" max="800000" type="" unit="" ddi="" /><signal spn="1086" name="Parking and/or Trailer Air Pressure" log="0" start="8" len="8" end="0" fac="0" offs="0" min="-800000" max="800000" type=""
unit="" ddi="" /> </message></device>`


`<evaluation/>`
This part is a manually maintained part and will be copyied for every signal to log to the canlogger configfile. For more inforamtion see the amcanlogger manual.


## Run

The module can be executed with the following line:

`$ ./isoident`

There are command line options to specify different paths to the configfile, the datasets and the path to the amcanloggers configfile to enable logging. The options can be used like the following:

`$ ./isoident -f /home/isoident.xml -g /home/amcanlogger/canlogger.xml -d /home/datasets/`

* `-a VALUE`- Interval for sending address claims. 1 = once on startup, 0 = disabled (no sending via CAN interface).
* `-c TYPE` - Sets the CAN interface to listen to. Can be i.e. can0, can1, vcan0.
* `-d DIR`  - Custom path to the datasets used to identify the participants, messages and signals.
* `-f FILE` - Custom configfile. If not given, it takes the isoident.xml from the running dir.
* `-g FILE` - If this option is given, logging via the amcanlogger is enabled. The path to the amcanlogger configfile needs to be provided.
* `-h` - Shows the help prompt.

If used with other software using the CAN devices, the loopback option has to be enabled resend the messages to the buffer after receiving them. On Linux systems, this can be done with:

`$ ip link set down [CAN-DEVICE]`

`$ ip link set [CAN-DEVICE] type [CAN-TYPE] loopback on`

`$ ip link set up [CAN-DEVICE]`

Errorlogging can be done by redirecting stderr to a file.

`$ ./isoident 2>isoident_err.log`

Included is the "S54isoident" startscript, that can be copied to  /etc/init.d/. It sets the default locations to the  /media/disk/isoident/ folder.
