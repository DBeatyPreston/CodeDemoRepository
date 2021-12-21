//Dennis Preston Beaty
//COSC 360 Lab 6
//10/22/2021
// C

#include "mymalloc.h"
#include <stdio.h>
#include <unistd.h>

typedef struct block{
  unsigned int size;
  struct block* flink;
} *Block;

Block block_begin= NULL;

//push a node to the head of the list
void block_insert(Block node){
  //if empty set beginning to node
  if (block_begin==NULL) {
    block_begin= node;
    return;
  }
  Block temp;
  for (temp= free_list_begin(); temp->flink!= NULL && node> temp->flink; temp= temp->flink);
  //if at the end add node to the end
  if (temp->flink== NULL) {
    /* code */
    temp->flink= node;
    return;
  }
  //place node in place of temp
  node->flink= temp->flink;
  temp->flink= node;
}

//My_malloc() is a buffered interface to sbrk(), doling out heap memory for programs that call it.
void* my_malloc(size_t size) {
    // Your code here.
    Block allocate;
    size_t allocate_size= (size+7+8)&-8;
    //look for node
    for (allocate= free_list_begin();allocate!=NULL;allocate= free_list_next(allocate)) {
      /* code */
      if (allocate->size>= allocate_size) {
        /* code */
        break;
      }
    }
    //if node not found create a node
    if (allocate==NULL) {
      /* code */
      size_t min_val= 8192;
      if (size > 8184) {
        /* code */
        min_val= (size+7+8)&-8;
      }
      //create node for return
      Block node_create= (Block) sbrk(min_val);
      node_create->size= min_val;
      node_create->flink= NULL;
      //add note_create list
      block_insert(node_create);
      allocate= node_create;
    }
    //size of new node
    allocate->size-= allocate_size;
    Block allocate_2= (void*)allocate+allocate->size;
    //see if there is room for a second allocation
    if (allocate->size<=16) {
      /* code */
      allocate_size+=allocate->size;
      if (allocate== block_begin) {
        /* code */
        block_begin= allocate->flink;
      }
      else{
        Block b_link= block_begin;
        while (b_link->flink!= allocate) {
          /* code */
          b_link= b_link->flink;
        }
        b_link->flink= allocate->flink;
        allocate= allocate->flink;
      }
    }
    //set new size
    allocate_2->size= allocate_size;
    return (void*)allocate_2+8;
}

//my_free(p), the chunk of memory should be returned to the free list
void my_free(void* ptr) {
    // Your code here.
    block_insert((Block)(ptr-8));
}

//returns a pointer to the first node on the free list. If the free list is empty, it should return NULL. The first four bytes on a free list node should contain the size of the node (including all metadata, like the size and pointers).
void* free_list_begin() {
    // Your code here.
    return block_begin;
}

//takes a pointer to a free list node and returns a pointer to the next node on the free list. If n is the last node on the free list, then free_list_next(n) should return NULL.
void* free_list_next(void* node) {
    // Your code here.
    return ((Block) node)->flink;
}

//coalesce_free_list() should process the free list and combine all adjacent entries.
void coalesce_free_list() {
    // Your code here.
    Block temp= free_list_begin();
    while (temp!= NULL) {
      /* code */
      Block temp_next= temp->flink;
      //if node and node after are adjacent combine the two nodes
      if ((void*)temp+ temp->size== temp_next) {
        /* code */
        temp->size+= temp_next->size;
        if (temp_next== block_begin) {
          /* code */
          block_begin= temp_next->flink;
        }
        else{
          Block b_link= block_begin;
          while (b_link->flink!= temp_next) {
            /* code */
            b_link= b_link->flink;
          }
          b_link->flink= temp_next->flink;
          temp_next= temp_next->flink;
        }
      }
      else{
        temp= temp->flink;
      }
    }
}
