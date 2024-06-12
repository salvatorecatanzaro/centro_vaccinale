#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "utils.h"


int socket_loop(int socket_fd, char *server_v_ip)
{
	int conn_fd = -1;
	int server_v_fd = -1;
	int n;
	struct sockaddr_in server_v;
	struct payload_green_pass green_pass;
	pid_t pid;

	for (;;)
	{
		printf("In attesa di connessioni . . . \n");
		
		conn_fd = accept(socket_fd, (struct sockaddr *)NULL, NULL);
		if (conn_fd == -1)
		{
			perror("accept");
			exit(1);
		}

		if ((pid = fork()) < 0)
		{
			perror("fork");
		    exit(-1);
		}

		if (pid == 0)
		{
			printf("Connessione riuscita, il processo che la gestisce ha pid %ld\n", (long)getpid());
			close(socket_fd);
			while ((n = read(conn_fd, green_pass.numero_identificazione_tessera, 20)) > 0)
			{
				green_pass.numero_identificazione_tessera[n] = 0;
				green_pass.periodo_validita = time(NULL) + SEI_MESI_SEC;
				green_pass.numero_operazione = AGGIUNTA_GREEN_PASS;

				if (n < 0)
				{
					fprintf(stderr, "read error\n");
				    exit(1);
				}
				
				server_v_fd = socket(AF_INET, SOCK_STREAM, 0);
				if (server_v_fd == -1)
				{
					perror("socket");
				    exit(1);
				}

				server_v.sin_family = AF_INET;
				server_v.sin_port = htons(SERVER_V_PORT);

				if (inet_pton(AF_INET, server_v_ip, &server_v.sin_addr) < 0)
				{
					fprintf(stderr, "inet_pton error for %s\n", server_v_ip);
				    exit(1);
				}

				printf("Tentativo d connessione verso server v . . .\n");
				int conn_result = connect(server_v_fd, (struct sockaddr *)&server_v, sizeof(server_v));
				if (conn_result == -1)
				{
					perror("connect");
				    exit(1);
				}
				if (write(server_v_fd, &green_pass, sizeof(green_pass)) != sizeof(green_pass))
				{
					perror("write");
				    exit(1);
				}
				printf("Il numero di tessera è stato inviato con successo!\n");
				printf("In attesa di risposta dal server V . . . \n");
				int r = read(server_v_fd, &green_pass, sizeof(green_pass));
				if (r == -1)
				{
					perror("read");
					exit(1);
				}
				else if (green_pass.numero_operazione == ERRORE_GENERICO)
				{
					printf("Errore durante le operazioni in server v\n");
					exit(1);
				}
				
				printf("Tessera Sanitaria: %s\n", green_pass.numero_identificazione_tessera);
				printf("Scadenza: %.24s\r\n", ctime(&green_pass.periodo_validita));
				if (write(conn_fd, &green_pass, sizeof(green_pass)) < sizeof(green_pass))
				{
					perror("write");
					exit(1);
				}
				printf("Risposta inviata al client\n");
				close(server_v_fd);
				close(conn_fd);
			    exit(0);
			}
			exit(0);
		}
		else
		{
			close(conn_fd);
		}
	}
}

int main(int argc, char **argv)
{
	
	printf("#### Avviato il server Centro vaccinale ####\n");

	int socket_fd = init_socket(C_VACCINALE_PORT);
	
	socket_loop(socket_fd, SERVER_V_HOST);

	printf("#### Programma terminato ####\n");

}
