char* datasets_path;

int str_to_int(char* p);

char *int_to_string(int value);

FILE* load_file(const char *path);

int file_exists (const char *path);

typedef struct {
    int *array;
    size_t used;
    size_t size;
} dynArray;

typedef struct {
	char* name;
	int spn;
	u_int32_t start_bit; //source address
	u_int32_t len;
} signal_t;

void initArray(dynArray *a, size_t initialSize);

void insertArray(dynArray *a, int element);

void freeArray(dynArray *a);

const char* get_process_name_by_pid(const int pid);

char* concat(const char *s1, const char *s2);