#ifndef __json_h__
#define __json_h__

typedef enum json_type_e json_type_e;

typedef struct json_value_s json_value_s;
typedef struct json_object_s json_object_s;
typedef struct json_array_s json_array_s;
typedef struct json_string_s json_string_s;
typedef struct json_number_s json_number_s;

enum json_type_e {
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_STRING,
	JSON_NUMBER,
	JSON_TRUE,
	JSON_FALSE
};

extern int json_parse(char *src);

#endif

