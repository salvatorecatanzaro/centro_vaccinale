#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "utils.h"


void send_command(char* numero_identificazione_t){
  int sockfd;
  struct sockaddr_in servaddr;
  struct payload_green_pass green_pass;

  printf("Il numero di identificazione della tessera sanitaria è %s\n", numero_identificazione_t);
  strcpy(green_pass.numero_identificazione_tessera, numero_identificazione_t);
  green_pass.numero_operazione = VERIFICA_GREEN_PASS;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    perror("socket");
    exit(1);
  }

  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERVER_G_PORT);

  if (inet_pton(AF_INET, SERVER_G_HOST, &servaddr.sin_addr) < 0)
  {
    printf("inet_pton error for %s\n", SERVER_G_HOST);
    exit(1);
  }

    printf("Invio dati al server G . . .\n");

  int connect_res = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  if (connect_res == -1)
  {
    perror("connect");
    exit(1);
  }

  if (write(sockfd, &green_pass, sizeof(green_pass)) != sizeof(green_pass))
  {
    perror("write");
    exit(1);
  }

  printf("Il numero di identificaione tessera %s è stato consegnato all'indirizzo %s:%d per la validazione\n", numero_identificazione_t, SERVER_G_HOST, SERVER_G_PORT);
  printf("Leggo il risultato. . .\n");
  
  if (read(sockfd, &green_pass, sizeof(green_pass)) != sizeof(green_pass))
  {
    perror("read");
    exit(1);
  }

  if (green_pass.numero_operazione == ERRORE_GENERICO)
  {
    printf("Errore durante le operazioni lato server\n");
  }
  else
  {
    printf("Operazione avvenuta con successo\n");
    printf("Tessera Sanitaria: %s\n", green_pass.numero_identificazione_tessera);
    printf("Scadenza: %.24s\r\n", ctime(&green_pass.periodo_validita));
  }
  
  close(sockfd);
}


int main(int argc, char **argv)
{
  

  if (argc < 2)
  {
    printf("Inserisci valore tessera sanitaria \n");
    exit(1);
  }

  if (strlen(argv[1]) != 20)
  {
    printf("La tessera sanitaria deve contenere 20 caratteri \n");
    exit(1);
  }

	printf("#### Client S avviato ####\n");

  send_command(argv[1]);
  
  printf("#### Programma terminato ####\n");

  return 0;
}
