// Trabalho realizado por:
// Pedro Trindade 56342
// Joao Santos 56380
// Marcus Gomes 56326
#include <data.h>
#include <entry.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Função que cria uma entry, reservando a memória necessária para a
 * estrutura e inicializando os campos key e value, respetivamente, com a
 * string e o bloco de dados passados como parâmetros, sem reservar
 * memória para estes campos.
 */
struct entry_t *entry_create(char *key, struct data_t *data) {
	struct entry_t* entry = malloc(sizeof(struct entry_t));

	if (entry == NULL) {
		return NULL;
	}
	entry->key = key;
    entry->value = data;
    return entry;
}

/* Função que substitui o conteúdo de uma entrada entry_t.
*  Deve assegurar que destroi o conteúdo antigo da mesma.
*/
void entry_replace(struct entry_t *entry, char *new_key, struct data_t *new_value){
	free(entry->key);
    entry->key = new_key;

	data_destroy(entry->value);
	entry->value = new_value;
}

/* Função que elimina uma entry, libertando a memória por ela ocupada
 */
void entry_destroy(struct entry_t *entry) {
	if(entry){
		if(entry->value){
			data_destroy(entry->value);
		}
		if(entry->key){
			free(entry->key);
			entry->key = NULL;
		}
		free(entry);
		entry = NULL;
	}
}

/* Função que duplica uma entry, reservando a memória necessária para a
 * nova estrutura.
 */
struct entry_t *entry_dup(struct entry_t *entry) {
	if(!entry){
		return NULL;
	}

	struct entry_t* entry2 = malloc(sizeof(struct entry_t));
	if (!entry2) { //error init
		return NULL;
	}

	entry2->key = strdup(entry->key);
	entry2->value = data_dup(entry->value);
	return entry2;
}

/* Função que compara duas entradas e retorna a ordem das mesmas.
*  Ordem das entradas é definida pela ordem das suas chaves.
*  A função devolve 0 se forem iguais, -1 se entry1<entry2, e 1 caso
contrário.
*/
int entry_compare(struct entry_t *entry1, struct entry_t *entry2) {
	int result = strcmp(entry1->key, entry2->key);
	return (result == 0) ? 0 : (result > 0) ? 1 : -1;
}


