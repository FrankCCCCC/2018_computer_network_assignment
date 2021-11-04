#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <stdbool.h>
#include <string.h>

#define MAX_SIZE 2048
#define MY_ERROR(s) printf(s); system("PAUSE"); exit(1);
#define SERVER_PORT 9999
const char RA[] = ">~$readArti";
const char SA[] = ">~$selectArti";
const char UA[] = ">~$uploadArti";
const char EOFA[] = ">~$EOF";

SOCKET serverSocket;
struct sockaddr_in serverAddress;
// call WSAStartup first for Winsock
WSADATA wsadata;

int arrowSelecct(int listLen){
	int idx = 0, ch1=0, ch2 = 0;
	while(1){
		if(kbhit()){
			ch1 = getch();
			if(ch1 == 224){
				ch2 = getch();
				if(ch2 == 72){idx--;}
				if(ch2 == 80){idx++;}
			}else if(ch1 == 13){
				if(idx >= 0){idx = idx%6;}
				else{idx=0-idx; idx=listLen-idx%6;}
				//printf("%d\n", idx);
				return idx;
			}
		}
	}
}
void chooseIPEstabl(){
	printf("Which one do you want to connect to?\n");
	printf(" [1]Local Host 127.0.0.1\n");
	printf(" [2]Type an IP\n");

	if( WSAStartup(MAKEWORD(2,2),(LPWSADATA)&wsadata) != 0) {
		MY_ERROR("Winsock Error\n");
		//return false;
	}
	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(SERVER_PORT); //converts a u_short from host to TCP/IP network byte order

	while(1){
		int idx = 1;
		fflush(stdin);
		scanf("%d", &idx);
		printf("%d\n", idx);
		if(idx == 1){
			fflush(stdin);
			serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1"); // transform to 32-bit unsigned integer
			printf("Local Host Address Setted Up.\n");
			return ;
		}else if(idx == 2){
			fflush(stdin);
			printf("Please Enter Your IP.\n");
			char ip[30];
			scanf("%s", ip);
			serverAddress.sin_addr.s_addr = inet_addr(ip); // transform to 32-bit unsigned integer
			fflush(stdin);
			printf("Address Setted Up.\n");
			return ;
		}else{
			printf("Please Select a Connecting Way\n");
		}
	}
}

void readArti(){
	int bytesRead = 0;
	char buff[MAX_SIZE];
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(connect(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress))<0){MY_ERROR("Connect Error\n");}

	//Send a "Read Aticle" Request
	send(serverSocket, RA, 11, 0);

	//Receive "Server Start Read Artical"
	bytesRead = recv(serverSocket, buff, MAX_SIZE, 0);
	send(serverSocket, "200", 3, 0);
	printf("%s", buff);
	memset(buff, 0, sizeof(buff));
	if(strcmp(buff, "Server Fail to Open Data") == 0){return;}

	//Receive How Many Articles
	int artiNum = 0;
	bytesRead = recv(serverSocket, buff, MAX_SIZE, 0);
	send(serverSocket, "200", 3, 0);
	artiNum = atoi(buff);
	printf("%d\n", artiNum);
	memset(buff, 0, sizeof(buff));

	//Receive Each Article Title
	bytesRead = recv(serverSocket, buff, MAX_SIZE, 0);
	printf("%s", buff);
	closesocket(serverSocket);

	memset(buff, 0, sizeof(buff));

	//Select Article to read
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(connect(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress))<0){MY_ERROR("Connect Error\n");}

	char input[20];
	int selectIdx = 0;
	while(1){
		printf("Please Input The Index of Article that Wanted to Read.\n");
		scanf("%s", input);
		selectIdx = atoi(input);
		if(strcmp(input, "back") == 0){return ;}
		else if(selectIdx > artiNum || selectIdx <= 0){ printf("Please Input %d ~ 1\n", artiNum);}
		else if(selectIdx <= artiNum || selectIdx > 0){ printf("Read Articl [%d]\n\n", selectIdx); break;}
		else{printf("Please Input Correct Command\n");}
		memset(input, 0, 20);
		selectIdx = 0;
	}
	selectIdx--;
	char idxStr[20];
	itoa(selectIdx, idxStr, 10);
	send(serverSocket, SA, 13, 0);//Command
	printf("Send SA\n");
	recv(serverSocket, buff, MAX_SIZE, 0);//Receive "Wait For Selected Article"
	printf("%s\n", buff);
	memset(buff, 0, MAX_SIZE);
	send(serverSocket, idxStr, sizeof(int), 0);//Send Article Title Index

	//Transfer FILE
	char content[100000];
	int i=0;
	printf("\n---------------------------CONTENT------------------------\n");
	while(1){
		recv(serverSocket, buff, MAX_SIZE, 0);
		if(strcmp(buff, EOFA) == 0){memset(buff, 0, sizeof(buff));break;}
		strcat(content, buff);
		printf(">%s", buff);
		i++;
		memset(buff, 0, sizeof(buff));
	}
	if(i==0){printf("The Article is Empty or Not Found.\n");};
	printf("\n------------------------END OF CONTENT---------------------\n");
	closesocket(serverSocket);
}

void uploadArti(){
	//Establish Socket
	serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	if(connect(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress))<0){MY_ERROR("Connect Error\n");}

	char uploadFile[50] = "Cat.txt";
	FILE* fUp = NULL;
	printf("Please Input the File Name You Want to Upload.\n");
	scanf("%s", uploadFile);

	//fgets(uploadArti, 50, stdin);
	fUp = fopen(uploadFile, "r+");
	if(fUp == NULL){
		printf("Fail to Open File\n");
	}else{
		//Send a "Upload Aticle" Request
		char buff[MAX_SIZE] = {0};
		send(serverSocket, UA, 13, 0);
		recv(serverSocket, buff, MAX_SIZE, 0);
		printf("%s\n", buff);
		if(strcmp(buff, "Server is Ready to Receive Title") == 0){
			memset(buff, 0, sizeof(buff));
			//Upload Title
			send(serverSocket, uploadFile, 13, 0);
			recv(serverSocket, buff, MAX_SIZE, 0);
			printf("%s\n", buff);
			if(strcmp(buff, "Start Upload Article.") == 0){
				memset(buff, 0, sizeof(buff));
				//Read Content
				char contentBuff[MAX_SIZE] = {0};
				for(int i=0; i<100000/MAX_SIZE; i++){
					if(fgets(contentBuff, MAX_SIZE, fUp) != NULL){
						printf("%s\n", contentBuff);
						send(serverSocket, contentBuff, MAX_SIZE, 0);
						memset(contentBuff, 0, MAX_SIZE);
					}else{break;}
					send(serverSocket,EOFA, MAX_SIZE, 0);
				}
			}else{
				printf("Cannot Upload Article\n");
			}
		}else{
			printf("Cannot Upload Title\n");
		}
	}
	fclose(fUp);
}
void initLayout(){
	printf("Hello!! Wellcome to BBS\n");
	printf("-------------------------------------------------------------------------------------------\n");
}

int main(int argc, char** argv) {
	initLayout();
	chooseIPEstabl();

	char cmd[20] = {0};
	while(1) {
		printf("Please Input \"read\" or \"upload\" for Reading Articles or Upload an Article\n");
		scanf("%s", cmd);
		if(strcmp(cmd, "read") == 0){readArti();}
		else if(strcmp(cmd, "upload") == 0){uploadArti();}
	}
	/*
	int bytesRead;
	char buf[MAX_SIZE];

	while(1) {
		printf("Please Input Message\n");
		scanf("%s", buf);

		serverSocket = socket(PF_INET, SOCK_STREAM, 0);
		if(connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress))<0){
			MY_ERROR("Connect Error\n");
		}
		send(serverSocket, buf, strlen(buf), 0);
		bytesRead = recv(serverSocket, buf, MAX_SIZE, 0);
		buf[bytesRead] = '\0';
		if(bytesRead>0) printf("Receive %d byte(s): %s\n", bytesRead, buf);
		closesocket(serverSocket);
	}*/

	return 0;
}
