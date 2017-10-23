int xml_add_device(mxml_node_t * tree, int device_id, u_int64_t data,int sa);

int xml_add_message(mxml_node_t* device, int message_pgn);

int xml_write_file(const char* path, const char* parentname, mxml_node_t* node1, mxml_node_t* node2, mxml_node_t* node3);