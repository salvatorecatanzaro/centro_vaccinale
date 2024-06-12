#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <time.h>
#include "utils.h"

void stampa_operazione(int numero_operazione){
  if (numero_operazione == VERIFICA_GREEN_PASS)
  {
    printf("L'operazione selezionata è : Verifica green pass\n");
    
  }
  else if (numero_operazione == RIPRISTINO_GREEN_PASS)
  {
    printf("L'operazione selezionata è : Ripristino green pass\n");
  }
  else if (numero_operazione == INVALIDAMENTO_GREEN_PASS){
    printf("L'operazione selezionata è : Invalidamento green pass\n");
  }
  else{
    printf("L'operazione con valore %d non è ammissibile\n", numero_operazione);
  }
}

void socket_loop(int socket_fd){
  int sockfd;
  int n; 
  int requestfd;
  struct payload_green_pass green_pass;
  struct sockaddr_in servaddr;
  struct sockaddr_in requestaddr;
  pid_t pid;
  
  
  for (;;)
  {
    printf("In attesa di connessioni . . . \n");
    sockfd = accept(socket_fd, (struct sockaddr *)NULL, NULL);
    if(sockfd == -1)
    {
      perror("accept");
      exit(1);
    }

    if ((pid = fork()) < 0)
    {
      perror("fork error");
      exit(-1);
    }

    if (pid == 0)
    {
      close(socket_fd);
	    printf("Connessione riuscita, il processo che la gestisce ha pid %ld\n", (long)getpid());

      // ricevo greenPass e richiesta servizio da client
      while((n = read(sockfd, &green_pass, sizeof(green_pass))) > 0)
      {
        if (n < 0)
        {
          perror("read");
          exit(1);
        }
        if(strlen(green_pass.numero_identificazione_tessera) != 20)
        {
          printf("Errore, il numero di tessera deve essere uguale a 20\n");
          green_pass.numero_operazione = ERRORE_GENERICO;
          if (write(sockfd, &green_pass, sizeof(green_pass)) < sizeof(green_pass))
          {
            perror("write");
            exit(1);
          }
          printf("Risposta inviata al client\n");
          exit(1);
        }
        green_pass.numero_identificazione_tessera[21] = 0;

        printf("Tessera Sanitaria: %s\n", green_pass.numero_identificazione_tessera);

        stampa_operazione(green_pass.numero_operazione);

        requestfd = socket(AF_INET, SOCK_STREAM, 0);
        if(requestfd == -1){
          perror("socket");
          exit(1);
        }

        requestaddr.sin_family = AF_INET;
        requestaddr.sin_port = htons(SERVER_V_PORT);

        if (inet_pton(AF_INET, SERVER_V_HOST, &requestaddr.sin_addr) < 0)
        {
          perror("inet_pton");
          exit(1);
        }

        printf("Tentativo di connessione verso server v . . .\n");
        int connect_res = connect(requestfd, (struct sockaddr *)&requestaddr, sizeof(requestaddr));
        if(connect_res == -1)
        {
          perror("connect");
          exit(1);
        }

        if (write(requestfd, &green_pass, sizeof(green_pass)) != sizeof(green_pass))
        {
          perror("write");
          exit(1);
        }

        printf("Dati inviati con successo!\n");        
        
        printf("In attesa di risposta dal server V . . . \n");
        int r = read(requestfd, &green_pass, sizeof(green_pass));
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
        if (write(sockfd, &green_pass, sizeof(green_pass)) < sizeof(green_pass))
        {
          perror("write");
          exit(1);
        }
				printf("Risposta inviata al client\n");

        close(sockfd);
        close(requestfd);
        exit(0);
      }
    }
  }
}


int main(int argc, char **argv)
{

  printf("#### Avviato il server G ####\n");

  int socket_fd = init_socket(SERVER_G_PORT);

  socket_loop(socket_fd);

  close(socket_fd);

  printf("#### Programma terminato ####\n");
  return 0;
}
