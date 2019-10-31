#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// port 31415 is an unassigned port
// according to the IANA Port Number Registry
#define DEFAULT_PORT 31415

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

	return 0;
}
