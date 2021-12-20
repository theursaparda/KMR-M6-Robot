#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

// Bibliotecas para manipulação de sockets
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

// Bibliotecas para uso de barramento CAN por dispositivo com sistema operacional Linux
#include <linux/can.h>
#include <linux/can/raw.h>

#define n_pernas 6 // Número de pernas do robô-formiga

/* Número de caracteres da mensagem de controle das pernas robóticas a ser enviada por CAN.
Cada caractere possui 8 bits, sendo necessários, no mínimo, (6 pernas)*(2 bits/perna) = 12 bits
para controlar o deslocamento das pernas do robô-formiga. Sendo assim, serão utilizados 2 caracteres para
transmissão da mensagem. */
#define n_char 2

#define CAN_send 1 // Variável de controle para habilitar o envio de mensagens por barramento CAN
#define periodo_robo 5 // Tempo de movimento do robô antes de parar, em segundos

// Variáveis globais de criação de socket para comunicação via CAN
#ifdef CAN_send
	int s;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame;
#endif

/*----------------------------------------------------------------------------------------*/

void mensagem_CAN(int msg[n_char])
	
	#ifdef CAN_send	
		sprintf(frame.data, msg); // Envio da mensagem
		
		// Verifica se houve erro no envio da mensagem
		if(write(s, &frame, sizeof(struct can_frame)) != sizeof(struct can_frame))
		{
			perror("Write failed");
			return 1;
		}
	    else
		{
			printf("\nCaracteres enviados para a mbed:\n");		
	
			for(unsigned int i = 0; i < n_char; i++)
			{
				printf("\n%b\n", msg[i]); // Imprime caracteres em binário
			}
		}
	#endif
}

/*----------------------------------------------------------------------------------------*/

void executa_movimento(int deslocamento, int K)
{
	int msg[n_char] = {0};
	int bit; // Posição do bit da dupla de variáveis para cada perna
	int byte = 0; // Número do caractere CAN com as configurações das pernas
	
	while(byte < n_char)
	{
		for (unsigned int perna = 1; perna <= n_pernas + 1; perna++)
		{
			if(perna > 4)
			{
				byte++;
			}				
			else if(perna > n_pernas)
			{
				byte++;
				break;
			}
			
			if(perna <= 4)
			{
				bit = 2*(perna - 1); // bit = 0, 2, 4, 6
			}
			else
			{
				bit = 2*(perna - 5); // bit = 0, 2
			}
			
			if(perna % 2 == 0 && !K) // Se número da perna for par e flag K for false, perna levanta
			{
				msg[byte] = msg[byte] | (0xA0 >> bit);
				printf("\nRegistrando 1 no bit %d do byte %d. Mensagem: %b\n", bit, byte, msg[byte]);
			}
			else if(perna % 2 == 0 && K) // Se número da perna for par e flag K for true, perna abaixa
			{
				msg[byte] = msg[byte] & !(0xA0 >> bit);
				printf("\nRegistrando 0 no bit %d do byte %d. Mensagem: %b\n", bit, byte, msg[byte]);
			}
			else if(perna % 2 != 0 && !K) // Se número da perna for ímpar e flag K for false, perna abaixa
			{
				msg[byte] = msg[byte] & !(0xA0 >> bit);
				printf("\nRegistrando 0 no bit %d do byte %d. Mensagem: %b\n", bit, byte, msg[byte]);
			}
			else // Se número da perna for ímpar e flag K for true, perna levanta
			{
				msg[byte] = msg[byte] | (0xA0 >> bit);
				printf("\nRegistrando 1 no bit %d do byte %d. Mensagem: %b\n", bit, byte, msg[byte]);
			}
			
			bit++; // bit = 1, 3, 5, 7
			
			if(deslocamento == 0)
			{
				msg[byte] = msg[byte] & !(0xA0 >> bit); // Operação AND bit-a-bit
				printf("\nRegistrando 0 no bit %d do byte %d. Mensagem: %b\n", bit, byte, msg[byte]); // Robô vai para trás
			}
			else
			{
				msg[byte] = msg[byte] | (0xA0 >> bit); // Operação OR bit-a-bit
				printf("\nRegistrando 1 no bit %d do byte %d. Mensagem: %b\n", bit, byte, msg[byte]); // Robô vai para frente
			}			
		}		
	}	
	mensagem_CAN(msg);	
}

/*----------------------------------------------------------------------------------------*/

int main(int argc, char **argv) // Programa principal
{
	char opcao[7];
	clock_t t0 = clock(); // Tempo inicial
	int k = 0; // Flag de controle das pernas
	
	/*----------- Abrindo socket CAN -------------*/
	#ifdef CAN_send		
		if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
		{
			perror("Socket failed");
			return 1;
		}
		
		strcpy(ifr.ifr_name, "can0" );
		ioctl(s, SIOCGIFINDEX, &ifr);
		memset(&addr, 0, sizeof(addr));
		addr.can_family = AF_CAN;
		addr.can_ifindex = ifr.ifr_ifindex;

		if (bind(s, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		{
			perror("Bind failed");
			return 1;
		}

		frame.can_id = 0x555;
		frame.can_dlc = 5;
	#endif
	/*-----------------------------------------------*/

	// Operação do robô em loop
	while(1)
	{		
		printf("\nInsira comando de deslocamento para o robo (frente/tras/sair): ");
		scanf("%s", opcao);
		
		switch(opcao)
		{
			case "frente":
			printf("\nMovimento para frente\n");
			while( ( clock() - t0 ) / CLOCKS_PER_SEC <= periodo_robo )
			{
				executa_movimento(1, k);
				k = !k;
			}			
			t0 = clock();
			break;
			
			case "tras":
			printf("\nMovimento para tras\n");
			while( ( clock() - t0 ) / CLOCKS_PER_SEC <= periodo_robo )
			{
				executa_movimento(0, k);
				k = !k;
			}			
			t0 = clock();
			break;
			
			case "sair":
			printf("\nPrograma encerrado\n");
			exit(0); // Encerra programa
			break;
			
			default:
			printf("\nComando invalido\n");
		}
	}
		
	#ifdef CAN_send
		if(close(s) < 0)
		{
			perror("Socket closing failed");
			return 1;
		}
	#endif

	return 0;
}
