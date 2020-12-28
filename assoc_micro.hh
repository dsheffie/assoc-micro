#ifndef __assoc_micro__
#define __assoc_micro__

#include <cstdint>

static const uint64_t ptr_key = 0x1234567076543210UL;

template<int size>
struct node {
  node *next;
  uint8_t pad[size - sizeof(void*)];
};

static const void* failed_mmap = reinterpret_cast<void *>(-1);

template <bool enable, int size>
static inline node<size>* xor_ptr(node<size> *ptr) {
  if(enable) {
    uint64_t p = reinterpret_cast<uint64_t>(ptr);
    p ^= ptr_key;
    return reinterpret_cast<node<size>*>(p);
  }
  else {
    return ptr;
  }
}


template <bool enable, int size>
static inline volatile node<size>* xor_ptr(volatile node<size> *ptr) {
  if(enable) {
    uint64_t p = reinterpret_cast<uint64_t>(ptr);
    p ^= ptr_key;
    return reinterpret_cast<volatile node<size>*>(p);
  }
  else {
    return ptr;
  }
}

template <int size, bool en_xor>
volatile node<size> *traverse(volatile node<size> *n, uint64_t iters) {
  static_assert(sizeof(node<size>) == size, "sizeof(node<size>)doesn't match expectations!");
  while(iters) {
    n = xor_ptr<en_xor, size>(n->next);
    n = xor_ptr<en_xor, size>(n->next);
    n = xor_ptr<en_xor, size>(n->next);
    n = xor_ptr<en_xor, size>(n->next);    
    iters -= 4;
  }
  return n;
}

#endif
