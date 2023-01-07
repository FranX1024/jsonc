#define JSON_NUM 0
#define JSON_STR 1
#define JSON_ARR 2
#define JSON_MAP 3

typedef union {
  char* str;
  double num;
  hmap* map;
  dvec* arr;
} json_v;

typedef struct {
  uint type;
  json_v value;
} json_t;

json_t* json_init(uint type, json_v value);
json_t* json_get_str(json_t* obj, char* str);
json_t* json_get_num(json_t* obj, double num);
json_t* json_drop(json_t* obj);

uint json_hash(const void* arg);
bool json_cmp(const void* a1, const void* a2);
