dvec* dvec_init(uint bsize) {
  dvec* vec = (dvec*)malloc(sizeof(dvec));
  vec->size = 0;
  vec->asize = 1;
  vec->bsize = bsize;
  vec->data = malloc(bsize);
  return vec;
}

void dvec_push(dvec* vec, const void* val) {
  if(vec->size == vec->asize) {
    vec->data = realloc(vec->data, (vec->asize << 1)*(vec->bsize));
    vec->asize <<= 1;
  }
  memcpy(vec->data + (vec->size)*(vec->bsize), val, vec->bsize);
  vec->size += 1;
}

void* dvec_ptr(dvec* vec, uint i) {
  return vec->data + i * (vec->bsize);
}

void dvec_pop(dvec* vec) {
  vec->size--;
}

void dvec_drop(dvec* vec) {
  free(vec->data);
  free(vec);
}

void* dvec_data(dvec* vec) {
  void* ptr = malloc(vec->size * vec->bsize);
  memcpy(ptr, vec->data, vec->size * vec->bsize);
  return ptr;
}
