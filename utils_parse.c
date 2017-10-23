/** /file utils_parse.c 	
*	/brief Parsing the keys for manufacturer, pgn ,spn and function
*
*	This module takes the adress-claimed message (PGN 60928) and uses
*	the NAME from the CAN Data field to retrieve manufacturer, function
*	and class of the sender from the iso 11783 NAME list.
*
*	/author Florian Eidner
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <inttypes.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <string.h>

#include "utils_general.h"

/*
*   \brief Create bitmasks
*
*	To extract certain bits from a 64bit integer,
*	a bitmask is used. The mask is returned by this function.
*/

u_int64_t create_mask(int start_bit, int end_bit) {
	
	u_int64_t mask = 0;
	int i;
	
	if (start_bit <= end_bit) {

		for (i = start_bit; i <= end_bit; i++) {
			mask |= 1 << i;
		}
	
	}
	
	else {
		mask = 0;
	}
	
	return mask;
}


/*
*   \brief Extract Manufacturer key
*
*	Extracting the manufacturer key from the data field of a NAME claim
*/

int parse_get_manufac_id(u_int64_t can_data) {

	int manufac_id;
	u_int64_t mask = create_mask(0,10);
	manufac_id = mask & (can_data >> 21);
	printf("mask: 0x%" PRIx64 " - manufac_id: %d\n",mask, manufac_id );

	return manufac_id;
}


/*
*   \brief Create UUID
*
*	Extracting the manufacturer key and device id and use it as UUID
*	Consisting of the first 32bit of the data field.
*/
int parse_get_device_id(u_int64_t can_data) {

	int device_id;
	// u_int64_t mask = create_mask(0,20);
	// Instead of device ID, create UUID from
	// manufacturer ID and this device ID:

	u_int64_t mask = create_mask(0,31);

	device_id = can_data & mask;
	printf("mask: 0x%" PRIx64 " - device_id: %d\n",mask, device_id );

	return device_id;
}


/*
*   \brief Extract function key
*
*	Extracting the manufacturer key from the
*	data field of a NAME claim.
*/
int parse_get_function_id(u_int64_t can_data) {

	int function_id;
	u_int64_t mask = create_mask(0,7);
	function_id = mask & (can_data >> 40);
	printf("mask: 0x%" PRIx64 " - function_id: %d\n",mask, function_id );

	return function_id;
}


int parse_get_industry_id(u_int64_t can_data) {

	int industry_id;
 	u_int64_t mask = create_mask(0,2);
 	industry_id = mask & (can_data >> 60);
 	printf("mask: 0x%" PRIx64 " - industry_id: %d\n",mask, industry_id );

 	return industry_id;
}

int parse_get_class_id(u_int64_t can_data) {

 	int class_id;
 	u_int64_t mask = create_mask(0,6);
 	class_id = mask & (can_data >> 49);
 	printf("mask: 0x%" PRIx64 " - class_id: %d\n",mask, class_id );

 	return class_id;
}


/*
*   \brief Parse manufacturer name
*
*	Looking up manufacturer key in file and get the name of the manufacturer.
*/
int parse_get_manufacturer(u_int64_t can_data,char* man_name) {
	
	int man_id = parse_get_manufac_id(can_data);

	//printf("Manufacturer ID: %d\n",man_id);

	FILE *file;
	char* path = concat(datasets_path,"manufacturers.CSV");
	if ((file = load_file(path)) == NULL) {
		free(path);
		return EXIT_FAILURE;
	}
	free(path);

	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	int lineID;
	short match = 0;

	if (file == NULL)
        exit(0);

	if (file){
		while (((read =getline(&line, &len, file)) != -1)) {
		    char *line2 = line;
		    int lineID = (int)strtol((strtok(line2, ";")),((char **)NULL),10);
		    //printf("lineID: %d\n", lineID);
				if (lineID == man_id){
					char* man_name_update = strtok(NULL,"\n");
					strcpy(man_name,man_name_update); //This is important, strtok changes the pointer!
					match=1;
					break;
				}
			}
		if (match == 0){
			strcpy(man_name,"unknown");
			printf("Manufacturer not found.\n");
		}
		else {
			printf("Manufacturer: %s \n",man_name);
		}
	}

	else{
		printf("File could not be opened.");
	}
	free(line);
	fclose(file);

	return 0;


}

/*
*   \brief Parse function name
*
*	Looking up function key in file and get the name of the function.
*/

int parse_get_function(u_int64_t can_data, char* func_name) {
	int func_id = parse_get_function_id(can_data);
	if (func_id > 127) {
		char* industry_id = int_to_string(parse_get_industry_id(can_data));
		char* class_id =int_to_string(parse_get_class_id(can_data));
		char* function_id = int_to_string(func_id);
        char* concat1 = concat(industry_id,class_id);
		char* concat2 = concat(concat1,function_id);
		
		func_id = str_to_int(concat2);

		free (industry_id);
		free (class_id);
		free (function_id);
		free (concat1);
		free (concat2);
	}
	//printf("Function ID: %d\n",func_id);

	FILE *file;
	char* path = concat(datasets_path,"functions.CSV");

	if ((file = load_file(path)) == NULL) {
		free(path);
		return EXIT_FAILURE;
	}
	free(path);

	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	int lineID;
	short match=0;

	if (file == NULL)
        exit(0);

	if (file){
		while (((read =getline(&line, &len, file)) != -1)) {
		    char *line2 = line;
		    int lineID = (int)strtol((strtok(line2, ";")),((char **)NULL),10);
		    //printf("lineID: %d\n", lineID);
				if (lineID == func_id) {
					char* func_name_update = strtok(NULL,"\n");
					strcpy(func_name,func_name_update);
					match=1;
					break;
				}
			}
		if (match == 0){
			strcpy(func_name,"unknown");
			printf("Function not found.\n");
		}
		else {
			printf("Function: %s \n",func_name);
		}
	}

	else{
		printf("File could not be opened.");
	}

	free(line);

	fclose(file);
	
	return 0;

}

int parse_get_class_industry(u_int64_t can_data, char* class_name, char* industry_name) {
	int class_id = parse_get_class_id(can_data);
	int industry_id = parse_get_industry_id(can_data);
	
	//printf("Function ID: %d\n",func_id);

	FILE *file;

	char* path = concat(datasets_path,"industry_class.CSV");

	if ((file = load_file(path)) == NULL) {
		free(path);
		return EXIT_FAILURE;
	}
	free(path);

	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	int lineID;
	short industry_match=0;
	short class_match=0;

	if (file == NULL)
        exit(0);

	if (file){
		while (((read =getline(&line, &len, file)) != -1)) {
		    char* line2 = line;
		    int lineID = (int)strtol((strtok(line2, ";")),((char **)NULL),10);
		    //printf("lineID: %d\n", lineID);
				if (lineID == industry_id) {
					
					int lineID2 = (int)strtol((strtok(NULL, ";")),((char **)NULL),10);
					char* industry_name_update = strtok(NULL,";");
					strcpy(industry_name,industry_name_update);

					industry_match=1;

					if (lineID2 == class_id) {
						char* class_name_update = strtok(NULL,"\n\r\t");
						strcpy(class_name,class_name_update);
						class_match=1;
						fprintf(stdout, "Found class match - ID: %d, Name: %s\n",class_id,class_name);
						break;
					}
				
				}
			
			}

		if (industry_match == 0){
			strcpy(industry_name,"unknown");
			printf("Industry not found.\n");
		}
		else {
			printf("Industry: %s \n",industry_name);
		}

		if (class_match == 0){
			strcpy(class_name,"unknown");
			printf("Class not found.\n");
		}

	}

	else{
		printf("File could not be opened.");
	}

	free(line);
	fclose(file);
	
	return 0;

}




/*
*   \brief Get PGN
*
*	Extracting the PGN from the CAN identifier
*/

int parse_get_pgn(u_int32_t can_id) {
	int PGN;

	//clear lower 3 bits from canID, because its a 29bit identifier and
	can_id = (can_id & create_mask(0,28));

	//GET PF (PDU Format) from Identifier if PF<= 239 -> PDU1, else PDU2
	int PF = (can_id >> 16) & create_mask(0,7);

	if (PF < 240) {
		PGN = (can_id >> 16) & create_mask(0,9);
	}

	else {
		PGN = (can_id >> 8) & create_mask(0,17);
	}

	//printf("PF is: %d - PGN is: %d\n",PF, PGN );
	return PGN;

}


/*
*   \brief Parse PGN name
*
*	Looking up PGN in file and get the name of the PGN.
*/
int parse_get_pgn_name(int pgn_id, char* pgn_name) {
	
	FILE *file;
	char* path = concat(datasets_path,"parametergroups.CSV");
	if ((file = load_file(path)) == NULL) {
		free(path);
		return EXIT_FAILURE;
	}
	free(path);

	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	short match = 0;
	int lineID;

	int pgn_type = 0; //0 = Unknown, 1= ISO11783, 2=J1939, 3=NMEA2000

	if (file == NULL)
        exit(0);

	if (file){
		while (((read =getline(&line, &len, file)) != -1)) {
		    char* line2 = line;
		    int lineID = (int)strtol((strtok(line, ";")),((char **)NULL),10);
		    //printf("lineID: %d\n", lineID);
				if (lineID == pgn_id){
					char* pgn_name_update = strtok(NULL,";");

					char* pgn_type_c = strtok(NULL,"\n");

					fprintf(stdout,"FIRST CHAR: %c\n", pgn_type_c[0]);

					if (pgn_type_c != NULL) {
						switch (pgn_type_c[0]) {
							case 'I':
								pgn_type = 1;
								break;
							case 'J':
								pgn_type = 2;
								break;
							case 'N':
								pgn_type = 3;
								break;
							default:
								pgn_type = 0;
								break;
						}
					}
					else {
						fprintf(stderr, "Error in structure of parametergroups.csv\n");
					}

					strcpy(pgn_name,pgn_name_update);
					
					printf("Found PGN Name: ");
					match = 1;
					
					break;
				}
			}
		
		if (match == 0){
			pgn_name="unknown";
			printf("PGN name not found.\n");
		}
		
		else {
			printf("%s \n",pgn_name);
		}
	
	}

	else{
		printf("File could not be opened.");
	}

	free(line);

	fclose(file);

	return pgn_type;


}


/*
*   \brief Retrieve SPNs
*
*	Take a PGN and look up all the SPNs known.
*/
int parse_get_signals(int pgn_id, short round,signal_t* signal_spn) {

	printf("Extracting Signals for PGN\n");

	FILE *file;
	char* path = concat(datasets_path,"signals.CSV");
	if ((file = load_file(path)) == NULL) {
		free(path);
		return EXIT_FAILURE;
	}
	free(path);
	char * line = NULL;
	int lineID;
	size_t len = 0;
	ssize_t read;

	short counter = 0;

	if (file == NULL)
        exit(0);

	if (file){
		
		signal_spn->spn = 0;

		while (((read =getline(&line, &len, file)) != -1)) {
		    
		    char* line2 = line;
		    int lineID = (int)strtol((strtok(line, ";")),((char **)NULL),10);
		    //printf("lineID: %d\n", lineID);
				if (lineID == pgn_id){
					counter += 1;
					if (counter == round) {

						signal_spn->spn = str_to_int(strtok(NULL, ";"));

						strcpy(signal_spn->name,strtok(NULL,";"));

						signal_spn->start_bit = str_to_int((strtok(NULL,";")));

						signal_spn->len = str_to_int((strtok(NULL, "\n")));

						printf("Signal SPN: %s found.\n", signal_spn->name);

						break;
					}
				}
			}
	}

	else{
		printf("File could not be opened.");
	}

	free(line);

	fclose(file);

	return signal_spn->spn;


}