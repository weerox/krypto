#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <arpa/inet.h>

#include <netinet/in.h>

#include <openssl/evp.h>
#include <openssl/rand.h>

#include <sys/socket.h>

// port 31415 is an unassigned port
// according to the IANA Port Number Registry
#define DEFAULT_PORT 31415
#define DEFAULT_IP "127.0.0.1"

// generates a key and an IV from a password
// the salt used for generation is returned
unsigned char *generate_key_iv(
	char *password, unsigned char *key, unsigned char *iv
) {
	unsigned char *salt = malloc(8);

	RAND_bytes(salt, 8);

	EVP_BytesToKey(
		EVP_aes_256_gcm(), EVP_sha256(), (unsigned char *) salt,
		(unsigned char *) password, strlen(password), 1,
		key, iv
	);

	return salt;
}

// encrypts `filename' using `password'
// returns the filename of the encrypted file
char *encrypt_file(char *filename, char *password) {
	EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();

	if (ctx == NULL) {
		printf("Couldn't create a new cipher context\n");
		return 0;
	}

	if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, NULL, NULL) == 0) {
		printf("Couldn't initialize encryption\n");
		return 0;
	}

	unsigned char key[EVP_MAX_KEY_LENGTH];
	unsigned char iv[EVP_MAX_IV_LENGTH];

	unsigned char *salt = generate_key_iv(password, key, iv);

	// the filename of the encoded file is the Base64 encoding of the salt
	char *filename_enc = malloc(13);
	EVP_EncodeBlock(filename_enc, salt, 8);

	FILE *fplain = fopen(filename, "r");
	FILE *fenc = fopen(filename_enc, "w");

	char buffer[1024], buffer_enc[1024];
	int buffer_len = 0, buffer_enc_len = 0;

	if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), NULL, key, iv) == 0) {
		printf("Couldn't initialize encryption\n");
		return 0;
	}

	while (!feof(fplain)) {
		buffer_len = fread(buffer, 1, 1024, fplain);

		if (EVP_EncryptUpdate(
			ctx,
			(unsigned char *) buffer_enc, &buffer_enc_len,
			(unsigned char *) buffer, buffer_len
		) == 0) {
			printf("Couldn't encrypt data\n");
			return 0;
		}

		fwrite(buffer_enc, 1, buffer_enc_len, fenc);
	}

	if (EVP_EncryptFinal_ex(
		ctx,
		(unsigned char *) buffer_enc, &buffer_enc_len
	) == 0) {
		printf("Couldn't encrypt data\n");
		return 0;
	}

	fwrite(buffer_enc, 1, buffer_enc_len, fenc);

	fclose(fenc);
	fclose(fplain);

	return filename_enc;
}

void send_file(int sockfd, char *file) {
	/* send the Base64-encoded filename */
	write(sockfd, file, 13);

	FILE *fp = fopen(file, "r");

	char buffer[1024];
	int buffer_len = 0;

	while (!feof(fp)) {
		buffer_len = fread(buffer, 1, 1024, fp);

		write(sockfd, buffer, buffer_len);
	}

	fclose(fp);
}

int create_socket(char *ip, int port) {
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);

	if (sockfd == -1) {
		fprintf(stderr, "Couldn't create socket\n");
		return 1;
	}

	struct sockaddr_in address;

	address.sin_family = AF_INET;
	address.sin_port = htons(port);

	if (inet_aton(ip, &address.sin_addr) == 0) {
		fprintf(stderr, "Couldn't parse the supplied address\n");
		return 1;
	}

	if (connect(sockfd, (struct sockaddr *) &address, sizeof(address))) {
		fprintf(stderr, "Couldn't create a connection on the socket\n");
		return 1;
	}

	return sockfd;
}

void close_socket(int sockfd) {
	close(sockfd);
}

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

	if (argc - optind != 1) {
		usage();
		return 1;
	}

	char *file = *(argv + optind);

	int sockfd = create_socket(ip, port);

	printf("Password: ");
	char password[256];
	fgets(password, 256, stdin);

	char *file_enc = encrypt_file(file, password);

	send_file(sockfd, file_enc);

	// remove the encoded file stored which was temporarily stored
	remove(file_enc);

	close_socket(sockfd);
	return 0;
}
