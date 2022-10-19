// Trabalho realizado por:
// Pedro Trindade 56342
// Joao Santos 56380
// Marcus Gomes 56326
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "data.h"
#include "entry.h"
#include "serialization-private.h"

/* Serializa todas as keys presentes no array de strings keys
 * para o buffer keys_buf que será alocado dentro da função.
 * O array de keys a passar em argumento pode ser obtido através
 * da função tree_get_keys. Para além disso, retorna o tamanho do
 * buffer alocado ou -1 em caso de erro.
 */
// char *keys_buf;
// keyArray_to_buffer(keys, &keys_buf);
// Aqui o keys_buf já aponta para uma posição de memória preenchida
int keyArray_to_buffer(char **keys, char **keys_buf)
{
	//[10]["qwertyuiop"]	[2]["ab"]	[3]["qwe"]	[0]

	// Descobrir quantos bytes temos que alocar para o buffer
	int size = 4; // calc_buffer_size()
	for (int i = 0; keys[i] != NULL; i++)
	{
		size += 4 + strlen(keys[i]);
	}

	// int size = calc_buffer_size(keys);

	// Alocar esse espaço para o buffer
	*keys_buf = (char *)malloc(size);
	char *buffer_ptr = *keys_buf;

	// create_buffer
	//  Preencher o buffer com as strings
	// create_buffer(keys, *keys_buf);
	for (int i = 0; keys[i] != NULL; i++)
	{
		int strlength = strlen(keys[i]);
		memcpy(buffer_ptr, &strlength, sizeof(int));
		buffer_ptr += sizeof(int);
		memcpy(buffer_ptr, keys[i], strlength);
		buffer_ptr += strlength;
	}

	// e meter 0 final (create_buffer)
	int f = 0;
	memcpy(buffer_ptr, &f, sizeof(int));

	// Retornar a quantidade de bytes alocados
	return size;
}

/* De-serializa a mensagem contida em keys_buf, com tamanho
 * keys_buf_size, colocando-a e retornando-a num array char**,
 * cujo espaco em memória deve ser reservado. Devolve NULL
 * em caso de erro.
 */
char **buffer_to_keyArray(char *keys_buf, int keys_buf_size)
{
	// Calcular quantidade de strings no buffer
	int num_strings = num_strings_in_buffer(keys_buf);

	//  Alocar memória para o array de ponteiros para strings
	char **keys_array = (char **)malloc(num_strings * sizeof(char *));

	// Preencher o array com os ponteiros para essas strings
	copy_strings_from_buffer_to_keys_array(keys_buf, keys_array);

	return keys_array;
}

int num_strings_in_buffer(char *keys_buf)
{
	int num_strings = 0;
	int str_length;
	do
	{
		str_length = *keys_buf | *(keys_buf + 1) << 8 | *(keys_buf + 2) << 16 | *(keys_buf + 3) << 24;
		if (str_length > 0)
		{
			num_strings++; // Incrementar num_strings
		}
		keys_buf += 4 + str_length; // Avançamos até à próxima string
	} while (str_length != 0);
	// [][][][]
	//											0101 1111
	// |							0110 1110	0000 0000
	// |				1011 0000	0000 0000	0000 0000
	// |	1111 1000	0000 0000	0000 0000	0000 0000
	// 		1111 1000	1011 0000	0110 1110	0101 1111

	// Little Endian
	/**
	 * 100[0101 1111] <- LSB
	 * 101[0110 1110]
	 * 102[1011 0000]
	 * 103[1111 1000] <- MSB
	 */

	return num_strings;
}

void copy_strings_from_buffer_to_keys_array(char *keys_buf, char **keys_array)
{
	int str_length = *keys_buf | *(keys_buf + 1) << 8 | *(keys_buf + 2) << 16 | *(keys_buf + 3) << 24;
	keys_buf += 4;
	int index = 0;
	// Percorrer keys_buf
	while (str_length != 0)
	{
		// Allocate memory for the string
		keys_array[index] = (char *)malloc(str_length + 1);
		// Copy string
		memcpy(keys_array[index], keys_buf, str_length);
		// Advance pointer
		keys_buf += str_length;
		// Add terminating character
		keys_array[index][str_length] = '\0';
		// Get next string length
		str_length = *keys_buf | *(keys_buf + 1) << 8 | *(keys_buf + 2) << 16 | *(keys_buf + 3) << 24;
		keys_buf += 4;
		index++;
	}
}

int calc_buffer_size(char **keys)
{
	int size = 0;
	for (int i = 0; keys[i] != NULL; i++)
	{
		size += 4 + strlen(keys[i]);
	}
	return size + 4; // Buffer ends with the 0 integer
}

void create_buffer(char **keys, char *keys_buf)
{
	// create_buffer
	// Preencher o buffer com as strings
	for (int i = 0; keys[i] != NULL; i++)
	{
		int strlength = strlen(keys[i]);
		memcpy(keys_buf, &strlength, sizeof(int));
		keys_buf += sizeof(int);
		memcpy(keys_buf, keys[i], strlength);
		keys_buf += strlength;
	}

	// e meter 0 final (create_buffer)
	int f = 0;
	memcpy(keys_buf, &f, sizeof(int));
}
