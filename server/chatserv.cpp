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

int set_nonblock(int fd)
{
    int flags;
#if defined(O_NONBLOCK)
    if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
        flags = 0;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
    flags = 1;
    return (ioctrl(fd, FIOBNIO, &flags));
#endif
}

//Checking if id recieved properly
bool is_correct_form(char *p, int what)
{
    char *colon = strchr(p, ':');

    if (!colon || strchr(colon + 1, ':'))
        return false;
    
    if (what == AUTHORIZATION)
    {
        if (*p < '0' || *p > '9')
            return false;
        while (*p >= '0' && *p <= '9')
            p++;
        if (*p != ':')
            return false;
        p++;
        while (*p >= '0' && *p <= '9')
            p++;
        if (*p != '\0' && *p != '\n' && *p != '\r')
            return false;
    }
    return true;
}

int get_id(char *req, int who)
{
    if (!is_correct_form(req, AUTHORIZATION))
        return 0;

    char *p = req;
    if (who == RECIP) p = strchr(req, ':') + 1; 

    return atoi(p);
}

void set_name(struct user &usr, char *id_form)
{

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
                    int ID = get_id(buffer, USER);
                    int RECIP_ID = get_id(buffer, RECIP);
                    for (auto iter = Users.begin(); iter != Users.end(); iter++)
                        if (ID == iter->second.ID)
                        {
                            ID = -1;
                            break;
                        }
                    if (ID != -1)
                    {
                        if (ID == 0 || RECIP_ID == 0)
                        {
                            send(*iter, notifications[REAUTHOR], strlen(notifications[REAUTHOR]), MSG_NOSIGNAL);
                            continue;
                        }
                        current_user->ID = ID;
                        current_user->ID_adr = RECIP_ID;
                        current_user->authorized = true;
                    }
                    else
                    {
                        send(*iter, notifications[REDEFID], strlen(notifications[REDEFID]), MSG_NOSIGNAL);
                        continue;
                    }


                }
                else
                {
                    // adding message in vector<message>
                    if (buffer[0] != '-') //regular message
                    {
                        struct message cur_message;
                        buffer[RecvSize] = '\0';

                        memcpy(cur_message.Message, buffer, RecvSize + 1);
                        //cur_message.message = buffer;
                        cur_message.delivered = false;
                        cur_message.id_from = current_user->ID;
                        cur_message.id_recip = current_user->ID_adr;

                        Messages.push_back(cur_message);
                    }
                    else //command handler
                    {
                        char cmd[50];
                        memcpy(cmd, buffer, RecvSize);
                        commandhandler cmh(cmd, current_user);
                        if (cmh.do_commands())
                            send(*iter, notifications[WRONGCMD], strlen(notifications[WRONGCMD]), MSG_NOSIGNAL);
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
            new_user.ID = 0;
            new_user.sockfd = new_fd;
            new_user.authorized = false;

            send(new_fd, notifications[WHATID], strlen(notifications[WHATID]), MSG_NOSIGNAL);

            Users[new_fd] = new_user;
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
                char *whosend = (char *)("ID" + std::to_string(message->id_from) + ": ").c_str();
                send(recip_sock, whosend, strlen(whosend), MSG_NOSIGNAL);
                send(recip_sock, message->Message, strlen(message->Message), MSG_NOSIGNAL);
                message->delivered = true;
            }
        }

        //erasing delivered messages
        Messages.erase(std::remove_if(Messages.begin(), Messages.end(), is_delivered), Messages.end());
    }
    return 0;
}