// Trabalho realizado por:
// Pedro Trindade 56342
// Joao Santos 56380
// Marcus Gomes 56326

#include <string.h>

#include "bubble_sort-private.h"

void swap(char** string_array, int posA, int posB) {
	char* temp = string_array[posA];
	string_array[posA] = string_array[posB];
	string_array[posB] = temp;
}

/* bubble sort */
void bubble_sort(char** string_array, int size) {
	for (int i = 0; i < size - 1; i++) {
		for (int j = size - 1; j > i; j--) {
			if (strcmp(string_array[j - 1], string_array[j]) > 0) {
				swap(string_array, j - 1, j);
			}
		}
	}
}
