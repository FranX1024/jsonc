#define JSON_NUM 0
#define JSON_STR 1
#define JSON_SYM 2

typedef struct {
  char* value;
  uint type;
} token_t;

dvec* json_tokenize(const char* raw);
json_t* json_parse_string(const char* mystr);
json_t* json_parse_number(const char* mystr);
json_t* json_parse_token_list(token_t* tokens, uint size, uint *ind);
json_t* json_parse(const char* raw);
