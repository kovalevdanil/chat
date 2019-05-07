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

#include "codes.h"

std::mutex stream_mute;

void insertchar(char *where, char what, int ind)
{
    int size = strlen(where);

    if (ind < 0 || ind > size)
        return;

    for (int i = size + 1; i > ind; i--)
        where[i] = where[i - 1];
    where[ind] = what;
}

int format_command(char *cmd)
{
    char *space = strchr(cmd, ' ');
    if (!space)
        *space = '\0';
    command my_cmd;

    if (!strcmp(cmd, "~online"))
        my_cmd = online;
    else if (!strcmp(cmd, "~myid"))
        my_cmd = myid;
    else if (!strcmp(cmd, "~disconnect"))
        my_cmd = disconnect;
    else if (!strcmp(cmd, "~help"))
        my_cmd = help;
    else if (!strcmp(cmd, "~chat"))
        my_cmd = chat;
    else
        error;

    if (my_cmd == chat)
    {
        if (space)
        {
            *space = '\0';
            char *second_space = strchr(space + 1, ' ');
            if (second_space)
                *second_space = '\0';
        }
    }
}

void shift(char *buffer, int num)
{
    int i = strlen(buffer) + num;
    for (; i > num - 1; i--)
        buffer[i] = buffer[i - num];
}

void send_proc(int sockfd)
{
    static char buffer[1024];
    while (1)
    {
        std::cin.getline(buffer, 1022);

        size_t size = strlen(buffer);
        buffer[size++] = '\n';
        buffer[size] = '\0';

        if (buffer[0] == '~')
        {
            if (!format_command(buffer))
            {
                std::cout << "app: wrong command.\n";
                continue;
            }
            shift(buffer, 1);
            buffer[0] = COMMAND;
            buffer[1] = ':';
        }
        else
        {
            shift(buffer, 2);
            buffer[0] = MESSAGE;
            buffer[1] = ':';
        }

        // stream_mute.lock();
        send(sockfd, buffer, size, MSG_NOSIGNAL);
        // stream_mute.unlock();
    }
}

int main(int argc, char **argv)
{
    char *IP, *PORT;
    if (argc == 2)
    {
        IP = argv[1];
        PORT = argv[2];
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
            stream_mute.lock();
            std::cout << "server: connection lost\n";
            stream_mute.unlock();
            exit(2);
        }
        else
        {
            buffer[RecvSize] = '\0';
            char code = buffer[0];
            char *p = strchr(buffer, ':') + 1;
            switch (code)
            {
            case ERROR:
                // stream_mute.lock();
                std::cout << "server: " << responses[atoi(p)];
                // stream_mute.unlock();
                break;
            case MESSAGE:
                // stream_mute.lock();
                std::cout << p;
                // stream_mute.unlock();
                break;
            case CMD_RESPONSE:
                // stream_mute.lock();
                std::cout << "server: " << p;
                // stream_mute.unlock();
                break;
            }

            // stream_mute.lock();
            // std::cout << buffer;
            // stream_mute.unlock();
        }
    }
}