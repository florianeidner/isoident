int parse_get_device_id(u_int64_t);

int parse_get_manufac_id(u_int64_t);

int parse_get_function_id(u_int64_t);

int parse_get_industry_id(u_int64_t);

int parse_get_class_id(u_int64_t);

int parse_get_manufacturer(u_int64_t, char*);

int parse_get_function(u_int64_t, char*);

int parse_get_pgn(u_int32_t);

int parse_get_pgn_name(int, char*);

int parse_get_signals(u_int32_t, short,signal_t*);

int parse_get_class_industry(u_int64_t, char*,char*);