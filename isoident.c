

/** /file isoident.c
*	/brief main class
*	
*	
*
*	
*	Compile: $ gcc -D _BSD_SOURCE -o isoident isoident.c utils_general.c utils_parse.c utils_xml.c -lmxml -pthread -lm -std=c99
*			 $ gcc -D _BSD_SOURCE -o isoident isoident.c utils_general.c utils_parse.c utils_xml.c -lmxml -pthread -lm -std=c99 -fplan9-extensions -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable
*
*	Crosscompile: 	§ arm-linux-gnueabi-gcc -march=armv7 -D _BSD_SOURCE -o isoident-armv7 isoident.c utils_general.c utils_parse.c utils_xml.c mxml-2.10/mxml-attr.c mxml-2.10/mxml-entity.c mxml-2.10/mxml-file.c mxml-2.10/mxml-get.c mxml-2.10/mxml-index.c mxml-2.10/mxml-node.c mxml-2.10/mxml-search.c mxml-2.10/mxml-private.c mxml-2.10/mxml-set.c mxml-2.10/mxml-string.c -pthread -lm -std=c99 -fplan9-extensions -Wall -Wextra -Wno-unused-parameter -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable -mthumb -static
*
*	
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mxml-2.10/mxml.h"
#include <stdbool.h>
#include <time.h>

#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <inttypes.h>
#include <net/if.h>
#include <signal.h>

#include <linux/can.h>
#include <linux/can/raw.h>

#include <fcntl.h>
#include <unistd.h>

#include "utils_general.h"
#include "utils_xml.h"
#include "utils_parse.h"



int address_claim_cycle;
const char *canlogger_configfile_path;
const char *isoident_logfile_path;
const char *can_interface_name;

mxml_node_t* configfile_xml;
mxml_node_t* config_isoident_xml;
mxml_node_t* config_signallib_xml;
mxml_node_t* config_messagelib_xml;
mxml_node_t* config_devicelib_xml;
mxml_node_t* config_eval_xml;


bool canlogger_configfile_path_flag = 0;
mxml_node_t* canlogger_configfile_xml;
mxml_node_t* canlogger_agromicosbox_xml;
mxml_node_t* canlogger_signallib_xml;
mxml_node_t* canlogger_logfile_xml;
mxml_node_t* canlogger_eval_xml;


int can_socket;

dynArray known_devices;
dynArray known_messages;

typedef struct {
	u_int64_t data_LE;
	u_int64_t data_BE;
	u_int32_t id;
	u_int32_t sa; //source address
	u_int32_t pgn;
	short	  type; // 1 = normal message, 2 = answer to name claim, 3 = error message 
} can_message_t;

can_message_t last_message;

u_int64_t active_devices[255] = {0};
u_int64_t old_active_devices[255] = {0};

//No idea why i have to include this. Otherwise the compiler gives me a implicit declaration warning.
int getopt(int argc, char *const argv[], const char *optstring);
int kill(pid_t pid, int sig);


void show_help(void) {
   fprintf(stdout,"\n\trun: ./isoident\n\n"\
          "The isoident analyzes the ISOBUS traffic"\
          "and identifies devices as well as ISOBUS messages by their PGNs."\
          "The detected entities are saved within the isoident.xml."\
          "The software can adjust the amcanloggers config file (canlogger.xml) to"\
          "log ISOBUS signals. The signals to log can be manually defined in the isoident.xml for each signal.\n\n"\
          "The following options are accepted:\n\n"\

          "\t-a  OPTIONAL Set address claim cycle in [s]. If not provided, an address claim is send once on startup. Disable with '-c 0'\n"\
          "\t-c  OPTIONAL Set CAN interface. i.e. can0, can1, vcan0\n"\
          "\t-d  OPTIONAL Path to datasets from ISO11783. i.e. datasets/\n"\
          "\t-f  OPTIONAL Path to configfile. i.e.: /home/usr/isoident.xml\n"\
          "\t-g  OPTIONAL Path to canlogger configfile i.e.: /home/usr/canlogger.xml, enables logging via amcanlogger\n"\
          "\t-h  OPTIONAL This argument shows this message.\n\n");}


int handle_command_line_arguments(int argc, char *argv[]) {

	extern char *optarg;
	extern int optind, opterr, optopt;
	bool configfile_path_flag = 0;
	bool datasets_path_flag = 0;
	bool can_interface_name_flag = 0;
	bool address_claim_cycle_flag =0;
		
	int s;

	while ((s = getopt(argc, argv, ":a:c:f:g:d:h")) != -1) {
	    switch (s) {

	    case 'a':
	        /* handle -a, set address claim cycle period*/
	        address_claim_cycle = str_to_int(optarg);
	        address_claim_cycle_flag = 1 ;
	        break;

	    case 'c':
	        /* handle -c, set can interface*/
	        can_interface_name = optarg;
	        can_interface_name_flag = 1 ;
	        break;

	    case 'd':
	        /* handle -f, set path for logfile */
	        datasets_path = optarg;
	        datasets_path_flag = 1 ;
	        break;


	    case 'f':
	        /* handle -f, set path for custom configfile */
	        isoident_logfile_path = optarg;
	        configfile_path_flag = 1 ;
	        break;

	    case 'g':
	        /* handle -g, set path to canlogger configfile */
	        canlogger_configfile_path = optarg;
	        canlogger_configfile_path_flag = 1;
	        break;

	    case 'h':
	        /* handle -h, show help. */
	        show_help();
	        exit(EXIT_SUCCESS);
	        break;

	    case ':':
	        /* missing option argument */
	        fprintf(stderr, "ERR: %s: option '-%c' requires an argument\n",
	                argv[0], optopt);
	        show_help();
	        return EXIT_FAILURE;

	    case '?':
	    default:
	        /* invalid option */
	        fprintf(stderr, "ERR: %s: option '-%c' is invalid\n",
	                argv[0], optopt);
	        show_help();
	        return EXIT_FAILURE;
	    }
	}
	
	if (configfile_path_flag == 0) {
		fprintf(stdout, "No path to configfile given. Using default path.\n");
		isoident_logfile_path = "isoident.xml";
	}

	if (datasets_path_flag == 0) {
		fprintf(stdout, "No path to datasets given. Using default path.\n");
		datasets_path = "datasets/";
	}

	if (can_interface_name_flag == 0) {
		fprintf(stdout, "No can interface given. Trying vcan0.\n");
		can_interface_name = "vcan0";
	}

	if (address_claim_cycle_flag == 0) {
		fprintf(stdout, "No address claim cycle given. Setting it to default: 1 - sending claim once on startup.\n");
		address_claim_cycle = 1;
	}

	return EXIT_SUCCESS;}


int load_configfile() {

	FILE* configfile;

	if ((configfile = load_file(isoident_logfile_path)) != NULL) {

		configfile_xml = mxmlLoadFile(NULL,configfile,MXML_TEXT_CALLBACK);

		if (((config_isoident_xml = mxmlFindElement(configfile_xml,configfile_xml,"isoident",NULL,NULL,MXML_DESCEND))) == NULL) {
		
			fprintf(stderr,"Error parsing the xml structure in the configfile or configfile empty.\n");

			return EXIT_FAILURE;
		}


		/* Loading registered devices from configfile */
		if ((config_devicelib_xml = mxmlFindElement(config_isoident_xml,configfile_xml,"devicelib",NULL,NULL,MXML_DESCEND)) != NULL) {

			int device_count = 0;
			
			int message_count = 0; 

			mxml_node_t *device;

			for (device = mxmlFindElement(config_devicelib_xml,config_devicelib_xml,"device",NULL,NULL,MXML_DESCEND); device != NULL; device = mxmlGetNextSibling(device)) {
				if (mxmlElementGetAttr(device,"UUID") != NULL) {

					++device_count;
					insertArray(&known_devices, str_to_int((char*)mxmlElementGetAttr(device,"UUID")));
					mxmlElementSetAttr(device,"status","offline");
					int i;
					mxml_node_t *message;
					bool match;

					/* Iterate through messages and add to known_messages*/
					for (message = mxmlFindElement(device,device,"message",NULL,NULL,MXML_DESCEND); message != NULL; message = mxmlGetNextSibling(message)) {
						match = false;
						int temp_pgn = str_to_int((char*)mxmlElementGetAttr(message,"pgn"));

						if (known_messages.used == 0) {
							//fprintf(stdout,"insert message pgn: %d to array.\n",temp_pgn);
							insertArray(&known_messages,temp_pgn);
							message_count++;
							continue;
						}

						for (i=0; i < (known_messages.used); i++) {
							

							if (temp_pgn == known_messages.array[i]) {
								fprintf(stdout,"Message PGN: %d already in list\n", temp_pgn);
								match = true;
							}

						}

						if (match == false) {
							//fprintf(stdout,"insert message pgn: %d to array.\n", temp_pgn);
							insertArray(&known_messages,temp_pgn);
							message_count++;
						}
					}
				}
			}
			
			fprintf(stdout,"%d devices and %d messages found in configfile\n",device_count,message_count);
		}

		else {
			fprintf(stderr,"Error parsing the devicelib.\n");
			return EXIT_FAILURE;
		}

		if ((config_signallib_xml = mxmlFindElement(config_isoident_xml,configfile_xml,"signallib",NULL,NULL,MXML_DESCEND)) == NULL) {
			fprintf(stderr,"Error parsing the signallib.\n");
			return EXIT_FAILURE;
		}

		if ((config_messagelib_xml = mxmlFindElement(config_isoident_xml,configfile_xml,"messagelib",NULL,NULL,MXML_DESCEND)) == NULL) {
			fprintf(stderr,"Error parsing the messagelib.\n");
			return EXIT_FAILURE;
		}

		if ((config_eval_xml = mxmlFindElement(config_isoident_xml,configfile_xml,"evaluation",NULL,NULL,MXML_DESCEND)) == NULL) {
			fprintf(stderr,"Error parsing the evaluation section.\n");
			return EXIT_FAILURE;
		}

		fprintf(stdout, "Config file loaded successfully.\n");		
		
		fclose(configfile);

		return EXIT_SUCCESS;
	}

	else {
		
		fprintf(stdout, "Try to create new config file.\n");
		config_devicelib_xml = mxmlLoadString(NULL,"<devicelib></devicelib>",MXML_NO_CALLBACK);
		config_signallib_xml = mxmlLoadString(NULL,"<signallib></signallib>",MXML_NO_CALLBACK);
		config_messagelib_xml = mxmlLoadString(NULL,"<messagelib></messagelib>",MXML_NO_CALLBACK);
		config_eval_xml = mxmlLoadString(NULL,"<evaluation><level1 /><level2 /><logic /></evaluation>",MXML_NO_CALLBACK);

		free(configfile);

		if (xml_write_file(isoident_logfile_path,config_isoident_xml,config_signallib_xml,config_messagelib_xml,config_devicelib_xml,config_eval_xml) != EXIT_FAILURE) {
			fprintf(stdout,"Successfully created new configfile.\n");
			chmod(isoident_logfile_path, 0666);
			load_configfile();
			return EXIT_SUCCESS;
		}

		else{
			fprintf(stdout,"Failed creating new configfile.");
			return EXIT_FAILURE;
		}
	}

}


int load_canlogger_configfile() {
	FILE* canloggerfile;

	fprintf(stdout,"PAth to canlogger config file: %s\n",canlogger_configfile_path );

	if ((canloggerfile = load_file(canlogger_configfile_path)) != NULL) {

		canlogger_configfile_xml = mxmlLoadFile(NULL,canloggerfile,MXML_TEXT_CALLBACK);
		canlogger_agromicosbox_xml = mxmlFindElement(canlogger_configfile_xml,canlogger_configfile_xml,"agromicosbox",NULL,NULL,MXML_DESCEND);

		if ((canlogger_signallib_xml = mxmlFindElement(canlogger_agromicosbox_xml,canlogger_configfile_xml,"signallib",NULL,NULL,MXML_DESCEND)) == NULL) {
			fprintf(stderr, "Error loading canlogger configfile - signallib\n");
			return EXIT_FAILURE;
		}

		if ((canlogger_logfile_xml = mxmlFindElement(canlogger_agromicosbox_xml,canlogger_configfile_xml,"logfile",NULL,NULL,MXML_DESCEND)) == NULL) {
			fprintf(stderr, "Error loading canlogger configfile - logfile\n");
			return EXIT_FAILURE;
		}

		if ((canlogger_eval_xml = mxmlFindElement(canlogger_agromicosbox_xml,canlogger_configfile_xml,"evaluation",NULL,NULL,MXML_DESCEND)) == NULL) {
			fprintf(stderr, "Error loading canlogger configfile - evaluation\n");
			return EXIT_FAILURE;
		}

		return EXIT_SUCCESS;
		free(canloggerfile);

	}

	fprintf(stderr,"Error loading canlogger configfile.\n");
	return EXIT_FAILURE;

}

int can_init_socket() {
	struct ifreq ifr;
	struct sockaddr_can addr;

	can_socket = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (can_socket < 0) {// Error open socket
		fprintf(stderr, "could not open socket, Interface: %s\n",can_interface_name);
		return EXIT_FAILURE;
	}

	addr.can_family = AF_CAN;

	memset(&ifr.ifr_name, 0, sizeof(ifr.ifr_name));
	strncpy(ifr.ifr_name, can_interface_name, strlen(can_interface_name));

	if (ioctl(can_socket, SIOCGIFINDEX, &ifr) < 0)
	{
		fprintf(stderr, "ioctl failed for socket, Interface: %s\n",can_interface_name);
		return EXIT_FAILURE;
	}
	addr.can_ifindex = ifr.ifr_ifindex;

	if (bind(can_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		fprintf(stderr, "bind socket failed, Interface: %s\n",can_interface_name);
		return EXIT_FAILURE;
	}
	fprintf(stdout,"CAN Socket Nr.: %d bound to process! Interface: %s\n",can_socket, can_interface_name);
	
	//Set CAN Filter to only receive messages according to J1939/ISO11783/NMEA2000 by excluding messages with EDP = 1 and DP =1

	struct can_filter rfilter;

	rfilter.can_id = 0x23000000; // DP Bit = 1, EDP Bit = 1 and CAN_INV_FILTER enabled
	rfilter.can_mask = 0x03000000;

	setsockopt(can_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));

	return EXIT_SUCCESS;}


void can_close_socket() {

	if( close(can_socket) < 0 ) {
    	fprintf(stderr,"Could not close CAN socket: %d \n", can_socket);
    }
    else {
    	fprintf(stdout,"CAN socket shut down.\n");
	}}


int can_send(u_int32_t can_id, u_int64_t can_data, u_short can_dlc) {
	struct can_frame frame;
	int bytes_sent;
	int i;
	frame.can_id = can_id;

	//write can_data to frame bytes
	for (i=0 ; i<8 ; i++) {
		frame.data[i] = (can_data >>(i*8)) & 0xff;
	}

	frame.can_dlc = can_dlc;
	
	fprintf(stdout,"------\nSend CAN message:\nSocket: %d | ID: 0x%08x | Message: 0x",can_socket,frame.can_id);
	for (i=0 ; i<8 ; i++) {
		fprintf(stdout,"%02x",frame.data[i]);
	}

	bytes_sent = write(can_socket, &frame, sizeof(frame));
	
	fprintf(stdout,"\nBytes sent: %d\n",bytes_sent);
	
	if (bytes_sent > 0) {
		fprintf(stdout, "CAN message sent successfully.\n------\n");
		return EXIT_SUCCESS;
	}

	else {
		fprintf(stdout, "Error sending message to CAN Bus.\n");
		return EXIT_FAILURE;
	}
}


int can_read() {

	struct can_frame frame_read;
	int recvbytes = 0;
	int read_can_port = 1;
	int address;
	int can_read_timeout = 1;
	int64_t name=0;
	time_t timeStart = time(NULL);

	while (read_can_port && (time(NULL) < (timeStart+can_read_timeout))) {
 		
 		last_message.type = 0 ; //Reset old message
 		last_message.data_LE = 0;

 		struct timeval timeout = {1,0};
 		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(can_socket,&readSet);
		int i;

		if (select((can_socket + 1),&readSet,NULL,NULL,&timeout) >= 0) {
   			if (!read_can_port) {
    				break;
   			}
			if (FD_ISSET(can_socket, &readSet)) {
	    			recvbytes = read(can_socket,&frame_read,sizeof(struct can_frame));

				if (recvbytes) {
		     		last_message.id   = (frame_read.can_id & 0x1FFFFFFF); //clear lower 3 bits from canID
					last_message.sa   = (frame_read.can_id & 0x000000FF);
					last_message.pgn  = parse_get_pgn(frame_read.can_id);
					
					//Check if message is adress-claim-answer
					
					if ((last_message.id & 0xFFFFFF00) == 0x18EEFF00) {
						
						fprintf(stdout,"Address claim detected.\n");

						last_message.type = 2; //Message is a adress claim
						
						last_message.sa = frame_read.can_id & 0x000000FF;
					}

					else {
						last_message.type = 1; //Message is NO adress claim
					}
					

					//Merge data bytes to one data variable
					for (i=7; i>=0; i--) {
							
						last_message.data_LE = ((last_message.data_LE << 8) | (frame_read.data[i]));
					
					}

					read_can_port=0;
					
					fprintf(stdout,"Received new message. Type %d- PGN: %d SA: %d ID: 0x%x, DATA: 0x%" PRIx64 "\n" ,last_message.type,last_message.pgn,last_message.sa, last_message.id, last_message.data_LE);
					
					return last_message.type;
				}
			}
		}
	}
	fprintf(stdout,"No message received within timeout time ( t= %ds ).\n",can_read_timeout);
	return 0;
}

int read_pid (char *pidfile) {
  FILE *f;
  int pid;

  if (!(f=fopen(pidfile,"r")))
    return 0;
  fscanf(f,"%d", &pid);
  fclose(f);
  return pid;
}

int check_pid (int pid) {
  /*
   * The 'standard' method of doing this is to try and do a 'fake' kill
   * of the process.  If an ESRCH error is returned the process cannot
   * be found -- GW
   */
  /* But... errno is usually changed only on error.. */
  if (kill(pid, 0) && errno == ESRCH)
	  return(0);

  return pid;
}

int canlogger(int command) {
	switch (command) {
		case 0: { //status
			//get pid, the pid in the file is not always correct! Usually it is pid+2
 			int pid = (read_pid("/var/run/canlogger.pid"));
			pid= pid+2;

			if (check_pid(pid) == 0) {
				fprintf(stdout, "Process with PID %d not running.\n",pid);
				return 0;
			}

			get_process_name_by_pid(pid);

			fprintf(stdout, "Process with PID %d running.\n",pid);
			

			return pid;
			
			break;

		}

		case -1: //end
			if (kill(canlogger(0),SIGTERM)==0) {

				system("/etc/init.d/S53amcanlogger stop");
			}
			
			break;

		case 1: //start
			
			system("/etc/init.d/S53amcanlogger restart");
			
			break;
		
		default:
			fprintf(stderr, "Wrong input value.\n");
			return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}

int update_canlogger_configfile() {
	int i;

	//load_configfile();

	mxmlDelete(canlogger_logfile_xml);
	
	canlogger_logfile_xml = mxmlNewElement(canlogger_agromicosbox_xml,"logfile");

	mxml_node_t* temp_message;
	
	mxml_node_t* temp_signal;
	mxml_node_t* temp_signal_parent;
	mxml_node_t* temp_signal_parent_parent;
	
	mxml_node_t* new_log_signal;

	mxml_node_t* new_log_signal_lib;

	fprintf(stdout, "Look for signals to log.\n");

	//Look for active devices and look up which messages have to be logged.
	for (temp_signal = mxmlFindElement(config_isoident_xml,config_isoident_xml,"signal","log","1",MXML_DESCEND); temp_signal != NULL; temp_signal = mxmlFindElement(temp_signal,config_isoident_xml,"signal","log","1",MXML_DESCEND)) {
		
		fprintf(stdout,"Found signal to log - SPN: %s\n", mxmlElementGetAttr(temp_signal,"spn"));
		
		int temp_sa = -1;
		int temp_uuid = 0;
		int temp_pgn= -1;

		char* temp_pgn_str;
		char* temp_sa_str;

		int log=0;

		temp_signal_parent = mxmlGetParent(temp_signal);

		// Signal 
		if ((mxmlElementGetAttr(temp_signal_parent,"pgn")) != NULL) {

			temp_pgn_str = strdup(mxmlElementGetAttr(temp_signal_parent,"pgn"));

			fprintf(stdout," -> Signal belongs to message - PGN: %s\n", temp_pgn_str);

			temp_signal_parent_parent = mxmlGetParent(temp_signal_parent);

			
			if ((mxmlElementGetAttr(temp_signal_parent_parent,"UUID")) != NULL) {

				temp_uuid = str_to_int(strdup(mxmlElementGetAttr(temp_signal_parent_parent,"UUID")));

				fprintf(stdout," --> Message belongs to device - UUID: %d\n", temp_uuid);

				for (i=0; i < (sizeof(active_devices)/sizeof(active_devices[0])); i++) {

					if (active_devices[i] == temp_uuid) {
						temp_sa = i;
						log=1;
						break;
					}

				}

				if (log == 0) {
					fprintf(stdout, " ---> Device is not online - signal will not b logged.\n" );
					continue; //Continue if device is not online, and signal will not be logged.
				}
			}

		}

		else {
			temp_pgn_str = int_to_string(-1);
		}
		
		temp_sa_str = int_to_string(temp_sa);

		//Add signal to log

		new_log_signal = mxmlNewElement(canlogger_logfile_xml,"log");

		mxmlElementSetAttr(new_log_signal,"name",mxmlElementGetAttr(temp_signal,"name"));
		mxmlElementSetAttr(new_log_signal,"unit","-");
		mxmlElementSetAttr(new_log_signal,"des","-");

		//Check if signal is in lib, add new or edit the old.

		if ((new_log_signal_lib = mxmlFindElement(canlogger_signallib_xml,canlogger_configfile_xml,"iso","name",mxmlElementGetAttr(temp_signal,"name"),MXML_DESCEND)) == NULL) {
			
			//Not yet in the signallib, add signal to lib

			fprintf(stdout,"Add %s to sigallib\n", mxmlElementGetAttr(temp_signal,"name"));

			new_log_signal_lib = mxmlNewElement(canlogger_signallib_xml,"iso");
		}

		mxmlElementSetAttr(new_log_signal_lib,"name",mxmlElementGetAttr(temp_signal,"name"));
		mxmlElementSetAttr(new_log_signal_lib,"pgn",temp_pgn_str);
		mxmlElementSetAttr(new_log_signal_lib,"sa",temp_sa_str);
		mxmlElementSetAttr(new_log_signal_lib,"sbit",mxmlElementGetAttr(temp_signal,"start"));
		mxmlElementSetAttr(new_log_signal_lib,"len",mxmlElementGetAttr(temp_signal,"spn"));
		mxmlElementSetAttr(new_log_signal_lib,"spn",mxmlElementGetAttr(temp_signal,"len"));
		mxmlElementSetAttr(new_log_signal_lib,"end",mxmlElementGetAttr(temp_signal,"end"));
		mxmlElementSetAttr(new_log_signal_lib,"fac",mxmlElementGetAttr(temp_signal,"fac"));
		mxmlElementSetAttr(new_log_signal_lib,"offs",mxmlElementGetAttr(temp_signal,"offs"));
		mxmlElementSetAttr(new_log_signal_lib,"min",mxmlElementGetAttr(temp_signal,"min"));
		mxmlElementSetAttr(new_log_signal_lib,"max",mxmlElementGetAttr(temp_signal,"max"));
		mxmlElementSetAttr(new_log_signal_lib,"type",mxmlElementGetAttr(temp_signal,"type"));
		mxmlElementSetAttr(new_log_signal_lib,"unit",mxmlElementGetAttr(temp_signal,"unit"));
		mxmlElementSetAttr(new_log_signal_lib,"ddi",mxmlElementGetAttr(temp_signal,"ddi"));

		free(temp_pgn_str);
		free(temp_sa_str);

	}

	//write canlogger configfile


	// Copy eval part to canlogger.xml

	mxml_node_t* canlogger_eval_element_xml;
	mxml_node_t* canlogger_eval_next_element_xml;

	mxmlDelete(canlogger_eval_xml);
	
	mxmlAdd(canlogger_agromicosbox_xml,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,config_eval_xml);

	canlogger_eval_xml = config_eval_xml;
	fprintf(stdout,"Check for useless elements in eval part.\n");

	canlogger_eval_next_element_xml = mxmlFindElement(canlogger_eval_xml,canlogger_eval_xml,NULL,"name",NULL,MXML_DESCEND);
	
	for ((canlogger_eval_element_xml = mxmlFindElement(canlogger_eval_xml,canlogger_eval_xml,NULL,"name",NULL,MXML_DESCEND)); canlogger_eval_next_element_xml != NULL; canlogger_eval_element_xml = canlogger_eval_next_element_xml) {
		
		fprintf(stdout," -> Check for element with name: %s\n",mxmlElementGetAttr(canlogger_eval_element_xml,"name"));
		
		sleep(1);
		
		canlogger_eval_next_element_xml = mxmlFindElement(canlogger_eval_element_xml,canlogger_eval_xml,NULL,"name",NULL,MXML_DESCEND);

		fprintf(stdout," -> next element will be: %s\n",mxmlElementGetAttr(canlogger_eval_next_element_xml,"name"));

		if ( mxmlFindElement(canlogger_logfile_xml,canlogger_logfile_xml,"log","name",mxmlElementGetAttr(canlogger_eval_element_xml,"name"),MXML_DESCEND) == NULL) {
			fprintf(stdout," --> Logging of required signal is not enabled - removing from eval\n");
			mxmlDelete(canlogger_eval_element_xml);
		}
	}

	fprintf(stdout,"Write canlogger configfile.\n");

	FILE *canloggerfile = fopen(canlogger_configfile_path,"w");

	mxmlSaveFile(canlogger_configfile_xml,canloggerfile,MXML_NO_CALLBACK);

	fclose(canloggerfile);

	load_configfile();

	return EXIT_SUCCESS;
}

int handle_address_claim_message() {
	int device_id = parse_get_device_id(last_message.data_LE);
	int i; 
	
	active_devices[last_message.sa] = device_id;

	
	bool match = false;

	if (known_devices.used == 0) {
		fprintf(stdout,"Add device because there is no device known yet.\n");
			//Add new device to device log
			if (xml_add_device(config_devicelib_xml,device_id,last_message.data_LE,last_message.sa) == EXIT_FAILURE) {
				return EXIT_FAILURE;
			}
			insertArray(&known_devices,device_id);
			return 1;
	}

	for (i=0; i < known_devices.used; i++) {
		
		fprintf(stdout,"Check if ID:%d - is matching DB entry: %d\n",device_id, known_devices.array[i]);
		
		if (device_id == known_devices.array[i]) {
			fprintf(stdout,"Device is known from isoident.xml\n");
			
			char date_buff[70];
			time_t now = time(NULL);
 			struct tm *now_local = localtime(&now);
 
    		if (strftime(date_buff, sizeof date_buff, "%Y-%m-%d-%H:%M", now_local)) {
        		fprintf(stdout,"Device seen on %s\n",date_buff);
    		} else {
        		fprintf(stderr,"strftime failed");
    		}
    		char* active_device_sa = int_to_string(last_message.sa);
    		char* active_device_id = int_to_string(device_id);

    		mxml_node_t* device = mxmlFindElement(config_devicelib_xml,config_devicelib_xml,"device","UUID",active_device_id,MXML_DESCEND);
    		
    		mxmlElementSetAttr(device,"lastClaim",date_buff);
    		mxmlElementSetAttr(device,"lastSA",active_device_sa);
    		mxmlElementSetAttr(device,"status","online");

    		free(active_device_id);
    		free(active_device_sa);
			match = true;
			break;
		}

		else {
			fprintf(stdout,"No match found\n");
		}
	}

	if (match == false) {
		fprintf(stdout,"Add device because is not yet in the db.\n");
		if (xml_add_device(config_devicelib_xml,device_id,last_message.data_LE,last_message.sa) == EXIT_FAILURE) {
			return EXIT_FAILURE;
		}; //Add new device to device log
		
		insertArray(&known_devices,device_id);
	}
	return EXIT_SUCCESS;
}

int quit_isoident() {
	can_close_socket(socket);
	exit(EXIT_SUCCESS);
}

void SIGTERMHandler(int nr) {

  	fprintf(stdout, "SIGTERM received: %d\n", nr);

  	fflush(stdout);
  	fflush(stderr);
	//if (pthread_join (p_sync, NULL))
	quit_isoident();

	// Should never be reached
  	exit(EXIT_SUCCESS);
}


int init(int argc, char *argv[]) {
	/* Handle the command line arguments */
	if (handle_command_line_arguments(argc, argv) == EXIT_FAILURE) {
		fprintf(stderr,"Error handling command line arguments.\n");
		return EXIT_FAILURE;}

	/* Initialize dynamic arrays */
	initArray(&known_devices,1);
	initArray(&known_messages,1);

	/* Load config files */
	if (load_configfile() == EXIT_FAILURE) {
		fprintf(stderr,"Error loading data from config file.\n");
		return EXIT_FAILURE;
	}

	/*Load canlogger.xml if requested. */
	if ((canlogger_configfile_path_flag == 1) && (load_canlogger_configfile() != EXIT_SUCCESS)) {
		fprintf(stderr, "Error loading canlogger.xml\n");
		return EXIT_FAILURE;
	}

	/* Initialize CAN Socket*/
	if (can_init_socket() == EXIT_FAILURE) {
		fprintf(stderr,"Error initialising CAN socket.\n");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}

int main(int argc, char *argv[]) {

	signal(SIGTERM, SIGTERMHandler);
	
	int address_claim_timeout = 3; //How long to wait for address-claimed messages before updating device list in [s]
	time_t time_last_claim;

	if (init(argc, argv) == EXIT_FAILURE) {
		fprintf(stderr,"Error in init routine.\n");
		return EXIT_FAILURE;
	}

	time_last_claim = 0;
	
	while (1) {
		if (address_claim_cycle != 0 && (time(NULL) > (time_last_claim+address_claim_cycle))) {
			
			if (address_claim_cycle == 1) {
				fprintf(stdout,"Send address claim once on startup.\n");
				address_claim_cycle = 0 ;
			}

			can_send(0x18EAFFFE,0x00EE00,3);
			time_last_claim = time(NULL);
		}
		
		switch(can_read()) {
			case 0: //no message
				break;

			case 1: { //normal message
			
				bool match = false;
				//Check if the sender is registered
				fprintf(stdout,"Sender SA: %d, active device list: %" PRIu64 "\n", last_message.sa,active_devices[last_message.sa]);
				if (active_devices[last_message.sa] != 0) {
					char* active_devices_uuid = int_to_string(active_devices[last_message.sa]);
					mxml_node_t* sender = mxmlFindElement(config_devicelib_xml,config_devicelib_xml,"device","UUID",active_devices_uuid,MXML_DESCEND);
					free(active_devices_uuid);
					mxml_node_t* message;

					for (message = mxmlFindElement(sender,sender,"message",NULL,NULL,MXML_DESCEND); message != NULL; message = mxmlGetNextSibling(message)) {
						if (last_message.pgn == str_to_int((char*)mxmlElementGetAttr(message,"pgn"))) {
							fprintf(stdout, "Message with PGN: %d is already in the config file.\n", last_message.pgn);
							match = true;
							break;
						}
					}

					if (match == false) {
						//Add message to isoident.xml
						fprintf(stdout,"New message detected! PGN: %d\n",last_message.pgn);
						xml_add_message(sender,last_message.pgn);
						xml_write_file(isoident_logfile_path,config_isoident_xml,config_signallib_xml,config_messagelib_xml,config_devicelib_xml,config_eval_xml);
					}
				}

				else {

					fprintf(stderr, "Message from unknown sender (SA: %d). Message will be added to messagelib.\n",last_message.sa);
					char* last_message_pgn = int_to_string(last_message.pgn);
					if (mxmlFindElement(config_messagelib_xml,config_messagelib_xml,"message","pgn",last_message_pgn,MXML_DESCEND) != NULL){
						fprintf(stdout, "Message is already in messagelib\n");
					} else {
						xml_add_message(config_messagelib_xml,last_message.pgn);
						fprintf(stdout, "Message added to messagelib\n");
						xml_write_file(isoident_logfile_path,config_isoident_xml,config_signallib_xml,config_messagelib_xml,config_devicelib_xml,config_eval_xml);
					}
					free (last_message_pgn);
				}

				break;
			}
			case 2: //Address claim
				//address claim
				{
				memset(active_devices, 0, sizeof(active_devices)); //Clear device list.

				handle_address_claim_message();

				time_t time_first_claim_answer = time (NULL);

				/*This loop listens for additional address claims from the ISOBUS devices for a certain time.
				This period is defined through the adress_claim_timeout var in seconds*/
				while (time(NULL) < (time_first_claim_answer + address_claim_timeout)) {
					fprintf(stdout, "Waiting for other adress claims.\n");
					if (can_read() == 2) {
						if (handle_address_claim_message() == EXIT_FAILURE) {
							return EXIT_FAILURE;
						}
					}
					else {
						fprintf(stdout,"No address claim detected.\n");
					}
					
				}		
				
				if (memcmp(active_devices,old_active_devices,sizeof(active_devices)) != 0) {
					
					fprintf(stdout, "There are new devices active. The isoident configfile will be updated.\n");
					
					xml_write_file(isoident_logfile_path,config_isoident_xml,config_signallib_xml,config_messagelib_xml,config_devicelib_xml,config_eval_xml);

					if (canlogger_configfile_path_flag == 1) {

						fprintf(stdout, "The canlogger configfile will be updated.\n");

						//Lookup messages to be logged.

						update_canlogger_configfile();

						canlogger(1); //Restart canlogger.
					}
				}

				memcpy(old_active_devices,active_devices,sizeof(active_devices));


				break;}

			default:
				fprintf(stdout,"Message could not be classified.\n");
		}

	}


	fprintf(stdout,"\n---------------End----------------\n");
	fprintf(stdout,"---This should never be reached---");
	return EXIT_FAILURE;

}