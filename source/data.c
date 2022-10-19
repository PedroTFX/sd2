// Trabalho realizado por:
// Pedro Trindade 56342
// Joao Santos 56380
// Marcus Gomes 56326
#include <data.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Função que cria um novo elemento de dados data_t, reservando a memória
 * necessária para armazenar os dados, especificada pelo parâmetro size
 */
struct data_t *data_create(int size) {
	//invalid size
	if (size <= 0) {
		return NULL;
	}

	struct data_t* data = malloc(sizeof(struct data_t));
	if (data == NULL) { //error on init
		data = NULL;
		return NULL;
	}

	data->datasize = size;
	data->data = malloc(size);

	//error on memory reserve
	if (data->data == NULL) {
		free(data);
		return NULL;
	}
	return data;
}

/* Função que cria um novo elemento de dados data_t, inicializando o campo
 * data com o valor passado no parâmetro data, sem necessidade de reservar
 * memória para os dados.
 */
struct data_t *data_create2(int size, void *data) {
	//invalid size or data
	if ((size <= 0) || data == NULL) {
		return NULL;
	}

	struct data_t* data_st = malloc(sizeof(struct data_t));
	if (data_st == NULL) {	//error on init
		data_st = NULL;
		return NULL;
	}
	
	data_st->datasize = size;
	data_st->data = data;
	return data_st;
}

/* Função que elimina um bloco de dados, apontado pelo parâmetro data,
 * libertando toda a memória por ele ocupada.
 */
void data_destroy(struct data_t *data) {
	if(data){
		if(data->data) {
			free(data->data);
			data->data = NULL;
		}
		free(data);
		data = NULL;
	}
}

/* Função que duplica uma estrutura data_t, reservando toda a memória
 * necessária para a nova estrutura, inclusivamente dados.
 */
struct data_t *data_dup(struct data_t *data) {
	if (!data || data->datasize <= 0 || !data->data) {
		return NULL;
	}

	struct data_t* data_st = malloc(sizeof(struct data_t));
	if (!data_st) { // error on init
		return NULL;
	}

	data_st->datasize = data->datasize;
	data_st->data = malloc(data->datasize);

	// error on init
	if (!data_st->data) {
		data_destroy(data_st);
		return NULL;
	}
	memcpy(data_st->data, data->data, data->datasize);
	return data_st;
}

/* Função que substitui o conteúdo de um elemento de dados data_t.5
 *  Deve assegurar que destroi o conteúdo antigo do mesmo.
 */
void data_replace(struct data_t *data, int new_size, void *new_data) {
	//memset(data, 0, data->datasize);	//clean mem
	free(data->data);					//reclaim mem
	data->data = new_data;
	data->datasize = new_size;
	
}


