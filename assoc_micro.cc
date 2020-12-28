#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <fstream>

#include "assoc_micro.hh"
#include "perf.hh"

#define PROT (PROT_READ | PROT_WRITE)
#define MAP (MAP_ANONYMOUS|MAP_PRIVATE|MAP_POPULATE)

template <typename T>
void swap(T &x, T &y) {
  T t = x;
  x = y; y = t;
}

template <typename T>
void shuffle(std::vector<T> &vec, size_t len) {
  for(size_t i = 0; i < len; i++) {
    size_t j = i + (rand() % (len - i));
    swap(vec[i], vec[j]);
  }
}

template <int size,bool en_xor,int tries=16>
void run(void *ptr, size_t n_keys, std::ofstream &out) {
  std::vector<uint64_t> keys(n_keys);
  std::vector<double> cycle_results(tries),time_results(tries);
  static const size_t iters = (1UL<<22);
  node<size> *nodes = reinterpret_cast<node<size>*>(ptr);

  for(int try_ = 0; try_ < tries; try_++) {
    cycle_counter cc;
    for(uint64_t i = 0; i < n_keys; i++) {
      keys[i] = i;
    }
    shuffle(keys, n_keys);
    node<size> *h = &nodes[keys[0]];
    node<size> *c = h;  
    h->next = h;
    for(uint64_t i = 1; i < n_keys; i++) {
      node<size> *n = &nodes[keys[i]];
      node<size> *t = c->next;
      c->next = n;
      n->next = t;
      c = n;
    }
    if(en_xor) {
      for(uint64_t i = 0; i < n_keys; i++) {
	nodes[i].next = xor_ptr<true>(nodes[i].next);
      }
    }
    cc.reset_counter();    
    auto start = std::chrono::high_resolution_clock::now();
    cc.enable_counter();
    auto c_start = cc.read_counter();
    traverse<size,en_xor>(h, iters);
    auto c_stop = cc.read_counter();    
    auto stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = stop-start;
    double t = elapsed.count() / (1e-9);
    double c_t = static_cast<double>(c_stop-c_start);        
    t /= iters;
    c_t /= iters;
    cycle_results[try_] = c_t;
    time_results[try_] = t;
  }
  std::sort(cycle_results.begin(), cycle_results.end());
  std::sort(time_results.begin(), time_results.end());
  double t = time_results[tries/2];
  double c_t = cycle_results[tries/2];
  std::cout << n_keys << "," << c_t <<" cycles," <<t << " ns \n";
  out << n_keys << "," << c_t <<"," << t << "\n";
  out.flush();  
}


int main(int argc, char *argv[]) {
  int c;
  void *ptr = nullptr;
  bool xor_pointers = false;
  const size_t max_mem = 1UL<<26;

  while ((c = getopt (argc, argv, "x:")) != -1) {
    switch(c)
      {
      case 'x':
	xor_pointers = (atoi(optarg) != 0);
	break;
      default:
	break;
      }
  }
  ptr = mmap(nullptr, max_mem, PROT, MAP|MAP_HUGETLB, -1, 0);

  if(ptr == failed_mmap) {
    ptr = mmap(nullptr, max_mem, PROT, MAP, -1, 0);
    if(ptr == failed_mmap) {
      std::cout << "unable to mmap memory\n";
      return -1;
    }
    else {
      std::cout << "unable to allocate memory with hugetlb\n";
    }
  }
  else {
    std::cout << "allocated nodes with hugetlb\n";
  }
  std::ofstream out("cpu.csv");

  static const int stride = 1<<20;
  
  for(int i = 1; i < 32; i++) {
    if(xor_pointers) {
      run<stride,true>(ptr, i, out);
    }
    else {
      run<stride,false>(ptr, i, out);
    }
  }
  
  
  out.close();
  munmap(ptr, max_mem);  
  return 0;
}
