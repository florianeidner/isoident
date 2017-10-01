#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <math.h>

#include "utils_general.h"

int str_to_int(char* p) {
    //fprintf(stdout,"Parsing '%s':\n", p);
    char *end;
    for (long i = strtol(p, &end, 10);
         p != end;
         i = strtol(p, &end, 10))
    {
        //printf("'%.*s' -> ", (int)(end-p), p);
        p = end;
        if (errno == ERANGE){
            fprintf(stderr,"range error, got ");
            errno = 0;
        }
        //printf("%ld\n", i);
        return i;
    }
    return EXIT_FAILURE;
}

char *int_to_string(int value) {
    
    int digitcount;
    char * tmp_string;
    int increment = 2; // one for rounding, one for '\0' terminator
    if(value <0){
            increment += 1; // make room for sign 
    }
    if(0 == value){
            tmp_string = malloc(2* sizeof(char));
            sprintf(tmp_string, "%u", 0);
    }
    else{
            digitcount = (int)floor(log10((double)abs(value)))+ increment;
            tmp_string = malloc(digitcount * sizeof(char));
            sprintf(tmp_string, "%d", value);
    }
    return tmp_string;
}


/*
*   \brief Check file availability
* 
*   Checking if file exists. Returns 1 if file exists
*/
int file_exists (const char *path) {
	if( access( path, F_OK ) != -1 ) {
    	return 1;
	} else {
    	return 0;
	}
}


/*
*   \brief Load file
* 
*   Loading file from given path to given file var.
*/
FILE* load_file(const char *path) {
	if (file_exists(path)) {
		
		fprintf(stdout,"File found: %s \n",path);
		
		FILE* tempfile = fopen(path,"r");

		return tempfile;
		
		}

	else {
		fprintf(stderr,"File not found: %s \n",path);
		return NULL;
	}
}

const char* get_process_name_by_pid(const int pid)
{
    char* name = (char*)calloc(1024,sizeof(char));
    if(name){
        sprintf(name, "/proc/%d/cmdline",pid);
        FILE* f = fopen(name,"r");
        if(f){
            size_t size;
            size = fread(name, sizeof(char), 1024, f);
            if(size>0){
                if('\n'==name[size-1])
                    name[size-1]='\0';
            }
            fclose(f);
        }
    }

    printf("Process name: %s\n", name );

    return name;
}

void initArray(dynArray *a, size_t initialSize) {
	a->array = (int *)malloc(initialSize * sizeof(int));
	a->used = 0;
	a->size = initialSize;
}

void insertArray(dynArray *a, int element) {
  // a->used is the number of used entries, because a->array[a->used++] updates a->used only *after* the array has been accessed.
  // Therefore a->used can go up to a->size 
  if (a->used == a->size) {
    a->size *= 2;
    a->array = (int *)realloc(a->array, a->size * sizeof(int));
  }
  a->array[a->used++] = element;
}

void freeArray(dynArray *a) {
  free(a->array);
  a->array = NULL;
  a->used = a->size = 0;
}

char* concat(const char *s1, const char *s2){
    //printf ("Merging strings\n");
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}