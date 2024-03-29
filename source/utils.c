#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

char* json_copy_cstring(const char* string) {
  char* copied;
  U32 length = strlen(string);
  copied = (char*)malloc(length + 1);
  memcpy(copied, string, length + 1);
  return copied;
}

U32 json_hash_cstring(const char* string) {
  U32 hash = 5381;
  for(U32 i = 0; string[i] != 0; i++) {
    hash = (hash << 5) + hash + string[i];
  }
  return hash;
}

/* StringBuilder is equivalent of JSON
   array implementation, but with char type instead.
   It's intended to be used by the json_stringify
   function to avoid having to determine the size of
   output before computing the output itself
*/

json_StringBuilder* json_create_strb() {
  U32 real_size = 8;
  json_StringBuilder* strb = (json_StringBuilder*)malloc(sizeof(json_StringBuilder));
  strb->string = (char*)malloc(real_size);
  strb->real_size = real_size;
  strb->size = 0;
  return strb;
}

void json_append_strb(json_StringBuilder* strb, const char* cstring) {
  U32 append_length = 0;
  while(cstring[append_length]) append_length++;
  
  U32 new_size = strb->size + append_length;

  // new_size + 1 so that memory is left for null terminator
  
  if(new_size + 1> strb->real_size) {
    while(new_size + 1 > strb->real_size) strb->real_size <<= 1;
    strb->string = (char*)realloc(strb->string, strb->real_size);
  }

  // append null terminator too
  
  for(U32 i = 0; i <= append_length; i++) {
    strb->string[strb->size + i] = cstring[i];
  }
  strb->size = new_size;
}

char* json_finish_strb(json_StringBuilder* strb) {
  char* string = strb->string;
  free(strb);
  return string;
}

double json_string_to_number(const char* string) {
  int state = 0;
  /* 0 = - or whole
     1 = whole
     2 = decimal
     3 = exponent sign
     4 = exponent
   */
  double result = 0;
  double decimal_order = 1;
  char sign = '+';
  int exponent = 0;
  char exponent_sign = '+';
  for(U32 i = 0; string[i] != 0; i++) {
    switch(state) {
    case 0:
      state = 1;
      if(string[i] == '-') {
	sign = '-';
	break;
      }
      // otherwise, jump straight to state 1
      
    case 1: // whole
      if(string[i] >= '0' && string[i] <= '9') {
	result = result * 10 + string[i] - '0';
	
      } else if(string[i] == '.') {
	state = 2;

      } else if(string[i] == 'e' || string[i] == 'E') {
	state = 3;
	  
      } else return F64NAN;
      break;

    case 2: // decimal
      if(string[i] >= '0' && string[i] <= '9') {
	decimal_order *= 0.1;
	result += decimal_order * (string[i] - '0');
	
      } else if(string[i] == 'e' || string[i] == 'E') {
	state = 3;
	
      } else return F64NAN;
      break;

    case 3: // exponent sign
      state = 4;
      if(string[i] == '+') break;
      if(string[i] == '-') {
	exponent_sign = '-';
	break;
      }
      
    case 4: // exponent
      if(string[i] >= '0' && string[i] <= '9') {
	exponent = exponent * 10 + string[i] - '0';
	
      } else return F64NAN;
    }
  }
  double factor = 1;
  double exp_base = 10;
  if(exponent_sign == '-') exp_base = 0.1;
  
  for(U32 i = 31; i < ~0; i--) {
    factor *= factor;
    if((exponent >> i) & 1) factor *= exp_base;
  }
  result *= factor;
  if(sign == '-') result = -result;
  return result;
}

char* json_number_to_string(double number) {
  
  // handle infinity/NaN separately
  if((((*(U64*)&number) >> 52) & 2047) == 2047) {
    if((*(U64*)&number) >> 63)
      return json_copy_cstring("Infinity");
    else
      return json_copy_cstring("-Infinity");
  }
  
  char sign = '+';
  
  if(number < 0) {
    sign = '-';
    number = -number;
  } 
  int exponent = 0; // exponent base 10
  double shifted_number = number;

  /* max double exponent base 2 is
     1024. log10(2^1024) = ln(2^1024)/ln(10) =
     = 1024 * ln(2)/ln(10) =~ 308.25
  */
  static double exp10s[9] = {1e1, 1e2, 1e4, 1e8, 1e16, 1e32, 1e64, 1e128, 1e256};
  static double exp01s[9] = {1e-1, 1e-2, 1e-4, 1e-8, 1e-16, 1e-32, 1e-64, 1e-128, 1e-256};

  if(number < 1) {
    for(int i = 8; i >= 0; i--) {
      if(exp10s[i] * shifted_number < 1) {
	shifted_number *= exp10s[i];
	exponent -= 1 << i;
      }
    }
    shifted_number *= 10;
    exponent--;
  } else if(number >= 10) {
    for(int i = 8; i >= 0; i--) {
      if(exp01s[i] * shifted_number >= 10) {
	shifted_number *= exp01s[i];
	exponent += 1 << i;
      }
    }
    shifted_number *= 0.1;
    exponent++;
  }

  int zeropad = 0; // 1e-2 -> 0.01, zeropad = 2
  int dpointpos = 0; // after which index
  U64 digits_u8 = (U64)(1e14 * shifted_number + 0.5); // 0.5 is for rounding
  
  if(exponent > 0 && exponent < 15) {
    dpointpos = exponent;
    exponent = 0;
  }
  if(exponent < 0 && exponent > -5) {
    zeropad = -exponent;
    exponent = 0;
  }
  char* numstring = (char*)malloc(30);
  char* digitstring = (char*)malloc(16);
  sprintf(digitstring, "%llu", digits_u8);

  int ind = 0;
  if(sign == '-') {
    numstring[ind] = '-';
    ind++;
    dpointpos++;
  }
  for(int i = 0; i < zeropad; i++) {
    numstring[ind] = '0';
    if(ind == dpointpos) {
      numstring[ind+1] = '.';
      ind++;
    }
    ind++;
  }
  for(int i = 0; i < 15; i++) {
    numstring[ind] = digitstring[i];
    if(ind == dpointpos) {
      numstring[ind+1] = '.';
      ind++;
    }
    ind++;
  }
  // get rid of unnecessary 0s and .
  while(ind > 1 && (numstring[ind-1] == '0' || numstring[ind-1] == '.')) {
    ind--;
    if(numstring[ind] == '.') break;
  }

  // exponent if necessary
  if(exponent != 0) {
    numstring[ind] = 'e';
    ind++;
    char* expstring = (char*)malloc(5); // sign + 3 digits + terminator
    sprintf(expstring, "%d", exponent);
    for(int i = 0; expstring[i] != 0; i++) {
      numstring[ind] = expstring[i];
      ind++;
    }
    free(expstring);
  }
  
  // terminator
  numstring[ind] = 0;

  free(digitstring);
  return numstring;
}
