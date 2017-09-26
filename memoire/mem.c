#include "mem.h"
#include <stdio.h>

struct fb {
	size_t size;
	struct fb *next;
};

// Head of free block chained list
struct fb *head;
// Adress of the end of memory
struct fb *end;

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

	end = (struct fb *)head + head->size;

	// Reinitialization of search function to fit_first
	mem_fit(mem_fit_first);
}

void* mem_alloc(size_t size) {

	// The rounded modified size asked
	size_t sizeAsked = size;
	// Round the size asked to a multiple of struct fb
	if( size % sizeof(struct fb) != 0) {
		sizeAsked = size + (sizeof(struct fb) - size % sizeof(struct fb));
	}

	// The total size that won't be available any more.
	size_t totalAllocated = sizeAsked + sizeof(size_t);
	// free block found
	struct fb *freeB = NULL;
	// We search for a free block with enough space to insert the allocated size
	// Also the search function will ensure that the free block will be either just small enough to be 
	// totally allocated or big enough to still have a correct struct fb.
	// It will also ensure that if the free block is the head, it will be big enough to contain its own struct fb.
	freeB = searchFunction(freeB, totalAllocated);

	// If we didn't find a free block with rounded size, then search with exact size.
	if(freeB == NULL) {
		sizeAsked = size;
		totalAllocated = sizeAsked + sizeof(size_t);
		freeB = searchFunction(freeB, totalAllocated);
	}
	// If we found nothing here, then there isn't enough available memory.
	if(freeB == NULL) {
		fprintf(stderr, "No free block available in memory\n");
		return NULL;
	}

	// If we need to delete this free block. Also make sure that we keep head (even if it is empty).
	if (totalAllocated == freeB->size && freeB != head) {
		// Searching for previous free block pointer
		struct fb *prev = head;
		// If head is not the allocated one
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


	// The beginning of the allocated space for the user
	void *userPointer = (void*)freeB + freeB->size - sizeAsked;

	size_t freeBSize = freeB->size;


	size_t *allocatedBlockSizePointer = (void *) userPointer - sizeof(size_t);
	/*printf("fb : %p, fbSize : %zu, up : %p, sizeof(size_t) : %zu, sizeof(struct) : %zu, sizeAsked : %zu\n", (size_t *)freeB, freeBSize, userPointer, sizeof(size_t), sizeof(struct fb), sizeAsked);
	printf("user pointer - sizeof(size_t) = %p\n", 	allocatedBlockSizePointer);
	printf("user pointer - 1 = %p\n", 	((size_t*) userPointer - 1));
	printf("whats %zu\n", *allocatedBlockSizePointer);*/

	//printf("user pointer : %p; size pointer : %p, size writen %zu\n", userPointer, allocatedBlockSizePointer, sizeAsked);
	// Write size of the allocated block before the allocated block wich is at the end of the free block
	*allocatedBlockSizePointer = sizeAsked;


	// Update the available size in this free block only if freeB will stay (if the block wasn't totally allocated)
	if(freeBSize >= totalAllocated + sizeof(struct fb)) {
		freeB->size -= totalAllocated;
	}

	return userPointer;
}

// Get the size of an allocated block by reading it with a (little) validity check.
size_t mem_get_size(void * p) {
	// Get the size of the allocated block
	size_t blockSize = *((size_t *)((void*)p - sizeof(size_t)));

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
	size_t totalAllocatedSize = blockSize + sizeof(size_t);

	// Find the previous free block
	struct fb *prev = head;
	while (prev->next != NULL && (void*)prev->next < (void*)p) {

		if(prev->next == NULL) {
			fprintf(stderr, "Internal error in mem_free\n");
			return;
		}
		prev = prev->next;
	}

	// Get the following one
	struct fb *nextFb = prev->next;


	// Boolean to flag if we need to fusion with previouse and/or next free block.
	printf("prev : %p, p : %p, size : %zu\n", prev, p, blockSize);
	int prevFusion = (void*)prev + prev->size == (void *)p - sizeof(size_t);
	int nextFusion = 0;
	// Test if next fb is right next to it (no allocated space between)
	if(nextFb != NULL) {
		nextFusion = (void*)nextFb == (void *)p + blockSize;
	}

	// For each cases
	if(prevFusion && nextFusion) {
		prev->size += totalAllocatedSize + nextFb->size;
		prev->next = nextFb->next;
		printf("patate 1\n");
	}
	else if(prevFusion) {
		prev->size += totalAllocatedSize;
		printf("patate 2\n");
	}
	else if(nextFusion) {
		size_t nextFbSize = nextFb->size + totalAllocatedSize;
		nextFb = ((void*)p - sizeof(size_t));
		nextFb->size += nextFbSize;
		printf("patate 3\n");
	}
	// Create a new fb
	else {
		struct fb *newFb = ((void*)p - sizeof(size_t));
		newFb->size = totalAllocatedSize;
		newFb->next = prev->next;
		prev->next = newFb;
		printf("patate 4\n");
	}
}

/* Itérateur sur le contenu de l'allocateur */
void mem_show(void (*print)(void *, size_t, int free)) {

	struct fb *currentFb = head;

	while(currentFb != NULL) {
		// This verification is for the head, because the head will always exist
		if(currentFb->size > 0) {
			print(currentFb, currentFb->size, 1);
		}


		size_t allocatedSize = 0;

		// Where is the next free block
		struct fb *nextBlock;
		// If there is one then it's its memory
		if(currentFb->next != NULL) {
			nextBlock = currentFb->next;
		}
		// If there is no next free block then it's the end of memory
		else {
			nextBlock = end;
		}
		// Iterate over all allocated blocks between 2 free blocks
		while((struct fb *)currentFb + currentFb->size + allocatedSize < (struct fb *)nextBlock) {
			void *p = (void *)currentFb + currentFb->size + allocatedSize + sizeof(size_t);
			// Get the size of the allocated block
			size_t blockSize = *((size_t *)((void*)p - sizeof(size_t)));

			allocatedSize += blockSize + sizeof(size_t);
			print(p, blockSize, 0);
		}


		currentFb = currentFb->next;
	}
}

void mem_fit(mem_fit_function_t* function) {
	searchFunction = function;
}

// We search for the first free block with enough space to insert the allocated size
// Also the search function will ensure that the free block will be either just small enough to be 
// totally allocated or big enough to still have a correct struct fb.
// It will also ensure that if the free block is the head, it will be big enough to contain its struct fb.
struct fb* mem_fit_first(struct fb* fb, size_t size) {
	fb = head;

	// Test the head with all constraints
	if(fb->size >= size + sizeof(struct fb)) {
		return fb;
	}
	else {
		fb = fb->next;
	}

	while (fb != NULL) {

		// We ensure that either the block will be totally allocated, or either there will be still enough place to 
		// have the struct fb
		if(fb->size >= size && (fb->size - size == 0 || fb->size - size >= sizeof(struct fb)) ) {
			break;
		}
		fb = fb->next;
	}

	return fb;
}

/* Si vous avez le temps */
struct fb* mem_fit_best(struct fb* fb, size_t size) {
	return fb;
}

struct fb* mem_fit_worst(struct fb* fb, size_t size) {
	return fb;
}