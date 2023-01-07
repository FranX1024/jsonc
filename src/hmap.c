hmap* hmap_init(uint keysize, uint valsize, hm_comparator cmp, hm_hasher hsh) {
  hmap* hm = (hmap*)malloc(sizeof(hmap));
  hm->keysize = keysize;
  hm->valsize = valsize;
  hm->comparator = cmp;
  hm->hasher = hsh;
  return hm;
}

void hmap_set(hmap* hm, const void* key, const void* val) {
  uint H = hm->hasher(key);
  /* check if it already exists and if so set value of that */
  hnode* nod1 = hm->buckets[H];
  while(nod1 != 0) {
    if(hm->comparator(nod1->key, key)) {
      memcpy(nod1->val, val, hm->valsize);
      return;
    }
    nod1 = nod1->next;
  }
  /* if not, create new entry */
  hnode* nod = (hnode*)malloc(sizeof(hnode));
  nod->key = malloc(sizeof(key));
  nod->val = malloc(sizeof(val));
  memcpy(nod->key, key, hm->keysize);
  memcpy(nod->val, val, hm->valsize);
  nod->hash = H;
  nod->next = 0;
  if(hm->buckets[H] != 0) {
    nod->next = hm->buckets[H];
  }
  hm->buckets[H] = nod;
}

void* hmap_get(hmap* hm, const void* key) {
  uint H = hm->hasher(key);
  hnode* nod = hm->buckets[H];
  while(nod != 0) {
    if(hm->comparator(nod->key, key)) return nod->val;
    nod = nod->next;
  }
  return 0;
}

hnode* hmap_begin(hmap* hm) {
  return hm->buckets[0];
}

hnode* hmap_next(hmap* hm, hnode* nod) {
  if(nod == 0) return 0;
  if(nod->next) return nod->next;
  if(nod->hash == BUCKET_COUNT) return 0;
  return hm->buckets[nod->hash + 1];
}

void hmap_drop(hmap* hm) {
  hnode* nod = hmap_begin(hm);
  while(nod != 0) {
    hnode* nod2 = hmap_next(hm, nod);
    free(nod->key);
    free(nod->val);
    free(nod);
    nod = nod2;
  }
  free(hm);
}
