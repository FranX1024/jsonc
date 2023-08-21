# JSONC
JSONC is a JSON library written in C. It provides a JSON parser, JSON stringify function and JSON object implementation.

## including the library

Library consists of 2 parts - the header file (/source/jsonc.h) and the object library (/lib/libjsonc.a).
So, assuming you have a c file in the same directory as this repository, you would include the header file
```c
#include <stdio.h>
#include "jsonc/source/jsonc.h"

int main() {
  JSON data = json_parse("{\"greeting\": \"Hello world!\"}");
  char* greeting = json_get_string(json_geth(data, "greeting"));
  printf("%s\n", greeting);
  json_drop(data);
  return 0;
}
```
And to compile you would run
```
gcc program.c -o program -Ljsonc/lib -ljsonc
```

## JSON objects

All JSON objects are contained within the same type, `JSON`. `JSON` is a struct consisting of 1 byte denoting type and 8 bytes denoting either data or a pointer.
These attributes are named `.type` and `.data`.

### types

- number (type `JSON_NUMBER`)
- boolean (type `JSON_BOOL`, instances `JSON_TRUE` and `JSON_FALSE`)
- null (type `JSON_NULL`, instance `JSON_NIL`)
- string (type `JSON_STRING`)
- array (type `JSON_ARRAY`)
- hashmap (type `JSON_TABLE`)

### functions

```
JSON   json_number(double number);
```
- converts a double to JSON number
```
double json_get_number(JSON json);
```
- converts a JSON number to double

```
JSON   json_string(const char* string);
```
- converts a C string to JSON string. The provided C string is copied internally, so it can be deallocated while you're still using the JSON string.
```
char*  json_get_string(JSON json);
```
- converts a JSON string to a C string. Unlike the previous function, this one does not create a copy before returning the C string. Do not deallocate the returned C string.

```
JSON  json_make_array(unsigned int size);
```
- creates an instance of JSON array
```
void  json_push(JSON json, JSON value);
```
- pushes a new element to the JSON array
```
JSON  json_pop(JSON json);
```
- pops the last element of the JSON array
```
void  json_setf(JSON json, unsigned int ind, JSON val);
```
- sets the value of a specific index in the array
```
JSON  json_getf(JSON json, unsigned int ind);
```
- gets the value of a specific index in the array
```
U4    json_length(JSON json);
```
- gets the length of the array

```
JSON  json_make_table();
```
- creates a json table instance. Keys are exclusively strings.
```
void  json_seth(JSON json, const char* key, JSON value);
```
- sets the value associated with the specified key.
```
JSON  json_geth(JSON json, const char* key);
```
- gets the value associated with the specified key.
```
void  json_delete(JSON json, const char* key);
```
- deletes the entry associated with the speicfied key.

```
JSON_TABLE_iterator* json_table_iterator(JSON json);
```
- creates an iterator object for a table. This is used for iteration over all entries in a table.
```
JSON_TABLE_iterator* json_next(JSON_TABLE_iterator* it);
```
- gets the next pointer to the iterator object. In reality, this function returns the same pointer every time unless if the end of the table had been reached. In that case it returns NULL pointer.

```
void  json_drop(JSON root);
```
- deallocate JSON object tree. If you wish to preserve some children of a JSON array or table, set the value associated with their index/key to JSON_NIL before calling json_drop.
- NOTE: do NOT make cyclic JSON objects. If you do, this function will produce a SEGFAULT.

```
JSON  json_parse(const char* raw);
```
- parses a C string into a JSON object
```
char* json_stringify(JSON data);
```
- converts a JSON object to a JSON C string
