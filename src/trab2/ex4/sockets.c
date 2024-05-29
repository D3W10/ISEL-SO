#include "sockets.h"

#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>

int tcp_socket_server_init(int serverPort) {
    int sockfd;
    struct sockaddr_in serv_addr;

    // Criar sockect TCP
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        return -1;
    }

    // Registar endereco local de modo a que os clientes nos possam contactar
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(serverPort);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        close(sockfd);
        return -1;
    }

    // Activar socket com fila de espera de dimensao 5
    if (listen(sockfd, 5) == -1) {
        close(sockfd);
        return -1;
    }

    return sockfd;
}

int tcp_socket_server_accept(int serverSocket) {
    struct sockaddr_in client_addr;
    socklen_t dim_client = sizeof(client_addr);

    int newsockfd = accept(serverSocket, (struct sockaddr *)(&client_addr), &dim_client);
    if (newsockfd < 0 )
        return -1;

    return newsockfd;
}

int tcp_socket_client_init(const char *host, int port) {
    int sockfd;
    in_addr_t serverAddress;
    struct hostent *phe;
    struct sockaddr_in serv_addr;

    // Determina endereco IP do servidor
    if ((phe = gethostbyname(host)) != NULL) {
        memcpy(&serverAddress, phe->h_addr_list[0], phe->h_length);
    }
    else if ((serverAddress = inet_addr(host)) == -1) {
        return -1;
    }

    // Abrir um socket TCP (an Internet Stream socket)
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        return -1;
    }

    // Preencher a estrutura serv_addr com o endereco do servidor que pretendemos contactar
    memset((char *)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = serverAddress;
    serv_addr.sin_port = htons(port);

    // Ligar-se ao servidor
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        return -1;
    }
     
    return sockfd;
}

int un_socket_server_init(const char *serverEndPoint) {
    int sockfd;
    struct sockaddr_un serv_addr;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return -1;

    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, serverEndPoint);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        return -1;

    if (listen(sockfd, 5) < 0)
        return -1;

    return sockfd;
}

int un_socket_server_accept(int serverSocket) {
    struct sockaddr_un cli_addr;
    socklen_t clilen = sizeof(cli_addr);

    int newsockfd = accept(serverSocket, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
        return -1;

    return newsockfd;
}

int un_socket_client_init(const char *serverEndPoint) {
    int sockfd;
    struct sockaddr_un serv_addr;

    if ((sockfd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
        return -1;

    memset((char*)&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, serverEndPoint);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        return -1;

    return sockfd;
}

/**
 * readn
 *
 * Faz a leitura de count bytes
 *
 * Em caso de sucesso devolve count bytes.
 * Em caso de erro devolve:
 *   - valor < 0 se ocorreu de leitura ficando a variável errno com o
 *     respetivo código de erro
 *   - 0 se foi identificado EOF
 */
int readn(int fd, void *buf, size_t count)
{
    char *ptbuf = buf;
    size_t bytesToReceive = count;

    while (bytesToReceive > 0)
    {
        int nBytesRD = read(fd, ptbuf, bytesToReceive);
        if (nBytesRD <= 0)
            return nBytesRD;
        ptbuf += nBytesRD;
        bytesToReceive -= nBytesRD;
    }
    return count;
}

/**
 * writen
 *
 * Escreve count bytes realizando o nº de escritas necessárias
 *
 * Em caso de erro devolve um valor <= 0, caso contrário devolve o
 * valor count
 */
int writen(int fd, const void *buf, size_t count)
{
    int nBytesWR;
    const char *ptbuf = buf;
    size_t bytesToSend = count;

    while (bytesToSend > 0)
    {
        nBytesWR = write(fd, ptbuf, bytesToSend);
        if (nBytesWR <= 0)
            return nBytesWR;
        ptbuf += nBytesWR;
        bytesToSend -= nBytesWR;
    }
    return count;
}
