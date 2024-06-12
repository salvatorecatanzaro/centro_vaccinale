#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include "utils.h"
#include <time.h>

void send_command(char *identificazione_tessera, int numero_op)
{
  int sockfd = -1;
  struct sockaddr_in servaddr;
  struct payload_green_pass green_pass;

  strcpy(green_pass.numero_identificazione_tessera, identificazione_tessera);
  green_pass.numero_operazione = numero_op;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1)
  {
    perror("socket");
    exit(1);
  }

  printf("Il numero di identificazione della tessera sanitaria è %s\n", identificazione_tessera);

  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(SERVER_G_PORT);

  if (inet_pton(AF_INET, SERVER_G_HOST, &servaddr.sin_addr) < 0)
  {
    printf("inet_pton error for %s\n", SERVER_G_HOST);
    exit(1);
  }

  printf("Invio dati al server G . . . \n");
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
  printf("Richiesta Inoltrata\n");
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

int main(int argc, char *argv[])
{
  char *restore = "ripristino";
  char *deactivate = "invalidamento";
  int numero_op = -1;

  if (argc < 3)
  {
    printf("Inserire numero di [tessera sanitaria] e ripristino o invalidamento\n");
    exit(1);
  }

  if (strlen(argv[1]) != 20)
  {
    printf("Tessera Sanitaria non valida \n");
    exit(1);
  }

  if (strcmp(argv[2], restore) == 0)
  {
    numero_op = RIPRISTINO_GREEN_PASS;
  }
  else if (strcmp(argv[2], deactivate) == 0)
  {
    numero_op = INVALIDAMENTO_GREEN_PASS;
  }
  else
  {
    printf("Il valore inserito non è valido, le possibili operazioni sono: \"%s\" e \"%s\" \n", restore, deactivate);
    exit(1);
  }
  printf("#### Avviato il client T ####\n");

  send_command(argv[1], numero_op);

  printf("#### Programma terminato ####\n");

  return 0;
}
