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

// Bibliotecas para uso de barramento CAN por dispositivo Linux
#include <linux/can.h>
#include <linux/can/raw.h>

#define n_pernas 6 // Número de pernas do robô formiga
#define n_char 2 // Número de caracteres da mensagem CAN. Cada caractere possui 8 bits
#define CAN_send 1 // Variável de controle para habilitar o envio de mensagens por barramento CAN
#define periodo 2 // Período de cada movimento do robô em segundos

// Define variáveis globais de socket CAN caso a variável de controle p/ envio de mensagens por barramento CAN seja definida
#ifdef CAN_send
	int s;
	struct sockaddr_can addr;
	struct ifreq ifr;
	struct can_frame frame;
#endif

/*----------------------------------------------------------------------------------------*/

void mensagem_perna(int msg[n_char])
	
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
			printf("\nCaracteres enviados para a mbed: ");		
	
			for(unsigned int i = 0; i < n_char; i++)
			{
				printf("%b ", msg[i]);
			}
		}
	#endif
}

/*----------------------------------------------------------------------------------------*/

void envia_movimento(int deslocamento)
{
	int msg[n_char];
	int bit; // Posição do primeiro bit da dupla de variáveis para cada perna
	int byte; // Número do caractere CAN com as configurações das pernas
	
	for(unsigned int i = 0; i < n_char; i++)
	{
		msg[i] = 0;
	}
	
	for (unsigned int perna = 1; perna <= n_pernas; perna++)
	{		
		// bit = 0, 2, 4...
		if(perna <= 4)
		{
			bit = 2*(perna - 1);
			byte = 0;
		}
		else
		{
			bit = 2*(perna - 5);
			byte = 1;
		}
		
		if(deslocamento == 0)
		{
			msg[byte] = msg[byte] & !(0xA0 >> bit);
			printf("\nRegistrando 0 no bit %d do byte %d. Mensagem: %d", bit, byte, msg[byte]); // Perva vai para trás
		}
		else
		{
			msg[byte] = msg[byte] | (0xA0 >> bit);
			printf("\nRegistrando 1 no bit %d do byte %d. Mensagem: %d", bit, byte, msg[byte]); // Perna vai para frente
		}		
		
		bit++; // bit = 1, 3, 5...		
		
		if(perna % 2 == 0) // Se número da perna for par, perna levanta
		{
			msg[byte] = msg[byte] | (0xA0 >> bit);
			printf("\nRegistrando 1 no bit %d do byte %d. Mensagem: %d", bit, byte, msg[byte]);
		}
		else // Se número da perna for ímpar, perna abaixa
		{
			msg[byte] = msg[byte] & !(0xA0 >> bit);
			printf("\nRegistrando 0 no bit %d do byte %d. Mensagem: %d", bit, byte, msg[byte]);
		}
				
	}
	mensagem_perna(msg);
}

/*----------------------------------------------------------------------------------------*/

// Diferentes estados do módulo Toradex
enum estados{enviar,aguardar,sair}estado;

/*----------------------------------------------------------------------------------------*/

int main(int argc, char **argv) // Programa principal
{
	char opcao[32];	
	estado = enviar;
	clock_t t_wait = clock(); // Tempo inicial
	
	/* Variável para indicar se o movimento do robô deve continuar ou parar.
	Se parar = 0, robô deve continuar movimento.
	Se parar = 1, robô deve parar.*/
	int parar = 0;
	
	/*----------- Abrindo socket CAN -------------*/
	#ifdef CAN_send		
		
		if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0)
		{
			perror("Socket opening failed");
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

	// Máquina de estados do robô em loop
	while(1)
	{
		switch(estado)
		{
			case enviar:
				printf("\nInsira comando de deslocamento para o robo (frente/tras/sair)\n");
				scanf( "%s", opcao);
				
				if((strcmp(opcao, "frente")) == 0)
				{
					envia_movimento(1);
					estado = aguardar;
					t_wait = clock();
					parar = 0;
					printf("\nAndando para frente\n");
				}
				else if((strcmp(opcao, "tras")) == 0)
				{
					envia_movimento(0);
					state = aguardar;
					t_wait = clock();
					parar = 0;
					printf("\nAndando para tras\n");
				}
				else if((strcmp(opcao, "sair")) == 0)
				{
					estado = sair;
				}
				else 
				{
					printf("\nComando invalido\n");
				}
				
				break;
				
			// Neste instante, o clock() marca um tempo posterior a t_wait
			case aguardar:
				if( ( (clock() - t_wait) / CLOCKS_PER_SEC ) > periodo )
				{
					parar = 1;
				}

				if(parar == 1)
				{
					estado = enviar;
				}
				
				break;

			case sair:
				exit(0);
				break;
		}
	}
		
	#ifdef CAN_send
		if(close(s) < 0)
		{
			perror("Close failed");
			return 1;
		}
	#endif

	printf("\nFim do processo\n");
	return 0;
}
