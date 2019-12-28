#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#include<netinet/in.h>
#include "../central.h"
#define BUFFER_SIZE 350

int main(int argc, char** argv)
{
	int bob, alice, len, input, valid_inp = -1, con;
	pid_t pid;
	
	FILE *ciph, *hash, *rndm, *bob_c, *enc_mes, *alabel, *blabel, *initvec;
	alabel = fopen("alabel.txt", "r+");
	blabel = fopen("blabel.txt", "r+");
		
	//variables for reading encryptions from the file "ciph.txt"
	char *e1, *e2, *e3, *e4, *h_op0, *h_op1, *iniv; //h_op0 is the hashed encrypted output 0
	int n1, n2, n3, n4, *l1, *l2, *l3, *l4;
	l1 = (int*)malloc(sizeof(int));
	l2 = (int*)malloc(sizeof(int));
	l3 = (int*)malloc(sizeof(int));
	l4 = (int*)malloc(sizeof(int));


	//variables keys
	char *a_0, *b_0, *a_1, *b_1, *x_0, *x_1, *c; 
	a_0 = (char*) malloc(350*sizeof(char));
	b_0 = (char*) malloc(350*sizeof(char));
	a_1 = (char*) malloc(350*sizeof(char));
	b_1 = (char*) malloc(350*sizeof(char));
	x_0 = (char*) malloc(350*sizeof(char));
	x_1 = (char*) malloc(350*sizeof(char));
	c = (char*) malloc(350*sizeof(char));



	//Creation of socket Alice
	alice = socket(AF_INET, SOCK_STREAM, 0);


	//Setting necessary values for the server
	struct sockaddr_in server_alice;
	struct sockaddr_in server_bob;
	server_alice.sin_family=AF_INET;
	server_alice.sin_port=htons(atoi(argv[1]));
	server_alice.sin_addr.s_addr=INADDR_ANY;
	bzero(&server_alice.sin_zero, 8);


	//Binding socket Alice to the server values set for alice_server
	if((bind(alice, (struct sockaddr*)&server_alice, sizeof(struct sockaddr_in))) == -1){
		printf("Server binding unsuccessful!\n");
		exit(-1);
	}


	//Server listens to just one connection at a time
	if((listen(alice, 1)) == -1){
		printf("Listen unsuccessful!\n");
		exit(-1);
	}
	else
		printf("Alice is listening...\n");


	//len contains size of structure sockaddr_in
	len=sizeof(struct sockaddr_in);


	//Accepts incoming connection request
	if((bob=accept(alice, (struct sockaddr *)&server_bob, &len)) < 0){
		printf("Unable to accept connection!\n");
		exit(-1);
	}
	else
		printf("Alice has accepted a connection...\n");


	//Buffer is used to send and recieve messages
	char *buffer;
	buffer = (char*)malloc(BUFFER_SIZE*sizeof(char));

/*********************************************************/
	
	//Fork to execute al_gate
	pid = fork();

	//child process
	if(pid == 0)
	{
		//child process al_gate
		static char *argv[] = {"al_gate", NULL};
		execv("al_gate", argv);
		exit(-1);
	}

	//Waits for al_gate to finish
	wait(NULL);


/**********************************************************/

	printf("Sending initialisation vector...\n");

	iniv = (char*)malloc(20*sizeof(char));
	initvec = fopen("initvec.txt", "r");
	fscanf(initvec, "%s", iniv);
	send(bob, iniv, strlen(iniv), 0);    //A0
	bzero(iniv, strlen(iniv));
	

/*********************************************************/

	printf("Sending the garbled table...\n");

	//Send garbled table generated by alice_gate to Bob
	
	/*************here, we send e1, e2, e3, e4 in the same order****************************************************/

	ciph = fopen("ciph.txt", "r");
	e1 = (char*)malloc(CIPH_LENGTH*sizeof(char));
	for (int i = 0; i < CIPH_LENGTH; ++i)
	{
		e1[i] = fgetc(ciph);
	}
	send(bob, e1, CIPH_LENGTH, 0);   //A1

	e2 = (char*)malloc(CIPH_LENGTH*sizeof(char));
	for (int i = 0; i < CIPH_LENGTH; ++i)
	{
		e2[i] = fgetc(ciph);
	}
	send(bob, e2, CIPH_LENGTH, 0);   //A2
		
	e3 = (char*)malloc(CIPH_LENGTH*sizeof(char));
	for (int i = 0; i < CIPH_LENGTH; ++i)
	{
		e3[i] = fgetc(ciph);
	}
	send(bob, e3, CIPH_LENGTH, 0);   //A3

	e4 = (char*)malloc(CIPH_LENGTH*sizeof(char));
	for (int i = 0; i < CIPH_LENGTH; ++i)
	{
		e4[i] = fgetc(ciph);
	}
	send(bob, e4, CIPH_LENGTH, 0);   //A4


	fflush(ciph);
	fclose(ciph);
	printf("Alice has sent garbled table to Bob.\n");


	//Sending the hashes of op_0 and op_1 to Bob

	hash = fopen("hash.txt", "r");
	h_op0 = (char*)malloc(HASH_LENGTH*sizeof(char));
	for (int i = 0; i < HASH_LENGTH; ++i)
	{
		h_op0[i] = fgetc(hash);
	}
	send(bob, h_op0, HASH_LENGTH, 0);    //A5

	h_op1 = (char*)malloc(HASH_LENGTH*sizeof(char));
	for (int i = 0; i < HASH_LENGTH; ++i)
	{
		h_op1[i] = fgetc(hash);
	}
	send(bob, h_op1, HASH_LENGTH, 0);     //A6

	printf("Sending the hashes of encrypted outputs...\n");

	fclose(hash);

/**************************************************************************************************************/

	printf("Sending keys to Bob...\n");

	//Sending Bob Alice's input labels
	while(valid_inp == -1)
	{
		printf("Enter your input for gate-1: ");
		scanf("%d", &input);	
		fscanf(alabel, "%s", a_0);
		fscanf(alabel, "%s", a_1);
		fscanf(blabel, "%s", b_0);
		fscanf(blabel, "%s", b_1);
		
		if(input == 0)
		{
			send(bob, a_0, LABEL_LENGTH, 0);    //A7
			valid_inp = 1;
		}
		else if(input == 1)
		{
			send(bob, a_1, LABEL_LENGTH, 0);    //A7
			valid_inp = 1;
		}
		else{
			printf("Input can only be 1 or 0! \n");
			valid_inp = -1;
		}

	}

	fclose(alabel);
	fclose(blabel);

/***********************************Oblivious Transfer for sending b0/b1*******************************************/

	//Fork to execute alice1
	pid = fork();

	//child process
	if(pid == 0)
	{
		//Execute alice1
		static char *argv[] = {"alice1", NULL};
		execv("alice1", argv);
		exit(-1);
	}

	//Waits for execution of alice1 to finish
	wait(NULL);
	//Send randomly generated nymbers by alice1 to bob
	rndm = fopen("randomx.txt", "r");

	fscanf(rndm, "%s", x_0);
	con = htonl(strlen(x_0));
	send(bob, &con, 4, 0);
	send(bob, x_0, strlen(x_0), 0);    //A8

	fscanf(rndm, "%s", x_1);
	con = htonl(strlen(x_1));
	send(bob, &con, 4, 0);
	send(bob, x_1, strlen(x_1), 0);    //A9

	fclose(rndm);
	
	printf("Alice has sent randomly generated numbers\n");
	
	//Recieve the c generated by Bob and store in bob_c.txt
	bob_c = fopen("bob_c.txt", "w");
	int convert, n;
	recv(bob, &convert, 4, 0);
	n = ntohl(convert);
	recv(bob, c, n, 0);    //B1
	fprintf(bob_c, "%s", c);
	bzero(c, strlen(c));

	fclose(bob_c);
	printf("Alice has recieved c from Bob\n");

	//Fork to execute alice2
	pid = fork();

	//child process
	if(pid==0)
	{
		//child process alice2
		static char *argv[]={"alice2",NULL};
		execv("alice2", argv);
		exit(-1);
	}

	//Waits for alice2 to finish
	wait(NULL);

	//Send encrypted messages generated by alice2 to Bob
	enc_mes = fopen("enc_messages.txt", "r");
	fscanf(enc_mes, "%s",buffer);
	n1 = strlen(buffer);
	con = htonl(n1);
	send(bob, &con, 4, 0);
	send(bob, buffer, strlen(buffer), 0);    //A10

	fscanf(enc_mes,"%s",buffer);
	n1 = strlen(buffer);
	con = htonl(n1);
	send(bob, &con, 4, 0);
	send(bob, buffer, strlen(buffer), 0);    //A11

	fclose(enc_mes);
	printf("Encrypted messages sent to Bob\n");

	//Close connection with Bob
	close(bob);
	printf("Transfer Completed\n");	

	free(l1);
	free(l2);
	free(l3);
	free(l4);
	free(e1);
	free(e2);
	free(e3);
	free(e4);
}