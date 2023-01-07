#define BUCKET_COUNT 1000

typedef bool (*hm_comparator)(const void*, const void*);
typedef uint (*hm_hasher)(const void*);

typedef struct hnode {
  void* key;
  void* val;
  uint hash; // for itterating
  struct hnode* next;
} hnode;

typedef struct {
  uint keysize;
  uint valsize;
  hm_comparator comparator;
  hm_hasher hasher;
  hnode *buckets[BUCKET_COUNT];
} hmap;

hmap* hmap_init(uint keysize, uint valsize, hm_comparator cmp, hm_hasher hsh);
void  hmap_set(hmap* hm, const void* key, const void* val);
void* hmap_get(hmap* hm, const void* key);
void  hmap_drop(hmap* hm);

hnode* hmap_begin(hmap* hm);
hnode* hmap_next(hmap* hm, hnode* nod);
