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

// The total size of the memory
size_t sizeOfMem;

void mem_init(char* mem, size_t size) {

	// The total size of the memory
	sizeOfMem = size;
	
	// Create the head of chained list at 0x0
	head = (struct fb *) mem;
	head->size = size;
	head->next = NULL;

	// Reinitialization of search function to fit_first
	mem_fit(&mem_fit_first);
}

void* mem_alloc(size_t size) {

	// The rounded modified size asked
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

	// If we need to delete this free block. Also make sure that we keep head (even if it is empty).
	if (totalAllocated == freeB->size && freeB != head) {
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
	void *userPointer = freeB + freeB->size - sizeAsked;

	// Update the available size in this free block
	freeB->size -= totalAllocated;

	return userPointer;
}

size_t mem_get_size(void * p) {
	// Get the size of the allocated block
	size_t blockSize = *((size_t*)p - sizeof(size_t));

	// Test if the pointer is effectively one that we could have allocate
	if(blockSize % sizeof(struct fb) != 0) {
		fprintf(stderr, "Invalid pointer to get size\n");
		return 0;
	}

	return blockSize;
}

void mem_free(void* p) {

	// Get the size of the allocated block
	size_t blockSize = mem_get_size(p);

	// The total size that need to be freed
	size_t totalSize = blockSize + sizeof(size_t);

	// Check if there is a previous free block next to it
	struct fb *prev = head;
	while (prev->next < (struct fb *)p) {

		if(prev->next == NULL) {
			fprintf(stderr, "Internal error in mem_free\n");
			return;
		}
		prev = prev->next;
	}

	// Get the following one
	struct fb *nextFb = prev->next;


	// Boolean to flag if we need to fusion with previouse and/or next free block.
	int prevFusion = prev + prev->size == p - sizeof(size_t);;
	int nextFusion = 0;
	// Test if next fb is right next to it (no allocated space between)
	if(nextFb != NULL) {
		nextFusion = nextFb == p + blockSize;
	}

	// For each cases
	if(prevFusion && nextFusion) {
		prev->size += totalSize + nextFb->size;
		prev->next = nextFb->next;
	}
	else if(prevFusion) {
		prev->size += totalSize;
	}
	else if(nextFusion) {
		nextFb = (struct fb*)((size_t*)p - sizeof(size_t));
		nextFb->size += totalSize;
	}
	// Create a new fb
	else {
		struct fb *newFb = (struct fb*)((size_t*)p - sizeof(size_t));
		newFb->size = totalSize;
		newFb->next = prev->next;
		prev->next = newFb;
	}
}

/* Itérateur sur le contenu de l'allocateur */
void mem_show(void (*print)(void *, size_t, int free)) {

	struct fb *currentFb = head;
	// We read one free block then the next allocated one (since there's can't be 2 free block next to each other)
	while (currentFb != NULL) {

		print(currentFb, currentFb->size, 1);

		// If there is an allocated block next to the current fb
		if((size_t)currentFb + currentFb->size == sizeOfMem) {
			void *p = currentFb + currentFb->size + sizeof(size_t);
			// Get the size of the allocated block
			size_t blockSize = *((size_t*)p - sizeof(size_t));

			print(p, blockSize, 0);
		}

		currentFb = currentFb->next;
	}
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