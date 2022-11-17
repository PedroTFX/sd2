#ifndef _UTIL_H
#define _UTIL_H

int write_all(int sock, char* buf, int len);

int read_all(int sock, char** buf_ptr, int read_n_bytes);

int read_all2(int sock, char** buf_ptr);

int read_int_from_buffer(char *buffer);
#endif
