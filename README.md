# isoident
This module identifies participants and message on the ISOBUS network utilizing the address-claim procedure defined in ISO 11783-5. The detected participants and their messages will be saved in the configfile. Furthermore it can be an addition to the existing amcanlogger to adjust the CAN logging profile dynamically to the active devices.

TODO:
* ✓ Clean up (~~Memory Leaks~~, ~~CAN Filter~~, ~~Exceptions in datasets structure - utils_parse, line 408~~)
* ✓ J1939 Integration (~~PGN~~, ~~SPN~~, ~~Manufacturer~~)
* ✓ Add log file/ debug option
* ✓ Additional Data in isoident.xml (~~lastSA~~, ~~status~~, ~~lastSeen~~, ~~class and industry in isoident.xml~~)
* ✓ Identify messages from unknown sender (~~Restructure isoident.xml~~, ~~add method to handle unknown messages~~,~~isoident.xml generator~~)
* ✓ Optimize Canlogger.xml-handling (~~No overwriting, ...~~)
* Edit xml via web (~~Test nginx~~, ~~test Xonomy~~, ~~create basic docSpec~~ , ~~create web frame~~, test on box) 
* Update config section
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

The software loads depends on a configuration file that contains information about the identified devices, messages and signals.

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
