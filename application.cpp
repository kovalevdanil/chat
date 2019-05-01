#include <iostream>

#include <string.h>

#include <thread>
#include <mutex>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#include <fcntl.h>

std::mutex sendrecv_mute;

void send_proc(int sockfd)
{
    char buffer[1024];
    while (1)
    {
        std::cin.getline(buffer, 1024);
        sendrecv_mute.lock();
        size_t size = strlen(buffer);
        buffer[size++] = '\n';
        buffer[size] = '\0';
        send(sockfd, buffer, size, MSG_NOSIGNAL);
        sendrecv_mute.unlock();
    }
}

int main(int argc, char **argv)
{
    char *IP, *PORT;
    if (argc == 2)
    {
        IP = argv[1];
        PORT = NULL;
    }
    else
    {
        IP = new char[INET_ADDRSTRLEN];
        memcpy(IP, "127.0.0.1", strlen("127.0.0.1") + 1);
        PORT = new char[5];
        memcpy(PORT, "3490", strlen("3490") + 1);
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    struct addrinfo hints, *res;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    getaddrinfo(IP, PORT, &hints, &res);

    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1)
    {
        std::cerr << "application: connection failed (" << gai_strerror(errno) << ")\n";
        exit(1);
    };

    std::thread send_proccess(send_proc, sockfd);

    send_proccess.detach();

    while (1)
    {
        char buffer[1024];
        int RecvSize = recv(sockfd, buffer, sizeof(buffer), MSG_NOSIGNAL);
        if (RecvSize == -1 || RecvSize == 0)
        {
            shutdown(sockfd, SHUT_RDWR);
            close(sockfd);
            std::cout << "server: connection lost\n";
            exit(2);
        }
        else
        {
            buffer[RecvSize] = '\0';
            std::cout << buffer;
        }
    }
}