/*This module simulates a CAN participant that answers to a adress claim with a valid answer phrase*/


#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <string.h>

#ifndef PF_CAN
#define PF_CAN 29
#endif

#ifndef AF_CAN
#define AF_CAN PF_CAN
#endif

const char* interface =  "vcan0"; //Set CAN interface

struct participant {
	char *name;
	char *industry;
	char *function;
	int messagesToLog[20];   //How many messages per device to be logged
};


int init(void) {
	printf("Intitialize virtual CAN interface\n");
    system ("modprobe vcan");
	sleep(1);
	system ("ip link add dev vcan0 type vcan");
	sleep(1);
	system ("ip link set up vcan0");
	printf("Virtual CAN up and running\n");


	int skt = socket (PF_CAN, SOCK_RAW, CAN_RAW);
	struct ifreq ifr;
	strcpy(ifr.ifr_name, interface);
	ioctl(skt, SIOCGIFINDEX, &ifr);

	struct sockaddr_can addr;
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	bind (skt, (struct sockaddr*)&addr, sizeof(addr));
	return skt;
	}

int canSend(int skt, int canID, char* canDATA) {
	printf("Send Message\n");
	printf("Socket: %d\n",skt);
	printf("CAN Identifier: %04x\n",canID);
	printf("Message: %s\n",canDATA);
	struct can_frame frame;
	frame.can_id = canID | CAN_EFF_FLAG; //The flag solves the problem with the 0xd8 instead of 0x18 (3 placeholder)
	//strcpy(frame.data, canDATA);
	long canData;
	sscanf(canDATA,"%lx",&canData); //Turn hex string into hex value

	//Map the hex value to the single bytes of the CAN Message	
	int i;
	for (i=0; i<8; i++){
		frame.data[7-i] = (canData>>(i*8) & 0xff);
		printf("Byte: %d - %02x\n", (8-i), frame.data[7-i] );
		}

	frame.can_dlc = 8;
	int bytes_sent = write (skt, &frame, sizeof(frame));
	return 0;
}

int canRead(int skt) {
	struct can_frame frame;
	int recvbytes = 0;
	int read_can_port = 1;
	time_t timeStart = time(NULL);

	while (read_can_port && (time(NULL)<(timeStart + 0.5))) {
 		struct timeval timeout = {1,0};
 		fd_set readSet;
		FD_ZERO(&readSet);
		FD_SET(skt,&readSet);

		if (select((skt + 1),&readSet,NULL,NULL,&timeout) >= 0) {
   			if (!read_can_port) {
    				break;
   			}
			if (FD_ISSET(skt, &readSet)) {
	    			recvbytes = read(skt,&frame,sizeof(struct can_frame));

				if (recvbytes) {
		     		int can_id = (frame.can_id & 0x1FFFFFFF); //clear lower 3 bits from canID
					int can_address = (frame.can_id & 0x000000FF); 
					printf("skt= %d dlc = %d, id= %02x\n", skt,  frame.can_dlc,can_id);
		    		if (can_id == 0x18EAFFFE) {
						printf("Detected Adress-CLaim-Message");
						return 1;
					}
				}
			}
		}
	}
	return 0;
}


int main(void) {
	int skt = init();


	
	while (1){
		if (canRead(skt)==1) {
			canSend(skt,0x18EEFF0C,"01008020098604A1");
			//sleep(0.2);
			canSend(skt,0x18EEFF1A,"6790091f004e1aa1");
			//sleep(4);
			canSend(skt,0x18EEFF26,"f41fd412a41234a1");
			sleep(1);
			}
		canSend(skt,0x1CEC260C,"13090002FF00E600");
		canSend(skt,0x1CFE411A,"C2FFFFFF48484006");
		canSend(skt,0x18EB0C26,"023031FFFFFFFFFF");
		canSend(skt,0x1CE6D826,"C3FFFFFFFFFFFFFF");
		}
	return 0;
}
