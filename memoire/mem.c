#include "mem.h"
#include <stdio.h>
#include <commone.h>

struct fb {
	size_t size;
	struct fb *next;
};

// Head of free block chained list
struct fb *head;

// Current selected free block searching function
mem_fit_function_t *searchFunction;

size_t sizeOfMem;

void mem_init(char* mem, size_t size) {

	sizeOfMem = size;
	
	// Create the head of chained list at 0x0
	head = (struct fb *) mem;
	// We need to keep some space for the size of the allocated block, so
	// we tell to the fb that it has a little bit less available memory so when
	// we allocated we can safely ad the size of the allocated block without writing it
	// on previous blocks.
	head->size = size - sizeof(size_t);
	head->next = (struct fb*) NULL;

	// Reinitialization of search function to fit_first
	mem_fit(&mem_fit_first);
}

void* mem_alloc(size_t size) {

// TODO : voit feuille Thomas 
	// Check if size asked allows to write the size of the allocated block
	if(size > sizeOfMem - sizeof(size_t) {
		fprintf(stderr, "Size asked too big\n");
		return NULL;
	}

	// Round the size asked to a multiple of struct fb
	if( size % sizeof(struct fb) != 0) {
		size_t sizeAsked = size + (sizeof(struct fb) - size % sizeof(struct fb));
	}	

	// free block found
	struct fb *freeB = NULL;
	searchFunction(freeB, size);

	if(freeB == NULL) {
		fprintf(stderr, "No free block available in memory\n");
		return NULL;
	}

	// Searching for previous free block pointer
	struct fb *prev = head;
	// If head is not the previous one
	if(prev != freeB) {
		while (prev->next != freeB) {
			if(prev->next == NULL) {
				fprintf(stderr, "Internal error in mem_alloc\n");
				return NULL;
			}

			prev = prev->next;
		}
	}

	// Reachain free block list
	prev->next = freeB->next;

	// Write size of the allocated block
	*((size_t*) freeB) = freeB->size - sizeof(size_t); 

	// Return the beginning of the allocated block
	return freeB + sizeof(size_t);
}

void mem_free(void* p) {

	// Get the size of the allocated block
	size_t blockSize = *(p - sizeof(size_t));

	// Check there is a previous free block next to it
	struct fb *prev = head;
	// If head is not the previous one
	if(prev->next > p) {
		while (prev->next != freeB) {
			if(prev->next == NULL) {
				fprintf(stderr, "Internal error in mem_alloc\n");
				return NULL;
			}

			prev = prev->next;
		}
	}
}

size_t mem_get_size(void * p) {
	return 0;
}

/* Itérateur sur le contenu de l'allocateur */
void mem_show(void (*print)(void *, size_t, int free)) {

}

void mem_fit(mem_fit_function_t* function) {
	searchFunction = function;
}

struct fb* mem_fit_first(struct fb* fb, size_t size) {
	return fb;
}

/* Si vous avez le temps */
struct fb* mem_fit_best(struct fb* fb, size_t size) {
	return fb;
}

struct fb* mem_fit_worst(struct fb* fb, size_t size) {
	return fb;
}