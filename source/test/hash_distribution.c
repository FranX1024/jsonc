#include <stdlib.h>
#include <stdio.h>
#include "../jsonc.h"

const U4 ENTRY_COUNT = 819;
U4 bucket_counts[2048];
U4 buckets_of_size[2048];

int main() {
  JSON json = json_make_table();
  for(U4 i = 0; i < ENTRY_COUNT; i++) {
    char* key = (char*)malloc(10);
    sprintf(key, "key %u", i);
    json_seth(json, key, json_number((double) i * 0.1));
    free(key);
  }
  for(JSON_TABLE_iterator* it = json_table_iterator(json); it != 0; it = json_next(it)) {
    bucket_counts[it->current_bucket]++;
  }
  for(U4 i = 0; i < 2048; i++) {
    buckets_of_size[bucket_counts[i]]++;
  }
  for(U4 i = 0; i < 2048; i++) {
    if(buckets_of_size[i] != 0) {
      printf("%u x size %u\n", buckets_of_size[i], i);
    }
  }
  json_drop(json);
  return 0;
}
