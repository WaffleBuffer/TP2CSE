#include "mem.h"
#include <stdio.h>

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
	head->size = size;
	head->next = (struct fb*) NULL;

	// Reinitialization of search function to fit_first
	mem_fit(&mem_fit_first);
}

void* mem_alloc(size_t size) {

	// The slighty modified size asked
	size_t sizeAsked = 0;
	// Round the size asked to a multiple of struct fb
	if( size % sizeof(struct fb) != 0) {
		sizeAsked = size + (sizeof(struct fb) - size % sizeof(struct fb));
	}	

	// The total size of octets that won't be available any more.
	size_t totalAllocated = sizeAsked + sizeof(size_t);
	// free block found
	struct fb *freeB = NULL;
	// We search for a free block with enough space to insert the allocated size
	searchFunction(freeB, totalAllocated);

	if(freeB == NULL) {
		fprintf(stderr, "No free block available in memory\n");
		return NULL;
	}

	// If we need to delete this free block
	if (totalAllocated == freeB->size) {
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
	}

	// Write size of the allocated block before the allocated block wich is at the end of the free block
	*((size_t*) freeB + freeB->size - totalAllocated) = freeB->size; 

	// The beginning of the allocated space for the user
	size_t *userPointer = (size_t *)freeB + freeB->size - sizeAsked;

	// Update the available size in this free block
	freeB->size -= totalAllocated;

	return userPointer;
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