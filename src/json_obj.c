json_t* json_init(uint type, json_v value) {
  json_t* obj = (json_t*)malloc(sizeof(json_t));
  obj->type = type;
  obj->value = value;
  return obj;
}

json_t* json_get_str(json_t* obj, char* str) {
  if(obj->type != JSON_MAP) {
    printf("JSONError: tried to use json_get_str on a non-map JSON object.\n");
    return 0;
  }
  json_t* key = json_init(JSON_STR, (json_v)str);
  hmap* hm = obj->value.map;
  json_t** pp = hmap_get(hm, &key);
  if(pp == 0) return 0;
  return *pp;
}

json_t* json_get_num(json_t* obj, double num) {
  if(obj->type != JSON_MAP && obj->type != JSON_ARR) {
    printf("JSONError: tried to use json_get_num on a non-map and non-array JSON object.\n");
    return 0;
  }
  if(obj->type == JSON_MAP) {
    json_t* key = json_init(JSON_NUM, (json_v)num);
    hmap* hm = obj->value.map;
    json_t** pp = hmap_get(hm, &key);
    if(pp == 0) return 0;
    return *pp;
  } else {
    dvec* vec = obj->value.arr;
    return *(json_t**)dvec_ptr(vec, (uint)num);
  }
}

json_t* json_drop(json_t* obj) {
  if(obj->type == JSON_STR) {
    free(obj->value.str);
  }
  if(obj->type == JSON_ARR) {
    dvec* vec = obj->value.arr;
    for(uint i = 0; i < vec->size; i++) {
      json_drop(*(json_t**)dvec_ptr(vec, i));
    }
    dvec_drop(vec);
  }
  if(obj->type == JSON_MAP) {
    hmap* hm = obj->value.map;
    hnode* nod = hmap_begin(hm);
    while(nod != 0) {
      json_drop(*(json_t**)(nod->key));
      json_drop(*(json_t**)(nod->val));
      nod = hmap_next(hm, nod);
    }
    hmap_drop(hm);
  }
  free(obj);
}

uint json_hash(const void* arg) {
  json_t* key = *(json_t**)arg;
  if(key->type == JSON_NUM) return *(uint*)&(key->value) % BUCKET_COUNT;
  if(key->type == JSON_STR) {
    uint H = 0;
    char* ss = key->value.str;
    for(uint i = 0; ss[i] != 0; i++) {
      H = (H * 1337 + ss[i]) % BUCKET_COUNT;
    }
    return H;
  }
  return 0;
}

bool json_cmp(const void* a1, const void* a2) {
  json_t* v1 = *(json_t**)a1;
  json_t* v2 = *(json_t**)a2;
  if(v1->type != v2->type) return false;
  if(v1->type == JSON_NUM) return v1->value.num == v2->value.num;
  if(v1->type == JSON_STR) return !strcmp(v1->value.str, v2->value.str);
  return false;
}
