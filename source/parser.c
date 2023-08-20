#include <stdlib.h>
#include "jsonc.h"
#include "utils.h"

char* json_escape_string(const char* string) {
  int length = 0;
  for(int i = 0; string[i] != 0; i++) {
    if(string[i] == '\n' || string[i] == '"')
      length++;
    length++;
  }
  char* escaped = (char*)malloc(length + 3);
  escaped[0] = '"';
  escaped[length + 1] = '"';
  escaped[length + 2] = 0;
  int ind = 1;
  for(int i = 0; string[i] != 0; i++) {
    switch(string[i]) {
    case '\n':
      escaped[ind] = '\\';
      escaped[ind+1] = 'n';
      ind += 2;
      break;
    case '"':
      escaped[ind] = '\\';
      escaped[ind+1] = '"';
      ind += 2;
      break;
    default:
      escaped[ind] = string[i];
      ind++;
    }
  }
  return escaped;
}

char* json_stringify(JSON data) {

  json_StringBuilder* strb;

  switch(data.type) {
    
  case JSON_NUMBER:
    return json_number_to_string(json_get_number(data));
    
  case JSON_STRING:
    return json_escape_string(json_get_string(data));

  case JSON_ARRAY:
    strb = json_create_strb();
    json_append_strb(strb, "[");
    for(int i = 0; i < json_length(data); i++) {
      if(i != 0) json_append_strb(strb, ",");
      json_append_strb(strb, json_stringify(json_getf(data, i)));
    }
    json_append_strb(strb, "]");
    return json_finish_strb(strb);

  case JSON_TABLE:
    strb = json_create_strb();
    json_append_strb(strb, "{");
    bool first_entry = true;
    for(JSON_TABLE_iterator* it = json_table_iterator(data); it != 0; it = json_next(it)) {
      if(first_entry) {
	first_entry = false;
      } else {
	json_append_strb(strb, ",");
      }
      json_append_strb(strb, json_escape_string(it->entry->key));
      json_append_strb(strb, ":");
      json_append_strb(strb, json_stringify(it->entry->value));
    }
    json_append_strb(strb, "}");
    return json_finish_strb(strb);

    /* const char*s are copied so that
       users may always free the returned string
       after usage
    */
    
  case JSON_NULL:
    return json_copy_cstring("null");

  case JSON_BOOL:
    if(data.data == 0)
      return json_copy_cstring("false");
    else
      return json_copy_cstring("true");
  }
}

#define inc_macro(ind) (*ind)++;if((*ind)>=size){json_drop(data);return JSON_NIL;}

JSON json_recparse(U4 size, const JSON* tokens, U4 *ind) {
  char* first_token = (char*)tokens[*ind].data;
  JSON data;
  
  if(first_token[0] == '{') { 
    data = json_make_table();
    inc_macro(ind);
    while(((char*)tokens[*ind].data)[0] != '}') {
      JSON jkey = json_recparse(size, tokens, ind);
      inc_macro(ind);
      
      if(jkey.type != JSON_STRING) {
	json_drop(data);
	return JSON_NIL;
      }
      
      if(((char*)tokens[*ind].data)[0] != ':') {
	json_drop(data);
	return JSON_NIL;
      }
      inc_macro(ind);
	
      JSON jval = json_recparse(size, tokens, ind);
      inc_macro(ind);

      json_seth(data, (char*)jkey.data, jval);
      
      if(((char*)tokens[*ind].data)[0] == ',') {
        inc_macro(ind);
      }
    }
    return data;
    
  } else if(first_token[0] == '[') {
    data = json_make_array(0);
    inc_macro(ind);
    while(((char*)tokens[*ind].data)[0] != ']') {
	
      JSON jval = json_recparse(size, tokens, ind);
      inc_macro(ind);

      json_push(data, jval);
      
      if(((char*)tokens[*ind].data)[0] == ',') {
        inc_macro(ind);
      }
    }
    return data;
  }

  // all remaining options are limited to exactly one token
  // so ind is not increased at all
  
  if(first_token[0] == '"') {
    // unescape string
    U4 length = 0;
    for(U4 i = 1; first_token[i] != '"'; i++) {
      if(first_token[i] != '\\') length++;
    }
    char* unstr = (char*)malloc(length + 1);
    unstr[length] = 0;
    U4 pos = 0;
    for(U4 i = 1; first_token[i] != '"'; i++) {
      if(first_token[i] == '\\') {
	switch(first_token[i+1]) {
	case 'n':
	  unstr[pos++] = '\n';
	  break;
	case 't':
	  unstr[pos++] = '\t';
	  break;
	default:
	  unstr[pos++] = first_token[i+1];
	}
	// skip over next one as it was already handled
	i++;
      } else {
	unstr[pos++] = first_token[i];
      }
    }
    data = json_string(unstr);
    free(unstr);
    return data;
    
  } else if(first_token[0] >= '0' && first_token[0] <= '9' ||
	    first_token[0] == '.' || first_token[0] == '-') {
    // string to double
    return json_number(json_string_to_number(first_token));
    
  } else if(!strcmp(first_token, "true")) {
    return JSON_TRUE;
    
  } else if(!strcmp(first_token, "false")) {
    return JSON_FALSE;

  } else if(!strcmp(first_token, "null")) {
    return JSON_NIL;

  }

  return JSON_NIL;
}

JSON json_parse(const char* raw) {

  // tokenizer
  
  JSON tokens = json_make_array(0);
  json_StringBuilder* strb = json_create_strb();
  bool quoted = false;
  for(U4 i = 0; raw[i] != 0; i++) {
    char current_char[2];
    current_char[0] = raw[i];
    current_char[1] = 0;
    if(quoted) {
      json_append_strb(strb, current_char);
      if(raw[i] == '"' && raw[i-1] != '\\') {
	quoted = false;
	char* str = json_finish_strb(strb);
	strb = json_create_strb();
	json_push(tokens, json_string(str));
	free(str);
      }
      continue;
    }
    switch(raw[i]) {
    case '[': case ']': case '{': case '}':
    case ',': case ':':
      if(strb->size) {
	char* str = json_finish_strb(strb);
	strb = json_create_strb();
	json_push(tokens, json_string(str));
	free(str);
      }
      json_push(tokens, json_string(current_char));
      break;

    case '"':
      if(strb->size) {
	char* str = json_finish_strb(strb);
	strb = json_create_strb();
	json_push(tokens, json_string(str));
	free(str);
      }
      json_append_strb(strb, current_char);
      quoted = true;
      break;

    case ' ': case '\n': case '\t':
      if(strb->size) {
	char* str = json_finish_strb(strb);
	strb = json_create_strb();
	json_push(tokens, json_string(str));
	free(str);
      }
      break;

    default:
      json_append_strb(strb, current_char);
    }
  }  
  if(strb->size) {
    char* str = json_finish_strb(strb);
    json_push(tokens, json_string(str));
    free(str);
  } else free(json_finish_strb(strb));

  U4 token_pos = 0;
  JSON data = json_recparse(json_length(tokens), ((JSON_ARRAY_t*)tokens.data)->data, &token_pos);
  json_drop(tokens);
  return data;
}
