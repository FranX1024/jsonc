#include <stdio.h>
#include "../jsonc.h"

int main() {
  JSON data = json_make_table();

  JSON stuff = json_make_array(0);
  json_push(stuff, JSON_TRUE);
  json_push(stuff, JSON_FALSE);
  json_push(stuff, JSON_NIL);
  json_push(stuff, json_number(-2.3));
  json_push(stuff, json_string("Hello world!"));
  json_seth(data, "Array1", stuff);

  JSON thing2 = json_make_table();
  json_seth(thing2, "occupation", json_string("Leaf trimmer"));
  json_seth(thing2, "age", json_number(52.3059));
  json_seth(data, "Person", thing2);

  char* stringified = json_stringify(data);
  printf("%s\n", stringified);

  free(stringified);
  json_drop(data);
  return 0;
}
