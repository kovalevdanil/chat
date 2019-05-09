#include <iostream>
#include <set>
#include <algorithm>
#include <vector>
#include <string.h>
#include <map>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>

#include "commandhandler.cpp"

int free_id = 1;

int set_nonblock(int fd)
{
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return (ioctrl(fd, FIOBNIO, &fformat_message(char num, char *message, char *name = NULL) {

    } lags));
#endif
}

// void *get_auth_info(char *id_form, int what)
// {
    // if (!strchr(id_form, ' '))
        // return NULL;

    // char *p;
    // if (what == NAME)
    // {
        // int len = 0;
        // while (id_form[len] != ' ')
            // len++;
        // p = new char[len + 1];
        // memcpy(p, id_form, len);
        // *(p + len) = '\0';
        // return p;
    // }
    // else if (what == ID_AUTH)
    // {
        // p = strchr(id_form, ' ') + 1;
        // int *ret = new int(atoi(p));
        // return (void *)ret;
    // }
// }

int authorize(struct user *usr, char *id_form)
{
    char *name = id_form;
    char *separ = strchr(id_form, ':');
    if (!separ)
        return 0;
    char *passw = separ + 1;

    *separ = '\0';

    strcpy(usr->name, name);
    strcpy(usr->password, passw);

    usr->authorized = true;

    return 1;
}

void shift(char *buffer, int num)
{
    int i = strlen(buffer) + num;
    for (; i > num - 1; i--)
        buffer[i] = buffer[i - num];
}

void set_name(char *buffer, char *name)
{
    int len_name = strlen(name);
    int num = len_name + 4;
    shift(buffer, num);
    buffer[0] = MESSAGE;
    buffer[1] = ':';
    memcpy(buffer + 2, name, len_name);
    buffer[len_name + 2] = ':';
    buffer[len_name + 3] = ' ';
}

bool is_delivered(struct message msg)
{
    return msg.delivered;
}

int main()
{
    int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    std::set<int> Sockets;            // slave sockets (not slav)
    std::vector<message> Messages;    // message to delive
    std::map<int, struct user> Users; // all connected users <SOCKET_FD, USER>

    struct sockaddr_in SockAddr;

    SockAddr.sin_family = AF_INET;
    SockAddr.sin_port = htons(3490);
    SockAddr.sin_addr.s_addr = htons(INADDR_ANY);

    int optval = 1;
    setsockopt(MasterSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(MasterSocket, (struct sockaddr *)(&SockAddr), sizeof(SockAddr)))
    {
        std::cerr << "server: bind\n";
        exit(1);
    }

    set_nonblock(MasterSocket);

    if (listen(MasterSocket, SOMAXCONN))
    {
        std::cerr << "server: listen\n";
        exit(2);
    }

    while (1)
    {
        // Creating fd_set for using select()
        fd_set Set;
        FD_ZERO(&Set);
        FD_SET(MasterSocket, &Set);
        for (auto iter = Sockets.begin(); iter != Sockets.end(); iter++) // add Slaves in fd_set
            FD_SET(*iter, &Set);

        int Max = std::max(MasterSocket, *std::max_element(Sockets.begin(), Sockets.end())); //searching max descriptor for using select()

        select(Max + 1, &Set, NULL, NULL, NULL);

        for (auto iter = Sockets.begin(); iter != Sockets.end(); iter++)
        {
            if (!FD_ISSET(*iter, &Set)) // if 0 then no events, continue
                continue;

            struct user *current_user = &Users[*iter];

            static char buffer[1024];
            int RecvSize = recv(*iter, buffer, sizeof(buffer), MSG_NOSIGNAL);

            if ((RecvSize == 0) && errno != EAGAIN) // shut connection
            {
                shutdown(*iter, SHUT_RDWR);
                close(*iter);

                Users.erase(*iter);
                Sockets.erase(iter);
            }
            else if (RecvSize != 0)
            {
                if (!current_user->authorized) // authorizing (in work)
                {
                    if (!authorize(current_user, buffer + 2))
                        send(*iter, responses[AUTHORIZE],
                             sizeof(responses[AUTHORIZE]), MSG_NOSIGNAL);
                }
                else
                {
                    // adding message in vector<message>
                    if (buffer[0] == MESSAGE) //regular message
                    {
                        if (!current_user->ID_RECIP)
                        {
                            send(*iter, responses[CHOOSE_DIALOGUE], sizeof(responses[CHOOSE_DIALOGUE]), MSG_NOSIGNAL);
                            continue;
                        }

                        if (RecvSize > 99)
                            RecvSize = 99;

                        struct message cur_message;
                        buffer[RecvSize] = '\0';

                        memcpy(cur_message.Message, buffer + 2, RecvSize);
                        cur_message.delivered = false;
                        cur_message.id_from = current_user->ID;
                        cur_message.id_recip = current_user->ID_RECIP;
                        cur_message.name = current_user -> name; // very doubtful (if person is disconnected, his name is deleted)

                        Messages.push_back(cur_message);
                    }
                    else if (buffer[0] == COMMAND) //command
                    {
                        buffer[RecvSize] = '\0';
                        char *cmd = &buffer[2]; 
                        commandhandler command_handle(cmd, current_user, Users);
                    }
                }
            }
        }

        // Authorizing of new users
        if (FD_ISSET(MasterSocket, &Set))
        {
            int new_fd = accept(MasterSocket, 0, 0);
            set_nonblock(new_fd);
            Sockets.insert(new_fd);

            struct user new_user;
            new_user.ID = free_id++;
            new_user.ID_RECIP = 0;
            new_user.sockfd = new_fd;
            new_user.authorized = false;

            Users[new_fd] = new_user;

            send(new_fd, responses[AUTHORIZE],
                 sizeof(responses[AUTHORIZE]), MSG_NOSIGNAL);
        }

        // Messages delivering
        auto message = Messages.begin();
        for (; message != Messages.end(); message++)
        {
            int recip_sock = 0;
            int id_recip = message->id_recip;

            for (auto iter = Users.begin(); iter != Users.end(); iter++)
                if ((iter->second).ID == id_recip)
                {
                    recip_sock = iter->first;
                    break;
                }

            bool is_available = false;
            for (auto i = Sockets.begin(); i != Sockets.end(); i++)
            {
                if (recip_sock == *i)
                {
                    is_available = true;
                    break;
                }
            }

            if (is_available)
            {
                set_name(message->Message, message->name);
                send(recip_sock, message->Message, strlen(message->Message), MSG_NOSIGNAL);
                message->delivered = true;
            }
        }

        //erasing delivered messages
        Messages.erase(std::remove_if(Messages.begin(), Messages.end(), is_delivered), Messages.end());
    }
    return 0;
}