#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include "utils.h"

void send_command(char *numero_tessera_sanitaria)
{
    int socket_fd;
    int n;
    struct sockaddr_in servaddr;

    printf("Il numero di identificazione della tessera sanitaria è %s\n", numero_tessera_sanitaria);

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == -1)
    {
        perror("socket");
        exit(1);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(C_VACCINALE_PORT);

    if (inet_pton(AF_INET, C_VACCINALE_HOST, &servaddr.sin_addr) < 0)
    {
        perror("inet_pton");
        exit(1);
    }

    printf("Invio dati al centro vaccinale . . .\n");

    int connect_res = connect(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (connect_res == -1)
    {
        perror("connect");
        exit(1);
    }
    if (write(socket_fd, numero_tessera_sanitaria, strlen(numero_tessera_sanitaria)) != strlen(numero_tessera_sanitaria))
    {
        perror("write");
        exit(1);
    }

    printf("Il numero di identificaione tessera %s è stato consegnato all'indirizzo %s:%d\n", numero_tessera_sanitaria, C_VACCINALE_HOST, C_VACCINALE_PORT);
    printf("Leggo il risultato. . .\n");
    struct payload_green_pass response;
    if (read(socket_fd, &response, sizeof(response)) == -1)
    {
        perror("read");
        exit(1);
    }

    if (response.numero_operazione == ERRORE_GENERICO)
    {
        printf("Il green pass è ancora valido\n");
    }
    else
    {
        printf("Operazione avvenuta con successo\n");
        printf("Tessera Sanitaria: %s\n", response.numero_identificazione_tessera);
        printf("Scadenza: %.24s\r\n", ctime(&response.periodo_validita));
    }

    close(socket_fd);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Inserire numero tessera sanitaria\n");
        exit(1);
    }

    if (strlen(argv[1]) != 20)
    {
        fprintf(stderr, "Tessera Sanitaria non valida \n");
        exit(1);
    }

    printf("#### Client A avviato ####\n");

    send_command(argv[1]);

    printf("#### Programma terminato ####\n");

    return 0;
}
