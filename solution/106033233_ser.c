#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SIZE 2048
#define MY_ERROR(s) printf(s); system("PAUSE"); exit(1);
#define SERVER_PORT 9999
#define TITLELIMIT 40
const char RA[] = ">~$readArti";
const char SA[] = ">~$selectArti";
const char UA[] = ">~$uploadArti";
const char EOFA[] = ">~$EOF";
int artiNum = 0;
char** artiList = NULL;

SOCKET serverSocket, clientSocket; // create a socket
struct sockaddr_in serverAddress, clientAddress; // sockaddr_in：IP4 格式使用 ,  sockaddr_in6：IP6 格式使用 , sockaddr：通用格式
int clientAddressLen;
WSADATA wsadata;

void estbliConn(/* arguments */) {
	// call WSAStartup first for Winsock
	if( WSAStartup(MAKEWORD(2,2),(LPWSADATA)&wsadata) != 0) { // ( version of winsock )
		MY_ERROR("Winsock Error\n");
	 }

	serverSocket = socket(PF_INET, SOCK_STREAM, 0); // (address , type , protocal(0表示不強制) )

	memset(&serverAddress, 0, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(SERVER_PORT);//converts a u_short from host to TCP/IP network byte order

	if( bind(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
		MY_ERROR("Bind Error\n");
	}

	if( listen(serverSocket, 3) < 0) {
		MY_ERROR("Listen Error\n");
	}else{
		printf("Connection Successful.\n");
	}
}

int main(int argc, char** argv) {
	estbliConn();
	int bytesRead;
	char buf[MAX_SIZE];

	//Build List
	artiList = (char**)malloc(sizeof(char*)*1000);
  FILE* listfp = NULL;
	listfp = fopen("article.txt", "r");
	bool isOpen = listfp != NULL;
  fscanf(listfp, "%d\n", &artiNum);
  for(int i=0; i<artiNum ; i++){
    char* title = (char*)malloc(sizeof(char)*TITLELIMIT);
    fgets(title, TITLELIMIT, listfp);
    artiList[i] = title;
		artiList[i][strlen(title)-1] = '\0';
  }
	printf("Article Number: %d\n", artiNum);
	for(int i=0; i<artiNum ; i++){
    printf("ARTI: %s\n", artiList[i]);
  }
	fclose(listfp);
	listfp = NULL;

	while(1) {
		printf("Waiting...\n");

		clientAddressLen = sizeof(clientAddress);
		clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
		printf("Accepted\n");

		bytesRead = recv(clientSocket, buf, MAX_SIZE, 0);
		buf[bytesRead] = '\0';
		printf("Client IP is : %s \n", inet_ntoa(clientAddress.sin_addr));
		printf("Receive %d byte(s): %s\n", bytesRead, buf);


		if(strcmp(buf, RA) == 0){//Read Article List
			//FILE* fp = NULL;
	    //int artiNum = 0;
			char intToStr[5] = {0};
			char ACKBuff[3] = {0};
	    //fp = fopen("article.txt", "r+");

			//Fail to Open Data
	    if(!isOpen){printf("Fail to open data\n"); send(clientSocket, "Server Fail to Open Data", 25, 0);}
			else{printf("Server Start Read Artical\n"); send(clientSocket, "Server Start Read Artical, Article Amount: ", 43, 0);}

			//Read Article Number
	    //fscanf(fp, "%d\n", &artiNum);
	    printf("Article Number: %d\n", artiNum);
			itoa(artiNum, intToStr, 10);//integer to String
			recv(clientSocket, ACKBuff, 3, 0);
			memset(ACKBuff, 0, sizeof(ACKBuff));
			send(clientSocket, intToStr, 5, 0);

			//Transfer Article titles in a loop
			char msg[MAX_SIZE];
			memset(msg, 0, MAX_SIZE);

	    for(int i=0; i<artiNum ; i++){
				char buff[50] = " [";
				char num[5];
				char* title = (char*)malloc(sizeof(char)*TITLELIMIT);
				itoa(i+1, num, 10);
				strcat(buff, num);
				strcat(buff, "]");

	      //fgets(title, TITLELIMIT, fp);
				title = artiList[i];
				strcat(buff, title);
				strcat(buff, "\n");
				strcat(msg, buff);
				printf("%s", buff);
	    }
			recv(clientSocket, ACKBuff, 3, 0);
			memset(ACKBuff, 0, sizeof(ACKBuff));
			send(clientSocket, msg, MAX_SIZE, 0);
			memset(msg, 0, sizeof(msg));
			closesocket(clientSocket);


		}else if(strcmp(buf, UA) == 0){//Upload An Article
			//clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
			printf("Server is Ready to Receive Title\n");
			send(clientSocket, "Server is Ready to Receive Title", 33, 0);

			//Recieve Title and Create New File
			char list[] = "article.txt";
			char fileLocate[MAX_SIZE+20] = "./article/";
			char fileName[MAX_SIZE+10];
			recv(clientSocket, fileName, MAX_SIZE, 0);
			strcat(fileLocate, fileName);
			printf("%s\n", fileLocate);
			FILE* listfp = fopen(list, "w+");
			FILE* newfp = fopen(fileLocate, "w+");
			if(newfp != NULL){
				//Add 1 to Article Number
				artiNum++;
				fprintf(listfp, "%d\n", artiNum);

				//Add Title to List
				char f[MAX_SIZE+10] = {0};
				strncpy(f, fileName, strlen(fileName)-4);
				char* title = (char*)malloc(sizeof(char)*TITLELIMIT);
				strcpy(title, f);
				artiList[artiNum-1] = title;
				printf("fileName: %s\n", fileName);
				printf("f: %s\n", f);
				printf("title: %s\n", title);
				printf("artiList[artiNum]: %s\n", artiList[artiNum-1]);


				for(int i=0; i<artiNum; i++){
					fprintf(listfp, "%s\n", artiList[i]);
				}
				fclose(listfp);

				printf("Start Upload Article.\n");
				send(clientSocket, "Start Upload Article.", 25, 0);

				while(1){
					char contentBuff[MAX_SIZE];
					//clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);
					bytesRead = recv(clientSocket, contentBuff, MAX_SIZE, 0);
					if(strstr(contentBuff, EOFA) == NULL){
						fprintf(newfp, contentBuff);
						printf("%s", contentBuff);
					}else{printf("\nEnd of Uploading\n"); break;}
				}
			}else{
				printf("Unable to Upload Article.");
				send(clientSocket, "Unable to Upload Article.", 25, 0);
			}
			//
			fclose(newfp);
			fclose(listfp);

		}else if(strcmp(buf, SA) == 0){//Select An Article
			char buff[MAX_SIZE];
			memset(buff, 0, MAX_SIZE);
			//send(clientSocket, "Wait For Selected Article", 25, 0);
			send(clientSocket, "Selected Article", 25, 0);
			printf("Wait For Selected Article\n");

			//Accept Selected Article Title Index
			char articleName[MAX_SIZE] = "NotFound";
			recv(clientSocket, buff, 50, 0);//Receive Selected Article title
			int idx = atoi(buff);
			strcpy(articleName, artiList[idx]);
			printf("Select: \"%s\" ", articleName);
			memset(buff, 0, sizeof(buff));

			//Respond Content Article
			const int len = 70;
			char fileCat[5] = ".txt";
			char fileName[len];
			strcpy(fileName, "./article/");
			strcat(fileName, articleName);
			strcat(fileName, fileCat);
			FILE* fNP = NULL;
			fNP = fopen(fileName, "r+");
			if(fNP == NULL){ printf("Fail to Open File\n"); }
			else{ printf("Open \"%s\"\n", fileName);}
			char artiBuff[MAX_SIZE];

			while(fgets(artiBuff, MAX_SIZE, fNP) != NULL){
				printf("%s", artiBuff); send(clientSocket, artiBuff, MAX_SIZE, 0);
			}
			send(clientSocket, EOFA, 6, 0);
			fclose(fNP);
		}
		memset(buf, 0, sizeof(buf));

		closesocket(clientSocket);
	}

	return 0;
}
