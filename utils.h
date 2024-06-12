// Costanti
#define DUE_GIORNI_SEC 172800
#define SEI_MESI_SEC 15552000
#define GREEN_PASS_FILE_NAME "green_pass_list"

// lista porte
#define SERVER_V_PORT 1025
#define C_VACCINALE_PORT 1024
#define SERVER_G_PORT 1026

// lista host
#define SERVER_V_HOST "127.0.0.1"
#define SERVER_G_HOST "127.0.0.1"
#define C_VACCINALE_HOST "127.0.0.1"

// Numeri per operazioni possibili
#define AGGIUNTA_GREEN_PASS 1
#define VERIFICA_GREEN_PASS 2
#define RIPRISTINO_GREEN_PASS 3
#define INVALIDAMENTO_GREEN_PASS 4
#define ERRORE_GENERICO 500

int init_socket(int port);

struct payload_green_pass
{
	char numero_identificazione_tessera[21];
	time_t periodo_validita;
	int numero_operazione;
};