#include "mem.h"
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h> 

#define NB_TESTS 10
#define MAX_ALLOCATED_BLOCK 25
#define NB_ALLOCATION_TEST 100

void afficher_zone(void *adresse, size_t taille, int free)
{
  printf("Zone %s, Adresse : %lx, Taille : %lu\n", free?"libre":"occupee",
         (unsigned long) adresse, (unsigned long) taille);
}

// a included, b excluded
int rand_block(int a, int b){
    return rand()%(b-a) +a;
}

static void alloc_random(size_t estimate) {

	void *pointers [MAX_ALLOCATED_BLOCK];
	size_t size;
	size_t nbPointers = 0;

	void *result;
	for(int i = 0; i < MAX_ALLOCATED_BLOCK / 3; i++){
		size = rand_block(0, estimate/2);
		result = mem_alloc(size);

		if(result == NULL){
			printf("block size too big for remaining memory\n");
			//pointers[nbPointers] = NULL;
		}
		else{
			pointers[nbPointers] = result;
			nbPointers++;
		}
	}

	int randomNumber;

	int nbAlloc = 0;

	while(nbAlloc < NB_ALLOCATION_TEST) {
		randomNumber = rand_block(0, 2);
		
		if(randomNumber == 0) {
			size = rand_block(0, estimate / 2);
			pointers[nbPointers] = mem_alloc(size);
			size++;
			nbAlloc++;
		}
		else if (nbPointers > 0){
			size = rand_block(0, nbPointers);
			mem_free(pointers[size]);
		}

		if(nbAlloc % 5 == 0) {
			printf("======%d allocation done=====\n", nbAlloc);
			mem_show(afficher_zone);
			printf("=============================\n");
		}
	}

	printf("======END======\n");
	mem_show(afficher_zone);
}

int main(int argc, char *argv[]) {
	srand(time(NULL)); // initialisation de rand

	printf("Test Alloc random\n");
	mem_init(get_memory_adr(), get_memory_size());
	alloc_random(get_memory_size());

	// TEST OK
	return 0;
}
