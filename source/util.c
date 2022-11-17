#include "util.h"
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

int write_all(int sock, char* buf, int len) {
	char* buf_with_self_size = malloc(sizeof(int) + len);
	memcpy(buf_with_self_size, &len, sizeof(int));
	memcpy(buf_with_self_size + sizeof(int), buf, len);

	len += sizeof(int);
	int buf_size = len;
	char* cursor = buf_with_self_size;
	while (len > 0) {
		int res = write(sock, cursor, len);
		printf("Written %d bytes\n", res);
		if (res <= 0) {
			if (errno == EINTR)
				continue;
			return res;
		}
		cursor += res;
		len -= res;
	}
	free(buf_with_self_size);
	printf("Bytes written: %d\n", buf_size);
	return buf_size;
}



int read_all2(int sock, char** buf) {
	// Read 4 bytes
	int buffer_size;
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
	printf("Bytes read: %d\n", buffer_size);
	return buffer_size;
}



int read_all(int sock, char** buf_ptr, int read_n_bytes) {
	int bytes_read = 0;
	int total_bytes_read = 0;
	int fullBuffer;
	int buffer_size = read_n_bytes;
	int keepReading;
	//struct pollfd connection;
	//connection.events = POLLIN; // There is data to read...
	//connection.fd = sock; // ...on the client socket
	do {
		bytes_read = read(sock, (*buf_ptr) + total_bytes_read, read_n_bytes);
		if (bytes_read < 0) {
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
		//buffer_size-=bytes_read;
		// int socketHasDataToRead = poll(&connection, 1, 0) > 0;
		char c;
		int socketHasDataToRead = recv(sock, (void*)&c, 1, MSG_DONTWAIT | MSG_PEEK) > 0;
		// If bytes_read == 0, we reached EFO (client closed the connection: don't read again)
		keepReading = bytes_read && socketHasDataToRead;
	} while (keepReading);
	printf("Bytes read: %d\n", total_bytes_read);
	return total_bytes_read;
}

int read_int_from_buffer(char *buffer) {
	return *buffer | *(buffer + 1) << 8 | *(buffer + 2) << 16 | *(buffer + 3) << 24;
}
