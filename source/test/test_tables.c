#include <stdlib.h>
#include <stdio.h>
#include "../jsonc.h"

const U4 ENTRY_COUNT = 1000;

int main() {
  JSON json = json_make_table();
  printf("writing...\n");
  for(int i = 0; i < ENTRY_COUNT; i++) {
    char* str = (char*)malloc(6);
    sprintf(str, "%d", i);
    json_seth(json, str, json_number(i * 0.5));
    free(str);
  }
  printf("reading...\n");
  for(int i = 0; i < ENTRY_COUNT; i++) {
    char* str = (char*)malloc(6);
    sprintf(str, "%d", i);
    JSON jval = json_geth(json, str);
    double x = json_get_number(jval);
    if(x != 0.5 * i) {
      printf("ERROR: %s -> %.6f\n", str, x);
      free(str);
      return 0;
    }
    free(str);
  }
  printf("Iterating...\n");
  int entry_count = 0;
  for(JSON_TABLE_iterator* it = json_table_iterator(json); it != 0; it = json_next(it)) {
    int num = 0;
    char* key = it->entry->key;
    double val = json_get_number(it->entry->value);
    for(U4 i = 0; key[i] != 0; i++)
      num = 10 * num + key[i] - '0';
    if((double)num * 0.5 != val) {
      printf("ERROR: %s -> %.6f\n", key, val);
      return 0;
    }
    entry_count++;
  }
  if(entry_count != ENTRY_COUNT)
    printf("ERROR: entry count not matching %d vs %d\n", entry_count, ENTRY_COUNT);
  else
    printf("done!\n");
  return 0;
}
