#include <stdlib.h>
#include <stdio.h>

#include "../jsonc.h"

int main() {
  JSON json1 = json_number((double)5.0);
  JSON json2 = json_string("Hello world!");
  printf("%.6f\n", json_get_number(json1));
  printf("%.6f\n", json_get_number(json2));
  printf("%llu\n", json_get_string(json1));
  printf("%s\n", json_get_string(json2));
  return 0;
}
