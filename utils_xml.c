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
#include <unistd.h>


#include "utils_general.h"
#include "utils_parse.h"




int xml_add_device(mxml_node_t* tree, int device_id, u_int64_t data,int sa) {
	
	fprintf(stdout,"Add device to xml\n");
	
	char man_name[200]={0}; //Name max is 200 chars, Database max name length is.
	char func_name[200]={0};
	char class_name[200]={0};
	char industry_name[200]={0};

	parse_get_function(data,func_name);
	parse_get_class_industry(data,class_name,industry_name);
	parse_get_manufacturer(data,man_name);

	char* uuid = int_to_string(device_id);
	char* device_sa = int_to_string(sa);

	char date_time[70];

	get_time(date_time);
	
	mxml_node_t* device = mxmlNewElement(tree,"device");

	mxmlElementSetAttr(device, "UUID", uuid);
	mxmlElementSetAttr(device, "manufacturer", man_name);
	mxmlElementSetAttr(device, "function", func_name);
	mxmlElementSetAttr(device, "class",class_name);
	mxmlElementSetAttr(device, "industry",industry_name);
	mxmlElementSetAttr(device, "lastClaim",date_time);
	mxmlElementSetAttr(device, "lastSA",device_sa);
	mxmlElementSetAttr(device, "status","online");
	
	free(device_sa);
	free(uuid);

	return EXIT_SUCCESS;

}

int xml_update_device(mxml_node_t* tree, int device_sa, int device_uuid) {

		char date_time[70];
		
		get_time(date_time);

		char* active_device_sa = int_to_string(device_sa);
		char* active_device_uuid = int_to_string(device_uuid);

		mxml_node_t* device = mxmlFindElement(tree,tree,"device","UUID",active_device_uuid,MXML_DESCEND);
		
		mxmlElementSetAttr(device,"lastClaim",date_time);
		mxmlElementSetAttr(device,"lastSA",active_device_sa);
		mxmlElementSetAttr(device,"status","online");

		free(active_device_uuid);
		free(active_device_sa);

		return EXIT_SUCCESS;
}

/*
*   \brief Add message to xml-tree
*	Adds a detected message and its signals to the isoident.xml tree.
*/

int xml_add_message(mxml_node_t* device, int message_pgn) {
	fprintf(stdout,"Add message PGN: %d.\n",message_pgn);

	mxml_node_t* message = mxmlNewElement(device,"message");

	char pgn_name[200]={0};
	
	int pgn_type = parse_get_pgn_name(message_pgn, pgn_name);
	
	char* pgn = int_to_string(message_pgn);
	
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
	
	signal_spn.name = malloc(200);
	signal_spn.unit = malloc (20);

	for (i=1;(parse_get_signals(message_pgn,i,&signal_spn) != 0);i++) {
	 	sig = mxmlNewElement(message,"signal");
	 	
	 	fprintf(stdout, "Here1\n");
	 	
	 	char* spn_id = int_to_string(signal_spn.spn);
		char* spn_start_bit = int_to_string(signal_spn.start_bit);
		char* spn_len = int_to_string(signal_spn.len);
		char* spn_factor = int_to_string(signal_spn.factor);
		char* spn_offset = int_to_string(signal_spn.offset);
		char* spn_range_min = int_to_string(signal_spn.range_min);
		char* spn_range_max = int_to_string(signal_spn.range_max);

	 	fprintf(stdout,"Signal spn after:%s\n", spn_id);
	 	mxmlElementSetAttr(sig, "spn", spn_id);
	 	mxmlElementSetAttr(sig, "name", signal_spn.name);
	 	mxmlElementSetAttr(sig, "log", "0");
	 	mxmlElementSetAttr(sig, "start", spn_start_bit);
	 	mxmlElementSetAttr(sig, "len", spn_len);
	 	mxmlElementSetAttr(sig, "end", "0");
	 	mxmlElementSetAttr(sig, "fac", spn_factor);
	 	mxmlElementSetAttr(sig, "offs", spn_offset);
	 	mxmlElementSetAttr(sig, "min", spn_range_min);
	 	mxmlElementSetAttr(sig, "max", spn_range_max);
	 	mxmlElementSetAttr(sig, "type", "");
	 	mxmlElementSetAttr(sig, "unit", signal_spn.unit);
	 	mxmlElementSetAttr(sig, "ddi", "");

	 	fprintf(stdout,"Added Signal to message.\n");

	 	free(spn_id);
		free(spn_len);
		free(spn_start_bit);
		free(spn_factor);
		free(spn_offset);
		free(spn_range_max);
		free(spn_range_min);

	}
	
	free (signal_spn.name);
	free (signal_spn.unit);

	return EXIT_SUCCESS;
}


// /*
// *   \brief Write isoident.xml
// *	Takes the current tree and save it in file.
// */

int xml_write_file(const char* path, mxml_node_t* parentnode, mxml_node_t* node1, mxml_node_t* node2,mxml_node_t* node3,mxml_node_t* node4) {
	
	fprintf(stdout, "Writing isoident.xml...");
	
	FILE *xmlFile;
	
	mxml_node_t *xml;
	
	mxml_node_t *parent;

	xml = mxmlNewXML("1.0");

	parent = mxmlNewElement(xml,"isoident");

	mxmlAdd(parent,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node1);
	mxmlAdd(parent,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node2);
	mxmlAdd(parent,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node3);
	mxmlAdd(parent,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node4);

	xmlFile = fopen(path,"w");
	
	mxmlSaveFile(xml,xmlFile,MXML_NO_CALLBACK);

	mxmlAdd(parentnode,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node1);
	mxmlAdd(parentnode,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node2);
	mxmlAdd(parentnode,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node3);
	mxmlAdd(parentnode,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node4);


	fclose(xmlFile);

	mxmlDelete(xml);

	fprintf(stdout,"Successfully updated file.\n");
	
	return EXIT_SUCCESS;
}

