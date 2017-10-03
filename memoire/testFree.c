/**
 * This is a test file that aim to test a living memory.
 * It tries to randomly allocated and free some block to see the how the allocator reacts.
 * This test aims to test on a long time if the allocator still function normally.
 *
 */


#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h> 

// Max number of blocks that we manage
#define MAX_ALLOCATED_BLOCK 25
// The maximum number of alocation we try to do
#define NB_ALLOCATION_TEST 100

/**
 * Display a memory area
 */
void afficher_zone(void *adresse, size_t taille, int free)
{
  printf("Zone %s, Adresse : %p, Taille : %lu\n", free?"libre  ":"occupee",
         adresse, (unsigned long) taille);
}

/**
 * Generate a random number with a included, b excluded.
 * Need to call srand(time(NULL));
 */
int rand_block(int a, int b){
    return rand()%(b-a) +a;
}

static void alloc_random(size_t memSize) {

	printf("Testing with memory size = %zu\n\n", memSize);

	void *pointers [MAX_ALLOCATED_BLOCK];
	size_t size;
	// Current number of pointers to allocated blocks. < MAX_ALLOCATED_BLOCK.
	size_t nbPointers = 0;

	// >= 0, <= 100. Used to choose randomly between free and allocate. This value can vary along execution.

	void *result;
	// We try to alocate 1/3 of desired blocks in order to not have an empty memory.
	for(int i = 0; i < MAX_ALLOCATED_BLOCK / 3; i++){
		size = rand_block(0, memSize/2);
		result = mem_alloc(size);

		if(result == NULL){
			printf("block size (%zu) too big for remaining memory\n", size);
			//pointers[nbPointers] = NULL;
		}
		else{
			printf("Allocation done\n");
			afficher_zone(pointers[nbPointers], size, 0);
			pointers[nbPointers] = result;
			nbPointers++;
		}
	}

	printf("\n======First allocations done=====\n");
	mem_show(afficher_zone);
	printf("=============================\n\n");

	int randomNumber;

	unsigned int threshold = 50;
	int nbAlloc = 0;

	/*for (int i = 0; i < nbPointers; ++i) {
		mem_free(pointers[i]);
		mem_show(afficher_zone);
	}*/

	while(nbAlloc < NB_ALLOCATION_TEST) {
		randomNumber = rand_block(0, 100);
		
		// Try to allocate
		if(randomNumber >= threshold) {
			size = rand_block(0, memSize / 2);
			printf("Allocating %zu\n", size);
			result = mem_alloc(size);

			if(result != NULL) {
				printf("\nRandom : %d. Threshold : %u/100\n", randomNumber, threshold);
				pointers[nbPointers] = result;
				printf("Result %p\n", result);

				afficher_zone(pointers[nbPointers], size, 0);
				nbAlloc++;

				// Every 5 allocations, display memory.
				if(nbAlloc % 5 == 0) {
					printf("\n======%d allocation done=====\n", nbAlloc);
					mem_show(afficher_zone);
					printf("=============================\n\n");
				}

				nbPointers++;
			}
			else {
				//printf("failed to allocate\n");
			}
		}
		// Free a block
		else {
			printf("\nRandom : %d. Threshold : %u/100\n", randomNumber, threshold);
			size = rand_block(0, nbPointers);
			printf("Free %p, Number %zu/%zu\n", pointers[size], size, nbPointers);
			mem_free(pointers[size]);
			
			for (int i = size; i + 1 < nbPointers; ++i) {
				pointers[i] = pointers[i + 1];
			}

			nbPointers--;

			//mem_show(afficher_zone);

		}

		threshold = ((float)nbPointers / MAX_ALLOCATED_BLOCK) * 100;
	}

	mem_show(afficher_zone);

	printf("=======ALL ALOCATION DONE======\n");
	printf("free everything\n");
	for(int i = 0; i < nbPointers; ++i) {
		mem_free(pointers[i]);
	}

	printf("======END======\n");
	mem_show(afficher_zone);
}

int main(int argc, char *argv[]) {
	srand(time(NULL)); // initialisation de rand

	printf("\n==================\n");
	printf("Test Alloc random\n");
	printf("==================\n");

	size_t memSize = get_memory_size();
	mem_init(get_memory_adr(), memSize);

	alloc_random(memSize);

	// TEST OK
	return 0;
}
