//make by:
// João Santos 56380
// Marcos Gomes 56326
// Pedro Trindade 56342
#include <errno.h>
#include "util.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int write_all(int sock, char* buf, int len) {
	int bufsize = len;
	while (len > 0) {
		int res = write(sock, buf, len);
		if (res < 0) {
			if (errno == EINTR)
				continue;
			perror("write failed:");
			return res;
		}
		buf += res;
		len -= res;
	}
	return bufsize;
}

int read_all(int sock, char** buf_ptr, int read_n_bytes) {
	int bytes_read = 0;
	int total_bytes_read = 0;
	int fullBuffer;
	int buffer_size = read_n_bytes;
	do {
		bytes_read = read(sock, (*buf_ptr) + total_bytes_read, read_n_bytes);
		if(bytes_read < 0) {
			if (errno == EINTR)
				continue;
			perror("read failed:");
			return bytes_read;
		}
		fullBuffer = bytes_read == read_n_bytes;
		if (fullBuffer) {
			buffer_size += read_n_bytes;
			*buf_ptr = (char*)realloc(*buf_ptr, buffer_size);
		}
		total_bytes_read += bytes_read;
	} while (fullBuffer);
	return total_bytes_read;
}
