#include <stdio.h>
#include "../jsonc.h"

int main() {
  JSON data = json_parse("{\"Peter\": {\"abc\": 23} }");

  JSON peter = json_geth(data, "Peter");
  json_seth(peter, "abc", json_string("Hello \"fff\" world!"));
  
  char* stringified = json_stringify(data);
  printf("%s\n", stringified);

  free(stringified);
  json_drop(data);
  return 0;
}
