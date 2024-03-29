#ifndef JSONC
#define JSONC

#ifndef U64
#define U64 unsigned long long int
#endif

#ifndef U32
#define U32 unsigned int
#endif

#ifndef false
#define false 0
#endif

#ifndef true
#define true 1
#endif

#ifndef bool
#define bool char
#endif

#ifndef F64NAN
#define F64NAN 1e308;
#endif

#define JSON_NULL   0
#define JSON_NUMBER 1
#define JSON_STRING 2
#define JSON_ARRAY  3
#define JSON_TABLE  4
#define JSON_BOOL   5

#define JSON_TRUE   (JSON){.type = JSON_BOOL, .data = 1}
#define JSON_FALSE  (JSON){.type = JSON_BOOL, .data = 0}
#define JSON_NIL    (JSON){.type = JSON_NULL, .data = 0}

typedef struct {
  char type;
  U64 data;
} JSON;

JSON   json_number(double number);
double json_get_number(JSON json);

JSON   json_string(const char* string);
char*  json_get_string(JSON json);

typedef struct {
  unsigned int size;
  unsigned int real_size;
  JSON *data;
} JSON_ARRAY_t;

JSON  json_make_array(unsigned int size);
void  json_push(JSON json, JSON value);
JSON  json_pop(JSON json);
void  json_setf(JSON json, unsigned int ind, JSON val);
JSON  json_getf(JSON json, unsigned int ind);
U32    json_length(JSON json);

typedef struct JSON_CONS_t {
  struct JSON_CONS_t* next;
  char* key;
  JSON value;
} JSON_CONS_t;

typedef struct {
  unsigned int entry_count;
  unsigned int bucket_count;
  JSON_CONS_t** buckets;
} JSON_TABLE_t;

typedef struct {
  JSON_TABLE_t* table;
  JSON_CONS_t* entry;
  unsigned int current_bucket;
} JSON_TABLE_iterator;

JSON  json_make_table();
void  json_seth(JSON json, const char* key, JSON value);
JSON  json_geth(JSON json, const char* key);
void  json_delete(JSON json, const char* key);

JSON_TABLE_iterator* json_table_iterator(JSON json);
JSON_TABLE_iterator* json_next(JSON_TABLE_iterator* it);

void  json_drop(JSON root);

JSON  json_parse(const char* raw);
char* json_stringify(JSON data);

#endif
