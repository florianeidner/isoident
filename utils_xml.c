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

	
	mxml_node_t* device = mxmlNewElement(tree,"device");

	mxmlElementSetAttr(device, "UUID", int_to_string(device_id));
	mxmlElementSetAttr(device, "manufacturer", man_name);
	mxmlElementSetAttr(device, "function", func_name);
	// mxmlElementSetAttr(device,"class",parse_get_class(data));
	// mxmlElementSetAttr(device,"industry",parse_get_industry(data));
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
	parse_get_pgn_name(message_pgn, pgn_name);

	printf("Found device with UUID: %d\n",str_to_int((char*)mxmlElementGetAttr(device,"UUID")));
	mxmlElementSetAttr(message, "pgn", int_to_string(message_pgn));
	mxmlElementSetAttr(message, "name", pgn_name);

	//Add signals to message.
	int i;
	mxml_node_t* sig;
	signal_t signal_spn;
	signal_spn.name = malloc(50);

	for (i=1;(parse_get_signals(message_pgn,i,&signal_spn) != 0);i++) {
	 	sig = mxmlNewElement(message,"signal");
	 	printf("Signal spn after:%s\n", int_to_string(signal_spn.spn));
	 	mxmlElementSetAttr(sig, "spn", int_to_string(signal_spn.spn));
	 	mxmlElementSetAttr(sig, "name", signal_spn.name);
	 	mxmlElementSetAttr(sig, "log", "0");
	 	mxmlElementSetAttr(sig, "start", int_to_string(signal_spn.start_bit));
	 	mxmlElementSetAttr(sig, "len", int_to_string(signal_spn.len));
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

	free (signal_spn.name);

	return EXIT_SUCCESS;
}


// /*
// *   \brief Write isoident.xml
// *	Takes the current tree and save it in file.
// */

int xml_write_file(const char* path, const char* parentname, mxml_node_t* node1, mxml_node_t* node2) {
	
	fprintf(stdout, "Writing isoident.xml...");
	
	FILE *xmlFile;
	mxml_node_t *xml;
	mxml_node_t *parent;

	xml = mxmlNewXML("1.0");

	parent = mxmlNewElement(xml,parentname);

	mxmlAdd(parent,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node1);
	mxmlAdd(parent,MXML_ADD_AFTER,MXML_ADD_TO_PARENT,node2);

	xmlFile = fopen(path,"w");
	mxmlSaveFile(xml,xmlFile,MXML_NO_CALLBACK);
	fclose(xmlFile);
	fprintf(stdout,"Successfully updated file.\n");
	return 1;
}

