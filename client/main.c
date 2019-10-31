#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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

	return 0;
}
