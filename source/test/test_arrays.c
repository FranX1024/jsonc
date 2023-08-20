#include <stdlib.h>
#include <stdio.h>
#include "../jsonc.h"

int main() {
  JSON jarr = json_make_array(10);
  for(int i = 0; i < 10; i++)
    json_setf(jarr, i, json_number(0.5 * i));
  for(int i = 0; i < 10; i++)
    printf("%.6f\n", json_get_number(json_getf(jarr, i)));

  JSON jarr2 = json_make_array(0);
  for(int i = 0; i < 21; i++)
    json_push(jarr2, json_number(0.5 * i));
  for(int i = 0; i < 21; i++)
    printf("%.6f\n", json_get_number(json_getf(jarr2, i)));
  return 0;
}
