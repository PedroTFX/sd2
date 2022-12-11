// Trabalho realizado por:
// Pedro Trindade 56342
// Joao Santos 56380
// Marcus Gomes 56326
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

#include "util.h"

int write_all(int sock, char* buf, int len) {
	char* buf_with_self_size = malloc(sizeof(int) + len);
	memcpy(buf_with_self_size, &len, sizeof(int));
	memcpy(buf_with_self_size + sizeof(int), buf, len);

	len += sizeof(int);
	int buf_size = len;
	char* cursor = buf_with_self_size;
	while (len > 0) {
		int res = write(sock, cursor, len);
		if (res <= 0) {
			if (errno == EINTR)
				continue;
			return res;
		}
		cursor += res;
		len -= res;
	}
	free(buf_with_self_size);
	return buf_size;
}

int read_all2(int sock, char** buf) {
	// Read 4 bytes
	int buffer_size = -1;
	int bytes_read = read(sock, &buffer_size, sizeof(int));

	// Socket was closed
	int isEOF = bytes_read == 0;
	if(isEOF) {
		return 0;
	}

	// Allocate mem for buffer
	*buf = (char* )malloc(buffer_size);

	// Read buffer_size bytes to buffer
	int len = buffer_size;
	char* cursor = *buf;
	while (len > 0) {
		bytes_read = read(sock, cursor, len);
		if (bytes_read < 0) {
			if (errno == EINTR)
				continue;
			perror("read failed:");
			return bytes_read;
		}
		cursor += bytes_read;
		len -= bytes_read;
	}
	return buffer_size;
}
