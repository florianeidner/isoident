int xml_add_device(mxml_node_t * tree, int device_id, u_int64_t data,int sa);

int xml_update_device(mxml_node_t* tree, int device_sa, int device_uuid);

int xml_add_message(mxml_node_t* device, int message_pgn);

int xml_write_file(const char* path, mxml_node_t* parentnode, mxml_node_t* node1, mxml_node_t* node2, mxml_node_t* node3, mxml_node_t* node4);