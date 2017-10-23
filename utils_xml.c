/** /file utils_xml.c 	
*	/brief Parsing the keys for manufacturer, pgn ,spn and function
*
*	This module takes the adress-claimed message (PGN 60928) and uses
*	the NAME from the CAN Data field to retrieve manufacturer, function
*	and class of the sender from the iso 11783 NAME list.
*
*	/author Florian Eidner
*/

#include <stdio.h>
#include <stdlib.h>
#include "mxml-2.10/mxml.h"
#include <time.h>
#include <unistd.h>


#include "utils_general.h"
#include "utils_parse.h"




int xml_add_device(mxml_node_t* tree, int device_id, u_int64_t data) {
	
	printf("Add device to xml\n");
	
	char man_name[50]={0}; //Name max is 50 chars, Database max name length is.
	parse_get_manufacturer(data,man_name);
	
	char func_name[50]={0};
	parse_get_function(data,func_name);
	
	char class_name[50]={0};
	char industry_name[50]={0};

	parse_get_class_industry(data,class_name,industry_name);

	char* uuid = int_to_string(device_id);


	
	mxml_node_t* device = mxmlNewElement(tree,"device");

	mxmlElementSetAttr(device, "UUID", uuid);
	mxmlElementSetAttr(device, "manufacturer", man_name);
	mxmlElementSetAttr(device, "function", func_name);
	mxmlElementSetAttr(device,"class",class_name));
	mxmlElementSetAttr(device,"industry",industry_name);
	free(uuid);
	return EXIT_SUCCESS;

}

/*
*   \brief Add message to xml-tree
*	Adds a detected message and its signals to the isoident.xml tree.
*/

int xml_add_message(mxml_node_t* device, int message_pgn) {
	printf("Add message PGN: %d to device.\n",message_pgn);

	mxml_node_t* message = mxmlNewElement(device,"message");

	char pgn_name[50]={0};
	
	int pgn_type = parse_get_pgn_name(message_pgn, pgn_name);
	
	char* pgn = int_to_string(message_pgn);

	printf("Found device with UUID: %d\n",str_to_int((char*)mxmlElementGetAttr(device,"UUID")));
	
	mxmlElementSetAttr(message, "pgn", pgn);
	
	mxmlElementSetAttr(message, "name", pgn_name);
	
	free(pgn);

	switch (pgn_type) {
		case 1:
			mxmlElementSetAttr(message, "type", "ISO11783");
			break;

		case 2:
			mxmlElementSetAttr(message, "type", "J1939");
			break;

		case 3:
			mxmlElementSetAttr(message, "type", "NMEA2000");
			break;

		default:
			mxmlElementSetAttr(message, "type", "unknown");
			break;
	}

	//Add signals to message.
	int i;
	mxml_node_t* sig;
	signal_t signal_spn;
	signal_spn.name = malloc(50);
	char* spn = int_to_string(signal_spn.spn);
	char* start_bit = int_to_string(signal_spn.start_bit);
	char* spn_len = int_to_string(signal_spn.len);

	for (i=1;(parse_get_signals(message_pgn,i,&signal_spn) != 0);i++) {
	 	sig = mxmlNewElement(message,"signal");
	 	printf("Signal spn after:%s\n", spn);
	 	mxmlElementSetAttr(sig, "spn", spn);
	 	mxmlElementSetAttr(sig, "name", signal_spn.name);
	 	mxmlElementSetAttr(sig, "log", "0");
	 	mxmlElementSetAttr(sig, "start", start_bit);
	 	mxmlElementSetAttr(sig, "len", spn_len);
	 	mxmlElementSetAttr(sig, "end", "0");
	 	mxmlElementSetAttr(sig, "fac", "0");
	 	mxmlElementSetAttr(sig, "offs", "0");
	 	mxmlElementSetAttr(sig, "min", "-800000");
	 	mxmlElementSetAttr(sig, "max", "800000");
	 	mxmlElementSetAttr(sig, "type", "");
	 	mxmlElementSetAttr(sig, "unit", "");
	 	mxmlElementSetAttr(sig, "ddi", "");

	 	printf("Added Signal to message.\n");

	}
	free(spn);
	free(spn_len);
	free(start_bit);
	free (signal_spn.name);

	return EXIT_SUCCESS;
}


// /*
// *   \brief Write isoident.xml
// *	Takes the current tree and save it in file.
// */

int xml_write_file(const char* path, const char* parentname, mxml_node_t* node1, mxml_node_t* node2,mxml_node_t* node3) {
	
	fprintf(stdout, "Writing isoident.xml...");
	
	FILE *xmlFile;
	
	mxml_node_t *xml;
	
	mxml_node_t *parent;

	xml = mxmlNewXML("1.0");

	parent = mxmlNewElement(xml,parentname);

	mxmlAdd(parent,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node1);
	mxmlAdd(parent,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node2);
	mxmlAdd(parent,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node3);

	xmlFile = fopen(path,"w");
	
	mxmlSaveFile(xml,xmlFile,MXML_NO_CALLBACK);

	mxmlRemove(node1);
	mxmlRemove(node2);
	mxmlRemove(node3);


	fclose(xmlFile);

	mxmlDelete(xml);

	fprintf(stdout,"Successfully updated file.\n");
	
	return 1;
}

