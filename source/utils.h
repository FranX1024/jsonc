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
#define F64NAN 1e1023;
#endif

char* json_copy_cstring(const char* string);
U32    json_hash_cstring(const char* string);

typedef struct {
  U32 size;
  U32 real_size;
  char* string;
} json_StringBuilder;

json_StringBuilder* json_create_strb();
void  json_append_strb(json_StringBuilder* strb, const char* cstring);
char* json_finish_strb(json_StringBuilder* strb);


double json_string_to_number(const char* string);
char*  json_number_to_string(double number);
