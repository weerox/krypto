#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <netinet/in.h>

#include <sys/socket.h>

// port 31415 is an unassigned port
// according to the IANA Port Number Registry
#define DEFAULT_PORT 31415

void recv_file(int sockfd) {
	/* read the Base64-encoded filename */
	char *file = malloc(13);
	read(sockfd, file, 13);

	FILE *fp = fopen(file, "w");

	char buffer[1024];
	int buffer_len = 0;

	while ((buffer_len = read(sockfd, buffer, 1024)) > 0) {
fwrite(buffer, 1, buffer_len, stdout);
		fwrite(buffer, 1, buffer_len, fp);
	}

	fclose(fp);
}

int create_socket(int port) {
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		printf("Couldn't create socket\n");
		return 1;
	}

	struct sockaddr_in address;
	socklen_t socklen;

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = INADDR_ANY;

	socklen = sizeof(address);

	if (bind(sockfd, (struct sockaddr *) &address, socklen) == -1) {
		fprintf(stderr, "Couldn't bind the socket\n");
		return 1;
	}

	if (listen(sockfd, 0) == -1) {
		fprintf(stderr, "Couldn't listen on socket\n");
		return 1;
	}

	int accepted_socket =
		accept(sockfd, (struct sockaddr *) &address, &socklen);

	if (accepted_socket == -1) {
		fprintf(stderr, "Couldn't accept the connection\n");
		return 1;
	}

	close(sockfd);

	return accepted_socket;
}

void close_socket(int sockfd) {
	close(sockfd);
}

void usage() {
	fprintf(stderr, "usage: krypto [-p <port>]\n");
	fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
	char c;

	int port = DEFAULT_PORT;

	// getopt shouldn't print errors
	opterr = 0;

	while ((c = getopt(argc, argv, "p:")) != -1) {
		switch (c) {
			case 'p':
				port = atoi(optarg);
				break;
			default:
				usage();
				return 1;
		}
	}

	int sockfd = create_socket(port);

	recv_file(sockfd);

	close_socket(sockfd);
	return 0;
}
