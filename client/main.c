#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <sys/socket.h>

// port 31415 is an unassigned port
// according to the IANA Port Number Registry
#define DEFAULT_PORT 31415
#define DEFAULT_IP "127.0.0.1"

void usage() {
	fprintf(stderr, "usage: krypto [-p <port>] [-i <address>] <file>\n");
	fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
	char c;

	int port = DEFAULT_PORT;
	char *ip = DEFAULT_IP;

	// getopt shouldn't print errors
	opterr = 0;

	while ((c = getopt(argc, argv, "p:i:")) != -1) {
		switch (c) {
			case 'p':
				port = atoi(optarg);
				break;
			case 'i':
				ip = optarg;
				break;
			default:
				usage();
				return 1;
		}
	}

	int sockfd = socket(PF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		fprintf(stderr, "Couldn't create socket\n");
		return 1;
	}

	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if (inet_aton("127.0.0.1", &address.sin_addr) == 0) {
		fprintf(stderr, "Couldn't parse the supplied address\n");
		return 1;
	}

	if (connect(sockfd, (struct sockaddr *) &address, sizeof(address))) {
		fprintf(stderr, "Couldn't create a connection on the socket\n");
		return 1;
	}

	close(sockfd);
	return 0;
}
