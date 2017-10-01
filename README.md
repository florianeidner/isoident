# isoident
This module identifies participants and message on the ISOBUS network utilizing the address-claim procedure defined in ISO 11783-5. The detected participants and their messages will be saved in the configfile. Furthermore it can be an addition to the existing amcanlogger to adjust the CAN logging profile dynamically to the active devices.

## Compiling

The module uses the Mini-XML library which is included in the repo or can be found there: https://michaelrsweet.github.io/mxml/mxml.html

Compiliation can be done for the local machine, manually via:

`$ gcc -D _BSD_SOURCE -o isoident isoident.c utils_general.c utils_parse.c utils_xml.c -lmxml -pthread -lm -std=c99`

or

`$ gcc -D _BSD_SOURCE -o isoident isoident.c utils_general.c utils_parse.c utils_xml.c -lmxml -pthread -lm -std=c99 -fplan9-extensions -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable`

or via the included Makefile, executing:

`$ make`

Cross - Compiling for other Platforms, i.e. for an Cortex A7 system can be done utilizing the gcc-arm-linux-gnueabi toolchain, doing:

`$ arm-linux-gnueabi-gcc -march=armv7 -D _BSD_SOURCE -o isoident-armv7 isoident.c utils_general.c utils_parse.c utils_xml.c mxml-2.10/mxml-attr.c mxml-2.10/mxml-entity.c mxml-2.10/mxml-file.c mxml-2.10/mxml-get.c mxml-2.10/mxml-index.c mxml-2.10/mxml-node.c mxml-2.10/mxml-search.c mxml-2.10/mxml-private.c mxml-2.10/mxml-set.c mxml-2.10/mxml-string.c -pthread -lm -std=c99 -fplan9-extensions -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable` -mthumb -static

## Config

The software loads the config parameters from the given configfile (default: isoident.xml). The `<config>` section includes the following parameters:
* `can_interface` - Defines the CAN interface. (i.e. vcan0, can0, can1, ...)
* `adress_claim_cycle` - The interval in seconds between requesting the participants to claim their addresses. If this is set to 0 the module doesnt send messages via the CAN Bus and only listens to external address-claims.

Within the `<devicelog>` section in the config file, the identified devices, messages and signals will be saved. If used in combination with the amcanlogger, the "log" attribute of the "signal" nodes can be set to have the signal logged via the amcanlogger.

## Running

The module can be executed with the following line:

`$ ./isoident`

There are command line options to specify different paths to the configfile, the datasets and the path to the amcanloggers configfile to enable logging. The options can be used like the following:

`$ ./isoident -f /home/isoident.xml -g /home/amcanlogger/canlogger.xml -d /home/datasets/`

* `-d DIR` - Custom path to the datasets used to identify the participants, messages and signals.
* `-f FILE` - Custom configfile. If not given, it takes the isoident.xml from the running dir.
* `-g FILE` - If this option is given, logging via the amcanlogger is enabled. The path to the amcanlogger configfile needs to be provided.
* `-h` - Shows the help prompt.
