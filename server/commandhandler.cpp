#include <string.h>
#include <map>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include "usrmsg.h"
#include "consts.h"
// #include </home/martin/chat/format.cpp>

std::map<std::string, int> commands = {
    {"-cp", CHANGERECIP},
    {"-ci", CHANGEID},
    {"-myid", MYID},
    {"-q", DISCONNECT},
    {"-idr", RECIPIENTID}};

class commandhandler
{
    std::map<std::string, std::string> comms;
    char *line;
    struct user *usr;

    void change_recip(int ID)
    {
        usr->ID_adr = ID;
        std::string msg = "Server: your changed recipient ID to " + std::to_string(usr->ID_adr) + '\n';
        send(usr->sockfd, msg.c_str(), strlen(msg.c_str()), MSG_NOSIGNAL);
    }

    void change_id(int ID)
    {
        usr->ID = ID;
        std::string msg = "Server: you changed your ID to  " + std::to_string(usr->ID) + '\n';
        send(usr->sockfd, msg.c_str(), strlen(msg.c_str()), MSG_NOSIGNAL);
    }

    void send_id()
    {
        std::string msg = "Server: your ID is " + std::to_string(usr->ID) + '\n';
        send(usr->sockfd, msg.c_str(), strlen(msg.c_str()), MSG_NOSIGNAL);
    }

    void disconnect()
    {
    }

    void send_recipid()
    {
        std::string msg = "Server: recipient ID is " + std::to_string(usr->ID_adr) + "\n";
        send(usr->sockfd, msg.c_str(), strlen(msg.c_str()), MSG_NOSIGNAL);
    }

  public:
    commandhandler(char *str, struct user *User) : line(str), usr(User)
    {
        std::string command, arg;
        bool cmd = false, isarg = false;
        int size = strlen(line);
        for (int i = 0; i < size; i++)
        {
            bool add = (line[i] != '\r' && line[i] != '\n');
            if (line[i] == '-' && isarg)
            {
                cmd = true;
                isarg = false;
                comms[command] = arg;
                command.clear();
                arg.clear();
            }
            if (i == size - 1)
            {
                if (cmd && add)
                    command += line[i];
                if (isarg && add)
                    arg += line[i];
                comms[command] = arg;
                break;
            }

            if (line[i] == '-')
            {
                cmd = true;
                isarg = false;
            }
            if (line[i] == ' ' && cmd)
            {
                cmd = false;
                isarg = true;
            }

            if (cmd && add)
                command += line[i];
            else if (isarg && add)
                arg += line[i];
        }
    }

    int do_commands()
    {
        for (auto iter = comms.begin(); iter != comms.end(); iter++)
        {
            std::string cmd = iter->first;
            int cmd_num = commands[cmd];
            switch (cmd_num)
            {
            case CHANGERECIP:
                change_recip(atoi(iter->second.c_str()));
                break;
            case CHANGEID:
                change_id(atoi(iter->second.c_str()));
                break;
            case MYID:
                send_id();
                break;
            case RECIPIENTID:
                send_recipid();
                break;
            default:
                return 1;
            }
        }
        return 0;
    }
};