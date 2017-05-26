#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"

#define MAXLINE 256
extern int memsize;

extern int debug;

extern struct frame *coremap;

extern char * tracefile;

typedef struct tr{
   // Virtual address  
   addr_t vaddr;
   
   struct tr* next;

}trace;

static trace * head;
static trace * tail;

//helper to get num of instructions till next use
int till_next(pgtbl_entry_t *p){
	int i = 0 ;
	trace *curr = head;
	//searches through linked list from current point
   while(curr->next != NULL){
   	   i++;
   	   //compares adresses
   		if ( p->vaddr == curr->vaddr){
             return i;  		
   		}
   		curr = curr->next;
   		
   }
   return -1;
}


/* Page to evict is chosen using the optimal (aka MIN) algorithm. 
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	int max = -1;
	int index = 0;
	int i;
	// returns index of pte with smallest counter 
	for( i = 0; i < memsize; i++){
		pgtbl_entry_t *p = coremap[i].pte;
		//find how long till next use
		int curr = till_next(p);
      
      if (curr < 0){
      	// not used again 
         return i;
               
      }				
		if(curr > max){
			// time till next use longer than current max
        	max = curr;
        	index = i;     
      }
     
	}
	return index;
}

/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
   // save currennt trace	
	trace *curr = head;
	// move instructions foward
	head = head->next;
	//free previous trace
	free(curr);		
	return;
}


/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	
	FILE *file;
	if ((file = fopen(tracefile, "r")) == NULL){
		perror("Error Opening tracefile");
		exit(1);	
	}
	// declare variables needed for implementation
	char buf[MAXLINE];
	addr_t vaddr;
	char type;
	// initiaialize trace structs
   head  = NULL;   
   tail = NULL;
   // read file to get all instructions saved in linked list
   while(fgets(buf,MAXLINE,file) != NULL){
   	if( buf[0] != '='){
   			sscanf(buf, "%c %lx", &type, &vaddr);
   			trace* new_node = malloc(sizeof(trace));
   			new_node->vaddr = (vaddr >> PAGE_SHIFT) << PAGE_SHIFT;
   			new_node->next = NULL;
   			// head is empty
   			if( head == NULL){ 
   				head = new_node;
   				tail = new_node;
   			}else{
               tail->next = new_node;
               tail = new_node;    			
   			} 		
   	}
   }	
   fclose(file);
   	
}
        
   
