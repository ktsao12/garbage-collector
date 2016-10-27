#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

struct memory_region{
  size_t * start;
  size_t * end;
};

struct memory_region global_mem;
struct memory_region heap_mem;
struct memory_region stack_mem;

void walk_region_and_mark(void* start, void* end);

//how many ptrs into the heap we have
#define INDEX_SIZE 1000
void* heapindex[INDEX_SIZE];


//grabbing the address and size of the global memory region from proc 
void init_global_range(){
  char file[100];
  char * line=NULL;
  size_t n=0;
  size_t read_bytes=0;
  size_t start, end;

  sprintf(file, "/proc/%d/maps", getpid());
  FILE * mapfile  = fopen(file, "r");
  if (mapfile==NULL){
    perror("opening maps file failed\n");
    exit(-1);
  }

  int counter=0;
  while ((read_bytes = getline(&line, &n, mapfile)) != -1) {
    if (strstr(line, "hw4")!=NULL){
      ++counter;
      if (counter==3){
        sscanf(line, "%lx-%lx", &start, &end);
        global_mem.start=(size_t*)start;
        // with a regular address space, our globals spill over into the heap
        global_mem.end=malloc(256);
        free(global_mem.end);
      }
    }
    else if (read_bytes > 0 && counter==3) {
      if(strstr(line,"heap")==NULL) {
        // with a randomized address space, our globals spill over into an unnamed segment directly following the globals
        sscanf(line, "%lx-%lx", &start, &end);
        printf("found an extra segment, ending at %zx\n",end);						
        global_mem.end=(size_t*)end;
      }
      break;
    }
  }
  fclose(mapfile);
}


//marking related operations

int is_marked(size_t* chunk) {
  return ((*chunk) & 0x2) > 0;
}

void mark(size_t* chunk) {
  (*chunk)|=0x2;
}

void clear_mark(size_t* chunk) {
  (*chunk)&=(~0x2);
}

// chunk related operations

#define chunk_size(c)  ((*((size_t*)c))& ~(size_t)3 ) 
void* next_chunk(void* c) { 
  if(chunk_size(c) == 0) {
    printf("Panic, chunk is of zero size.\n");
  }
  if((c+chunk_size(c)) < sbrk(0))
    return ((void*)c+chunk_size(c));
  else 
    return 0;
}
int in_use(void *c) { 
  return (next_chunk(c) && ((*(size_t*)next_chunk(c)) & 1));
}


// index related operations

#define IND_INTERVAL ((sbrk(0) - (void*)(heap_mem.start - 1)) / INDEX_SIZE)
void build_heap_index() {
  // TODO
}

// the actual collection code
size_t block_size (size_t *b) {
    size_t *chunk = b;
    size_t *header = chunk + 1;
    return (*header & (~7));
}

size_t *nextBlock(size_t *b) {
    size_t size = block_size(b);
    return (size_t *)((char *)b + size);
}

int blockAllocated(size_t *b) {
    size_t *next = nextBlock(b);
    if (nextBlock(b) == heap_mem.end) {
	return 1;
    }

    size_t *temp = next + 1;
    if ((*temp & 0x01) == 1) {
	return 1;
    }
    return 0;
}

int markBitSet(size_t *b) {
    size_t *header = b + 1;
    size_t mark = *header;
    size_t marked = mark & 2;
    return (marked && 1);
}

void setMarkBit(size_t *b) {
    size_t *ptr = b + 1;
    *(ptr) = *(ptr) | 4;
}

void resetMarkBit(size_t *b) {
    size_t *ptr = b + 1;
    *(ptr) = *(ptr) & (~4);
}

//determine if what "looks" like a pointer actually points to a block in the heap
size_t * is_pointer(size_t *ptr) {
    if (ptr == (size_t *)0x0){
	return NULL;
    }

    size_t *start = heap_mem.start - 2;

    if ((ptr < heap_mem.start) || (ptr > heap_mem.end)) {
	return NULL;
    }	

    while (start < (size_t *)sbrk(0)) {
	if ((ptr >= start) && (ptr <= nextBlock(start))) {
	    return start;
	}
	start = nextBlock(start);
    }
    return NULL;
}

void markBlock(size_t *ptr) {
    if ((ptr < heap_mem.start) || (ptr > heap_mem.end)) {
	return;
    }

    size_t *b;
    if ((b = is_pointer(ptr)) == NULL) {
	return;
    }

    if (markBitSet(b) == 1) {
	return;
    }

    setMarkBit(b);
    size_t length = (block_size(b) / (sizeof(size_t)));
    int i;

    for (i = 1; i < length; i++) {
	markBlock((size_t *)b[i]);
    }
    return;
}

void walk_region_and_mark(void* start, void* end) {
    size_t *ptr = start;
    while (ptr < (size_t *)end) {
	markBlock((size_t *)(*ptr));
	ptr++;
    }
}

void sweep() {
    size_t *start = heap_mem.start - 2;
    size_t *end = sbrk(0);
    size_t *ptr = start;
    size_t *temp;

    while (ptr < end) {
	if (markBitSet(ptr)) {
	    resetMarkBit(ptr);
	}
	else if (blockAllocated(ptr)) {
	    temp = (size_t *)(ptr + 2);
	    if (nextBlock(nextBlock(ptr)) >= end) {
		free(temp);
		heap_mem.end = sbrk(0);
		return;
	    }
	    else {
		free(temp);
		heap_mem.end = sbrk(0);
	    }
	}
	heap_mem.end = sbrk(0);
	ptr = nextBlock(ptr);
    }
}

// standard initialization 

void init_gc() {
  size_t stack_var;
  init_global_range();
  heap_mem.start=malloc(512);
  //since the heap grows down, the end is found first
  stack_mem.end=(size_t *)&stack_var;
}

void gc() {
  size_t stack_var;
  heap_mem.end=sbrk(0);
  //grows down, so start is a lower address
  stack_mem.start=(size_t *)&stack_var;

  // build the index that makes determining valid ptrs easier
  // implementing this smart function for collecting garbage can get bonus;
  // if you can't figure it out, just comment out this function.
  // walk_region_and_mark and sweep are enough for this project.
  build_heap_index();

  //walk memory regions
  walk_region_and_mark(global_mem.start,global_mem.end);
  walk_region_and_mark(stack_mem.start,stack_mem.end);
  sweep();
}
