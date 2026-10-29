//Com o código server.c pronto acho que já tenho uma base para criar um socket aqui e conseguir me comunicar com o servidor então vou me desafiar um pouco aqui seguindo o código server

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"
#define _WIN32_WINNT 0x0600

#include<ws2tcpip.h> 
#include<stdio.h>
#include<winsock2.h> 
#include<windows.h>
#include<winbase.h>

struct addrinfo *result=NULL, *ptr=NULL, hints;

int main(int argc, char **argv){
    (void)argc;
    WSADATA wsaData;

    int iWsaResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iWsaResult != 0) {                                  
        printf("WSAStartup failed with error: %d\n", iWsaResult);
        WSACleanup();
        return 1;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM; 
    hints.ai_protocol = IPPROTO_TCP; 

    int iAiResult = getaddrinfo(argv[1], DEFAULT_PORT, &hints, &result);
    if (iAiResult != 0) { 
        printf("getaddrinfo failed: %d\n", iAiResult);
        WSACleanup();
        return 1;
    }

    SOCKET ConnectSocket = INVALID_SOCKET;
    ptr=result;

    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }

    int iConnectResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iConnectResult == SOCKET_ERROR) {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET) {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }

    int recvbuflen = DEFAULT_BUFLEN;

    const char *sendbuf = "this is a test"; //isso aqui tem minha fiel felicidade porque agora tá funcionando perfeitamente
    char recvbuf[DEFAULT_BUFLEN];

    int iResult;

    iResult = send(ConnectSocket, sendbuf, (int) strlen(sendbuf), 0);
    if (iResult == SOCKET_ERROR) {
        printf("send failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    printf("Bytes Sent: %d\n", iResult);

    iResult = shutdown(ConnectSocket, SD_SEND);
    if (iResult == SOCKET_ERROR) {
        printf("shutdown failed: %d\n", WSAGetLastError());
        closesocket(ConnectSocket);
        WSACleanup();
        return 1;
    }

    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, 0);
        if (iResult > 0)
            printf("Bytes received: %d\n", iResult);
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed: %d\n", WSAGetLastError());
    } while (iResult > 0);
}