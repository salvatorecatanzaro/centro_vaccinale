#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "utils.h"

#define SEM1 "sem1"

sem_t *green_pass_file_sem;


int send_response(int conn_fd, struct payload_green_pass* tmp ){
	printf("Invio risposta . . .\n");
	int write_res = write(conn_fd, tmp, sizeof(struct payload_green_pass)) ;
	if(write_res == -1){
		perror("write");
		exit(1);
	}
	printf("Risposta inviata con successo\n");
	return write_res != sizeof(struct payload_green_pass) ? 1 : 0;
}


void handle_request(int conn_fd, FILE* green_pass_file){
	struct payload_green_pass received;
	struct payload_green_pass tmp;
	int green_pass_letti = 0;

	printf("Connessione riuscita, il processo che la gestisce ha pid %ld\n", (long)getpid());
	
	if (read(conn_fd, &received, sizeof(received)) != sizeof(received))
	{
		perror("read");
		exit(1);
	}

	if(received.numero_operazione == AGGIUNTA_GREEN_PASS){
		// Richiesta di aggiunta da Centro vaccinale
		printf("Il servizio richiesto è l'aggiunta di un Green pass\n");
		fseek(green_pass_file, 0, SEEK_SET);
		while (fread(&tmp, sizeof(struct payload_green_pass), 1, green_pass_file) == 1)
		{
			// Se esiste aggiorno il green pass
			if (strcmp(tmp.numero_identificazione_tessera, received.numero_identificazione_tessera) == 0)
			{
				printf("Il green pass esiste, aggiorno le informazioni\n");
				fseek(green_pass_file, -(sizeof(struct payload_green_pass)), SEEK_CUR);
				sem_wait(green_pass_file_sem);
				fwrite(&received, sizeof(struct payload_green_pass), 1, green_pass_file);
				sem_post(green_pass_file_sem);
				send_response(conn_fd, &received);
				printf("Tessera Sanitaria: %s aggiornata\n", received.numero_identificazione_tessera);
				printf("La nuova scadenza è: %.24s\r\n", ctime(&received.periodo_validita));
				printf("Richiesta completata con successo\n");
				printf("###########\n");
				close(conn_fd);
				exit(0);
			}
		}

		printf("Aggiungo un nuovo green pass. . .\n");
		fseek(green_pass_file, 0, SEEK_END);
		sem_wait(green_pass_file_sem);
		fwrite(&received, sizeof(struct payload_green_pass), 1, green_pass_file);
		sem_post(green_pass_file_sem);
		send_response(conn_fd, &received);
		printf("Tessera Sanitaria: %s aggiunta\n", received.numero_identificazione_tessera);
		printf("La scadenza è: %.24s\r\n", ctime(&received.periodo_validita));
		printf("Richiesta completata con successo\n");
		printf("###########\n");
		close(conn_fd);
		exit(0);
	}
	else if(received.numero_operazione == VERIFICA_GREEN_PASS){
		printf("Richiesta di verifica di un green pass\n");
		fseek(green_pass_file, 0, SEEK_SET);
		while (fread(&tmp, sizeof(struct payload_green_pass), 1, green_pass_file) == 1)
		{
			if (strcmp(tmp.numero_identificazione_tessera, received.numero_identificazione_tessera) == 0)
			{
				printf("Green pass trovato\n");
				sem_wait(green_pass_file_sem);
				int exit_code = send_response(conn_fd, &tmp);
				printf("Richiesta completata con successo\n");
				sem_post(green_pass_file_sem);
				exit(exit_code);
			}
		}
		struct payload_green_pass green_pass_error;
		green_pass_error.numero_operazione = ERRORE_GENERICO; 
		int exit_code = send_response(conn_fd, &green_pass_error);
		close(conn_fd);
		printf("Richiesta completata con successo\n");
		exit(0);
	}
	else if(received.numero_operazione == RIPRISTINO_GREEN_PASS){
		// servizio richiesto da clientT (convalida) tramite serverG
		printf("Richiesta di ripristino greenPass\n");
		fseek(green_pass_file, 0, SEEK_SET);

		while (fread(&tmp, sizeof(struct payload_green_pass), 1, green_pass_file) == 1)
		{
			if (strcmp(tmp.numero_identificazione_tessera, received.numero_identificazione_tessera) == 0)
			{
				printf("Green pass trovato, procedo con il ripristino. . .\n");
				if(tmp.numero_operazione == 3){
					tmp.periodo_validita = time(NULL) + DUE_GIORNI_SEC;
				}
				else{
					tmp.periodo_validita = time(NULL) + SEI_MESI_SEC;
				}
				printf("Green Pass convalidato\n");
				fseek(green_pass_file, green_pass_letti * sizeof(struct payload_green_pass), SEEK_SET);
				sem_wait(green_pass_file_sem);
				fwrite(&tmp, sizeof(struct payload_green_pass), 1, green_pass_file);
				sem_post(green_pass_file_sem);
				send_response(conn_fd, &tmp);
				close(conn_fd);
				exit(0);
			}
			
			green_pass_letti++;
		}

		strcpy(tmp.numero_identificazione_tessera, received.numero_identificazione_tessera);
		tmp.periodo_validita = time(NULL) + DUE_GIORNI_SEC;
		tmp.numero_operazione = 3;
		printf("Tessera Sanitaria: %s\n", tmp.numero_identificazione_tessera);
		printf("Scadenza: %.24s\r\n", ctime(&tmp.periodo_validita));
		sem_wait(green_pass_file_sem);
		fseek(green_pass_file, 0, SEEK_END);
		fwrite(&tmp, sizeof(struct payload_green_pass), 1, green_pass_file);
		sem_post(green_pass_file_sem);
		send_response(conn_fd, &tmp);
		close(conn_fd);
		exit(0);
	}
	else if(received.numero_operazione == INVALIDAMENTO_GREEN_PASS){
		printf("Richiesta di invalidamento green pass\n");
		fseek(green_pass_file, 0, SEEK_SET);

		while (fread(&tmp, sizeof(struct payload_green_pass), 1, green_pass_file) == 1)
		{
			if (strcmp(tmp.numero_identificazione_tessera, received.numero_identificazione_tessera) == 0)
			{
				tmp.periodo_validita = time(NULL) - DUE_GIORNI_SEC;
				printf("Green Pass invalidato\n");
				fseek(green_pass_file, green_pass_letti * sizeof(struct payload_green_pass), SEEK_SET);
				sem_wait(green_pass_file_sem);
				fwrite(&tmp, sizeof(struct payload_green_pass), 1, green_pass_file);
				sem_post(green_pass_file_sem);
				send_response(conn_fd, &tmp);
				close(conn_fd);
				printf("Operazione completata con successo\n");
				exit(0);
			}
			
			green_pass_letti++;
		}

		strcpy(tmp.numero_identificazione_tessera, received.numero_identificazione_tessera);
		tmp.periodo_validita = time(NULL) - DUE_GIORNI_SEC;
		printf("Tessera Sanitaria: %s\n", tmp.numero_identificazione_tessera);
		printf("Scadenza: %.24s\r\n", ctime(&tmp.periodo_validita));
		sem_wait(green_pass_file_sem);
		fseek(green_pass_file, 0, SEEK_END);
		fwrite(&tmp, sizeof(struct payload_green_pass), 1, green_pass_file);
		send_response(conn_fd, &tmp);
		sem_post(green_pass_file_sem);
	}
	else {	
		struct payload_green_pass green_pass_error;
		green_pass_error.numero_operazione = ERRORE_GENERICO; 
		printf("Scenario non previsto\n");
		send_response(conn_fd, &green_pass_error);
		exit(1);
	}

	fclose(green_pass_file);
	close(conn_fd);
	exit(0);
}


void semaphore_initializer(char *sem_name, sem_t *sem)
{
	sem_unlink(SEM1);

	green_pass_file_sem = sem_open(SEM1, O_CREAT, O_RDWR, 1);
	if (green_pass_file_sem == SEM_FAILED)
	{
		perror("sem_open");
		exit(1);
	}
}

void semaphore_cleaner(char *sem_name, sem_t *sem)
{
	printf("Chiudo semaforo aperto");
	sem_unlink(sem_name);
	sem_close(sem);
}

void socket_loop(int socket_fd, char *server_v_ip)
{
	FILE *green_pass_file = fopen(GREEN_PASS_FILE_NAME, "rb+");
	int conn_fd = -1;
	pid_t pid;

	while (1)
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
			perror("fork error");
			exit(-1);
		}
		if (pid != 0)
		{
			// Il processo main torna in ascolto in attesa di connessioni
			close(conn_fd);
		}
		else
		{
			close(socket_fd);
			// Il processo figlio gestisce la richiesta
			handle_request(conn_fd, green_pass_file);
		}
	}
}

int main(int argc, char **argv)
{
	printf("#### Avviato il server V ####\n");

	// Inizializzo il semaforo con nome
	semaphore_initializer(SEM1, green_pass_file_sem);

	// Inizializzo una nuova socket
	int socket_fd = init_socket(SERVER_V_PORT);

	// La socket appena creata si mette in ascolto
	socket_loop(socket_fd, SERVER_V_HOST);

	// Chiudo semaforo con nome
	semaphore_cleaner(SEM1, green_pass_file_sem);

	printf("#### Programma terminato ####\n");

	return 0;
}
