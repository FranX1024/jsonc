#include <stdlib.h>
#include <string.h>
#include "jsonc.h"
#include "utils.h"

JSON json_number(double number) {
  JSON json;
  json.type = JSON_NUMBER;
  json.data = *(U64*)&number;
  return json;
}

double json_get_number(JSON json) {
  if(json.type != JSON_NUMBER)
    return F64NAN;
  return *(double*)&json.data;
}

/* json_string will create a copy of the
   given c string. This is done this way so
   that when json_drop is called, objects
   created by the user aren't deleted
   
   json_get_string, on the other hand,
   does not copy the string before returning
   it.
*/

JSON json_string(const char* string) {
  JSON json;
  json.type = JSON_STRING;
  char* copied = json_copy_cstring(string);
  json.data = *(U64*)&copied;
  return json;
}

char* json_get_string(JSON json) {
  if(json.type != JSON_STRING)
    return 0;
  return *(char**)&json.data;
}

/* extendable array, every push
   that exceeds real_size doubles
   the array real_size.
*/

JSON json_make_array(unsigned int size) {
  JSON json;
  JSON_ARRAY_t* arr;
  unsigned int real_size = 8;
  while(real_size < size) real_size <<= 1;
  arr = (JSON_ARRAY_t*)malloc(sizeof(JSON_ARRAY_t));
  arr->data = (JSON*)malloc(real_size * sizeof(JSON));
  arr->size = size;
  arr->real_size = real_size;
  for(U32 i = 0; i < size; i++) {
    JSON el;
    el.type = JSON_NULL;
    el.data = 0;
    arr->data[i] = el;
  }
  json.type = JSON_ARRAY;
  json.data = (U64)arr;
  return json;
}

void json_push(JSON json, JSON value) {
  if(json.type == JSON_ARRAY) {
    JSON_ARRAY_t* arr = (JSON_ARRAY_t*)json.data;
    if(arr->size == arr->real_size) {
      arr->real_size <<= 1;
      arr->data = (JSON*)realloc(arr->data, arr->real_size * sizeof(JSON));
    }
    arr->data[arr->size++] = value;
  }
}

JSON json_pop(JSON json) {
  if(json.type != JSON_ARRAY)
    return JSON_NIL;
  JSON_ARRAY_t* arr = (JSON_ARRAY_t*)json.data;
  if(arr->size == 0)
    return JSON_NIL;
  arr->size--;
  return arr->data[arr->size];
}

// Attention: no checks for out of bounds

void json_setf(JSON json, unsigned int ind, JSON val) {
  if(json.type == JSON_ARRAY) {
    JSON_ARRAY_t* arr = (JSON_ARRAY_t*)json.data;
    arr->data[ind] = val;
  }
}

JSON json_getf(JSON json, unsigned int ind) {
  if(json.type != JSON_ARRAY) {
    return (JSON){.type = JSON_NULL, .data = 0};
  }
  JSON_ARRAY_t* arr = (JSON_ARRAY_t*)json.data;
  return arr->data[ind];
}

U32 json_length(JSON json) {
  return ((JSON_ARRAY_t*)json.data)->size;
}

/* Hash tables begin with the size of 16
   and double once the number of entries
   exceeds 80% * number of buckets.
   Each time a hash table grows
   all entries get rehashed.
*/

JSON json_make_table() {
  JSON json;
  unsigned int bucket_count = 16;
  JSON_TABLE_t* table = (JSON_TABLE_t*)malloc(sizeof(JSON_TABLE_t));
  table->buckets = (JSON_CONS_t**)malloc(bucket_count * sizeof(JSON_CONS_t*));
  table->entry_count = 0;
  table->bucket_count = bucket_count;
  for(U32 i = 0; i < bucket_count; i++) {
    table->buckets[i] = (JSON_CONS_t*)0;
  }
  json.type = JSON_TABLE;
  json.data = (U64)table;
  return json;
}

/* only strings are permitted as table keys
   source: json.org
*/

void json_seth(JSON json, const char* key, JSON value) {
  if(json.type != JSON_TABLE)
    return;
    
  JSON_TABLE_t* table = (JSON_TABLE_t*)json.data;
  
  U32 hash = json_hash_cstring(key) & (table->bucket_count - 1);

  if(table->buckets[hash] == (JSON_CONS_t*)0) {
    
    JSON_CONS_t* cons = (JSON_CONS_t*)malloc(sizeof(JSON_CONS_t));
    cons->next = 0;
    cons->key = json_copy_cstring(key);
    cons->value = value;
    table->buckets[hash] = cons;
    table->entry_count++;
  } else {
    
    JSON_CONS_t* last = table->buckets[hash];
    bool already_in_table = false;
    while(true) {
      if(!strcmp(last->key, key)) {
	
	last->value = value;
	already_in_table = true;
	break;
      }
      if(last->next == 0) break;
      last = last->next;
    }
    if(!already_in_table) {
      JSON_CONS_t* cons = (JSON_CONS_t*)malloc(sizeof(JSON_CONS_t));
      cons->next = (JSON_CONS_t*)0;
      cons->key = json_copy_cstring(key);
      cons->value = value;
      last->next = cons;
      table->entry_count++;
    }
  }
  // check if table needs to be resized
  if(table->entry_count * 5 > table->bucket_count * 4) {
    
    U32 bucket_count = (table->bucket_count << 1);
    JSON_TABLE_t* table2 = (JSON_TABLE_t*)malloc(sizeof(JSON_TABLE_t));
    table2->buckets = (JSON_CONS_t**)malloc(bucket_count * sizeof(JSON_CONS_t*));
    table2->bucket_count = bucket_count;
    table2->entry_count = 0;

    JSON json2 = {.type = JSON_TABLE, .data = (U64)table2};
    
    JSON_TABLE_iterator* it = json_table_iterator(json);
    while(it != 0) {
      json_seth(json2, it->entry->key, it->entry->value);
      JSON_CONS_t* cons = it->entry;
      it = json_next(it);
      free(cons);
    }

    free(table->buckets);
    table->buckets = table2->buckets;
    table->bucket_count = table2->bucket_count;
    free(table2);
  }
}

JSON json_geth(JSON json, const char* key) {
  if(json.type != JSON_TABLE)
    return JSON_NIL;

  JSON_TABLE_t* table = (JSON_TABLE_t*)json.data;
  U32 hash = json_hash_cstring(key) & (table->bucket_count - 1);

  if(table->buckets[hash] == (JSON_CONS_t*)0)
    return JSON_NIL;
  
  JSON_CONS_t* cons = table->buckets[hash];
  while(cons) {
    if(!strcmp(cons->key, key))
      return cons->value;
    cons = cons->next;
  }
  return JSON_NIL;
}

void json_delete(JSON json, const char* key) {
  if(json.type != JSON_TABLE)
    return;

  JSON_TABLE_t* table = (JSON_TABLE_t*)json.data;
  U32 hash = json_hash_cstring(key) & (table->bucket_count - 1);

  if(table->buckets[hash] == (JSON_CONS_t*)0)
    return;

  JSON_CONS_t* cons = table->buckets[hash];
  JSON_CONS_t* last = (JSON_CONS_t*)0;
  while(cons) {
    if(!strcmp(cons->key, key)) {
      if(!last)
	table->buckets[hash] = cons->next;
      else
	last->next = cons->next;
      free(cons);
      table->entry_count--;
      return;
    }
    cons = cons->next;
    last = cons;
  }
}

JSON_TABLE_iterator* json_table_iterator(JSON json) {
  if(json.type != JSON_TABLE)
    return (JSON_TABLE_iterator*)0;

  JSON_TABLE_t* table = (JSON_TABLE_t*)json.data;
  U32 current_bucket = 0;
  while(!table->buckets[current_bucket] && current_bucket < table->bucket_count) current_bucket++;
  if(current_bucket == table->bucket_count)
    return (JSON_TABLE_iterator*)0;
  
  JSON_TABLE_iterator* it = (JSON_TABLE_iterator*)malloc(sizeof(JSON_TABLE_iterator));
  it->current_bucket = current_bucket;
  it->entry = table->buckets[current_bucket];
  it->table = table;

  return it;
}

JSON_TABLE_iterator* json_next(JSON_TABLE_iterator* it) {
  if(it->entry->next) {
    it->entry = it->entry->next;
  } else {
    U32 current_bucket = it->current_bucket + 1;
    JSON_TABLE_t* table = it->table;
    while(!table->buckets[current_bucket] && current_bucket < table->bucket_count) current_bucket++;
    if(current_bucket == table->bucket_count) {
      free(it);
      it = (JSON_TABLE_iterator*)0;
    } else {
      it->current_bucket = current_bucket;
      it->entry = table->buckets[current_bucket];
    }
  }
  return it;
}

/* json_drop deletes the entire subtree of
   json objects.
*/

void json_drop(JSON root) {
  switch(root.type) {
  case JSON_NUMBER:
  case JSON_STRING:
  case JSON_NULL:
  case JSON_BOOL:
    break;
    
  case JSON_ARRAY:
    for(U32 i = 0; i < json_length(root); i++)
      json_drop(json_getf(root, i));
    JSON_ARRAY_t* arr = (JSON_ARRAY_t*)root.data;
    free(arr->data);
    free(arr);
    break;

  case JSON_TABLE:
    for(JSON_TABLE_iterator* it = json_table_iterator(root); it != 0; it = json_next(it)) {
      json_drop(it->entry->value);
      free(it->entry->key);
    }
    JSON_TABLE_t* table = (JSON_TABLE_t*)root.data;
    free(table->buckets);
    free(table);
  }
}
