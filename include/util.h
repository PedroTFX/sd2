#ifndef _UTIL_H
#define _UTIL_H

int write_all(int sock, char* buf, int len);

int read_all(int sock, char** buf_ptr, int read_n_bytes);

#endif
