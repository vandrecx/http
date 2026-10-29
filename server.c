//A ideia é construir uma comunicação cliente-servidor que rode em cima de socket com tcp para sermos capazes de conversar com HTTP 1.0 seguindo a RFC1945
//A complicação em torno disso é basicamente uma: Sistema Operacional Windows

//Tive que configurar o C/C++ Compile Run no VSCode porque #pragma não funciona porque o GCC ignora ele segundo alguns usuários do stackoverflow
//Essa configuração no C/C++ foi basicamente adicionar flag link -lws2_32

//Vou copiar exatamente o passo a passo da documentação de um servidor socket https://learn.microsoft.com/en-us/windows/win32/winsock/about-clients-and-servers
//1.Initialize a Winsock
//2.Create a Socket
//3.Bind the Socket
//4.List on the socket for a client
//5.Accept a connection from a client
//6.Receive and send data
//7.Disconnect

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define _WIN32_WINNT 0x0600 //vou pesquisar pra que isso serve mas supriu um problema de declaração implícita de freeaddrinfo() e getaddrinfo()

#include<ws2tcpip.h> //Isso substitui <netdb.h>
#include<stdio.h>
#include<winsock2.h> //versão não POSIX do sys/socket.h
#include<windows.h>
#include<winbase.h>

struct addrinfo *result=NULL, *ptr=NULL, hints; //Precisa do netdb.h, porém não existe ntivamente no windows

int main(){
    WSADATA wsaData;

    int iWsaResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iWsaResult != 0) {                                  
        printf("WSAStartup failed with error: %d\n", iWsaResult);
        //Coloquei isso aqui pelo fato de ser um processo WSAAPI, mas na ref não usa diretamente nessa exceção
        WSACleanup();
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints)); //aqui precisava do winbase.h, mas só o windows.h já cobria a necessidade; Serve pra zerar toda referência 
    hints.ai_family = AF_INET; //especifica ipv4, aparentemente AF_INET é padrão ipv4, mas se precisar do ipv6 é AF_INET6
    hints.ai_socktype = SOCK_STREAM; //conexao de dupla via, usa TCP
    hints.ai_protocol = IPPROTO_TCP; //define a conexão tcp
    hints.ai_flags = AI_PASSIVE; //n vai agir de forma ativa em enviar mensagens ou se conectar, tlvz seja importante eventualmente tlvz mudar p mandar response

    //Ai pra AddressInfo
    int iAiResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iAiResult != 0) { 
        printf("getaddrinfo failed: %d\n", iAiResult);
        WSACleanup();
        return 1;
    }

    SOCKET ListenSocket = INVALID_SOCKET;
    ListenSocket = socket(result->ai_family,result->ai_socktype,result->ai_protocol);//aqui aceita parametros na mão de acordo com a ref mas o struct result já armazena os dados

    //Trata exceção de erro na criação do socket
    if (ListenSocket == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    int iBindResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen); //qnd o socket é criado ele n tem endereço nenhum atrelado a ele e bind() serve justamente pra isso
    //Trata exceção do bind
    if (iBindResult == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    //a função listen modifica o estado do servidor par aque ele possa aceitar conexões estrangeiras pra um socket definido no parâmetro
    if ( listen( ListenSocket, SOMAXCONN ) == SOCKET_ERROR ) {
        printf( "Listen failed with error: %d\n", WSAGetLastError() );
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    SOCKET ClientSocket=INVALID_SOCKET;

    //Aceita conexão estrangeira
    ClientSocket=accept(ListenSocket, NULL, NULL);
    if (ClientSocket == INVALID_SOCKET) {
        printf("accept failed: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    closesocket(ListenSocket);

    //Buffer que vai receber as mensagens e seu tamanho
    char recvbuf[DEFAULT_BUFLEN];
    int iReceiveResult = recv(ClientSocket,recvbuf, DEFAULT_BUFLEN,0);

    //Processo de recebimento e envio de mensagens entre o ClientSocket e o ServerSocket
    do{
        if(iReceiveResult>0){ 
            printf("Bytes received: %d\n", iReceiveResult);

            int iSendResult = send(ClientSocket, recvbuf, iReceiveResult, 0);
            if(iSendResult == SOCKET_ERROR){
                printf("Send failed: %d\n", WSAGetLastError());
                closesocket(ClientSocket);
                WSACleanup();
                return 1;
            }
            printf("Bytes sent: %d\n", iSendResult);
        }else if(iReceiveResult==0){ 
            printf("Connection closing...\n");
        }else{
            printf("Receive failed: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
    } while(iReceiveResult>0);

    //Desligamento do Socket
    int iShutResult = shutdown(ClientSocket, SD_SEND);
    if(iShutResult==SOCKET_ERROR){ 
        printf("Shutdown failed: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    //Acho que finalizada a construção desse Servidor próximo passo agora é modularizar de acordo com a necessidade e prosseguir com o plano
    //-Muito código repetido, acho que uma boa modularização deve ser feita em cima do passo a passo descrito no começo
    //-Construir um HTTP em cima disso

}