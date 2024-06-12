#include <arpa/inet.h>
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#define HOST "127.0.0.1"
#define MAX_LUNGHEZZA_CODA 50

int init_socket(int port){
	// Questo metodo crea una socket e ne ritorna il file descriptor
    int socket_fd = -1;
    struct sockaddr_in servCV;

    printf("Creo una socket\n");

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);

	if(socket_fd == -1)
	{
		perror("socket");
		exit(1);
	}

	servCV.sin_family = AF_INET;
	servCV.sin_addr.s_addr = htonl(INADDR_ANY);
	servCV.sin_port = htons(port);
	
	int bind_ret = -1;
	bind_ret = bind(socket_fd, (struct sockaddr *)&servCV, sizeof(servCV));
	if(bind_ret == -1)
	{
		perror("bind");
		exit(1);
	}

	int listen_ret = -1;
	listen_ret = listen(socket_fd, MAX_LUNGHEZZA_CODA);
	if(listen_ret == -1)
	{
		perror("listen");
		exit(1);
	}
    printf("Socket creata: INADDR_ANY:%d\n", port);

	return socket_fd;
}