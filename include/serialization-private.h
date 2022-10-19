

#ifndef _SERIALIZATION_PRIVATE_H
#define _SERIALIZATION_PRIVATE_H

#include "serialization.h"

void copy_strings_from_buffer_to_keys_array(char *keys_buf, char **keys_array);

int num_strings_in_buffer(char *keys_buf);

int calc_buffer_size(char **keys);

void create_buffer(char **keys, char *keys_buf);

#endif
