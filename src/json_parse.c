dvec *json_tokenize(const char* raw) {
  dvec* tokens = dvec_init(sizeof(token_t));
  dvec* ctoken = dvec_init(sizeof(char));
  uint ctype;
  bool quoted = false;
  bool escaped = false;
  for(uint i = 0; raw[i] != 0; i++) {
    if(quoted) {
      // escaping using backslash
      if(raw[i] == '\\')
	escaped = !escaped;
      // ...
      if(raw[i] == '"' && !escaped) {
	quoted = false;
      }
      dvec_push(ctoken, raw + i);
      // chars after this one no longer escaped
      if(escaped && raw[i] != '\\')
	escaped = false;
    } else {
      if(raw[i] == '\n' || raw[i] == ' ' || raw[i] == '\t' || raw[i] == 13 || raw[i] == 10)
	continue;
      // check if new ctoken is necessary
      if((raw[i] == '"' ||
	 raw[i] == '{' || raw[i] == '}' ||
	 raw[i] == '[' || raw[i] == ']' ||
	 raw[i] == ',' || raw[i] == ':' ||
	  (raw[i] >= '0' && raw[i] <= '9' || raw[i] == '-' || raw[i] == '.') &&
	  !(i>0 && (raw[i-1] >= '0' && raw[i-1] <= '9' || raw[i-1] == '-' || raw[i-1] == '.'))) &&
	 ctoken->size > 0)
	{
	  char zerochar = 0;
	  dvec_push(ctoken, &zerochar);
	  char* ctokenstr = dvec_data(ctoken);
	  token_t token;
	  token.value = ctokenstr;
	  token.type = ctype;
	  dvec_push(tokens, &token);
	  dvec_drop(ctoken);
	  ctoken = dvec_init(sizeof(char));
	}
      if(raw[i] == '"') {
	ctype = JSON_STR;
	quoted = true;
      } else
      if(raw[i] >= '0' && raw[i] <= '9' || raw[i] == '-' || raw[i] == '.')
	ctype = JSON_NUM;
      else ctype = JSON_SYM;
      dvec_push(ctoken, raw + i);
    }
  }
  char zerochar = 0;
  dvec_push(ctoken, &zerochar);
  char* ctokenstr = dvec_data(ctoken);
  token_t token;
  token.value = ctokenstr;
  token.type = ctype;
  dvec_push(tokens, &token);
  dvec_drop(ctoken);
  return tokens;
}

json_t* json_parse_string(const char* mystr) {
  dvec* vstr = dvec_init(sizeof(char));
  uint len = strlen(mystr);
  for(uint i = 1; i < len - 1; i++) {
    if(mystr[i] == '\\' && i < len - 2) {
      char z = mystr[i + 1];
      if(z == 'n') {
	z = '\n';
      }
      if(z == 't') {
	z = '\t';
      }
      dvec_push(vstr, &z);
      i++;
    } else {
      dvec_push(vstr, mystr + i);
    }
  }
  char zerr = 0;
  dvec_push(vstr, &zerr);
  char* cstr = dvec_data(vstr);
  dvec_drop(vstr);
  return json_init(JSON_STR, (json_v)cstr);
}

json_t* json_parse_number(const char* mystr) {
  double mynum = 0;
  double decord = .1;
  char sig = '+';
  char state = 0; // 0 = first char, 1 = integer part, 2 = decimal part
  uint len = strlen(mystr);
  for(uint i = 0; i < len; i++) {
    switch(state) {
    case 0:
      if(mystr[i] == '-') {
	sig = '-';
	state = 1;
	break;
      }
    case 1:
      if(mystr[i] == '.') {
	state = 2;
      } else if(mystr[i] >= '0' && mystr[i] <= '9') {
	mynum = mynum * 10 + (mystr[i] - '0');
      } else {
	printf("JSON Parsing Error: Unexpected character '%c' in a number!\n", mystr[i]);
      }
      break;
    case 2:
      if(mystr[i] >= '0' && mystr[i] <= '9') {
	mynum = mynum + (mystr[i] - '0') * decord;
	decord *= .1;
      } else {
	printf("JSON Parsing Error: Unexpected character '%c' in a number!\n", mystr[i]);
      }
      break;
    }
  }
  if(sig == '-') mynum *= -1;
  return json_init(JSON_NUM, (json_v)mynum);
}

json_t* json_parse_token_list(token_t* tokens, uint size, uint* ind) {
  if(size <= (*ind)) {
    printf("JSON Parsing Error: Expected a value!\n");
    return 0;
  }
  if(tokens[*ind].type == JSON_NUM) {
    return json_parse_number(tokens[*ind].value);
    
  } else if(tokens[*ind].type == JSON_STR) {
    return json_parse_string(tokens[*ind].value);
    
  } else if(tokens[*ind].value[0] == '[') {
    dvec* jarr = dvec_init(sizeof(json_t*));
    json_t* obj = json_init(JSON_ARR, (json_v)jarr);
    // ...
    char state = 0; // 0 = expecting value, 1 = expecting ","
    (*ind)++;
    while(*ind < size && tokens[*ind].value[0] != ']') {
      if(state == 0) {
	json_t* obj_ptr = json_parse_token_list(tokens, size, ind);
	dvec_push(jarr, &obj_ptr);
      } else {
	if(tokens[*ind].value[0] != ',')
	  printf("JSON Parsing Error: Expected \",\" got \"%s\"!\n", tokens[*ind].value);
      }
      (*ind)++;
      state = !state;
    }
    return obj;
    
  } else if(tokens[*ind].value[0] == '{') {
    hmap* jmap = hmap_init(sizeof(json_t*), sizeof(json_t*), json_cmp, json_hash);
    json_t* obj = json_init(JSON_MAP, (json_v)jmap);
    // ...
    char state = 0; // 0 = expecting key, 1 = expecting ":", 2 = expecting value, 3 = expecting ","
    (*ind)++;
    json_t* key = 0;
    json_t* val = 0;
    while(*ind < size && tokens[*ind].value[0] != '}') {
      //printf("ind = %d, state = %d\n", *ind, state);
      switch(state) {
      case 0:
	key = json_parse_token_list(tokens, size, ind);
	break;
      case 1:
	if(tokens[*ind].value[0] != ':')
	  printf("JSON Parsing Error: Expected \":\" got \"%s\"!\n", tokens[*ind].value);
	break;
      case 2:
	val = json_parse_token_list(tokens, size, ind);
	hmap_set(jmap, &key, &val);
	break;
      case 3:
	if(tokens[*ind].value[0] != ',')
	  printf("JSON Parsing Error: Expected \",\" got \"%s\"!\n", tokens[*ind].value);
	break;
      }
      (*ind)++;
      state = (state + 1) & 3;
    }
    return obj;
    
  } else {
    printf("JSON Parsing Error: Unexpected symbol \"%s\"!\n", tokens[*ind].value);
  }
}

json_t* json_parse(const char* rawstr) {
  dvec* tokenvec = json_tokenize(rawstr);
  token_t* tokens = (token_t*)(tokenvec->data);
  uint ind = 0;
  json_t* obj = json_parse_token_list(tokens, tokenvec->size, &ind);
  for(uint i = 0; i < tokenvec->size; i++) {
    free(((token_t*)dvec_ptr(tokenvec, i))->value);
  }
  dvec_drop(tokenvec);
  return obj;
}
