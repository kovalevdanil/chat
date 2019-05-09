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

command format_command(char *cmd)
{
    char *separator = strchr(cmd, ' ');
    if (!separator)
        separator = strchr(cmd, '\n');
    if (separator)
        *separator = '\0';
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
    else if (!strcmp(cmd, "~leave"))
        my_cmd = leave_chat;
    else
        error;

    if (my_cmd == chat)
    {
        if (!separator)
            return error;

        cmd[0] = COMMAND;
        cmd[1] = ':';
        cmd[2] = my_cmd + '0';
        cmd[3] = ':';
        char *next_sep = strchr(separator + 1, ' ');
        if (next_sep)
            *next_sep = '\0';
        size_t size = strlen(separator + 1);
        memcpy(cmd + 4, separator + 1, size);

        cmd[size + 4] = '\0';
    }
    else
    {
        cmd[0] = COMMAND;
        cmd[1] = ':';
        cmd[2] = my_cmd + '0';
        cmd[3] = '\0';
    }

    return my_cmd;
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

        if (buffer[0] == '~')
        {
            command cmd;
            if (!(cmd = format_command(buffer)))
            {
                std::cout << "app: wrong command.\n";
                continue;
            }
            size = strlen(buffer);
        }
        else
        {
            shift(buffer, 2);
            buffer[0] = MESSAGE;
            buffer[1] = ':';
            size += 2;
        }
        send(sockfd, buffer, size, MSG_NOSIGNAL);
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
            {
                int message_num = atoi(p);
                std::cout << server << responses_text[message_num] << std::endl;
                break;
            }
            case MESSAGE:
                std::cout << p;
                break;
            case CMD_RESPONSE:
            {
                if (*p >= '0' && *p <= '9')
                {
                    int cmd_num = atoi(p);
                    std::cout << server << cmd_responses_text[cmd_num] << std::endl;
                }
                else
                    std::cout << server << p << std::endl;
                break;
            }
            case NOTIFICATION:
                std::cout << p << std::endl;
                break;
            }
        }
    }
}