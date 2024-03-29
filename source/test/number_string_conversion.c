#include <stdio.h>
#include "../utils.h"

int main() {
  double numbers[] = {
    100,
    12,
    0.001,
    1e-9,
    123e100,
    3950.1024,
    -15.3435,
    -2e-9,
    123456789012345,
    1234567890123456
  };
  for(int i = 0; i < (sizeof(numbers) >> 3); i++) {
    char* nstr = json_number_to_string(numbers[i]);
    printf("%s\n", nstr);
    free(nstr);
  }
  return 0;
}
