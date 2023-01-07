typedef struct {
  uint size;
  uint asize;
  uint bsize;
  void* data;
} dvec;

dvec* dvec_init(uint bsize);
void dvec_push(dvec* vec, const void* val);
void* dvec_ptr(dvec* vec, uint i);
void dvec_pop(dvec* vec);
void* dvec_data(dvec* vec);
