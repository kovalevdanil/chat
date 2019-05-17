#include <string.h>
#include <map>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <stdlib.h>
#include <string.h>

#include "usrmsg.h"
//#include "consts.h"
#include "codes.h"

using namespace std;

typedef map<int, struct user>  users_struct;

class commandhandler
{
    void my_id(struct user *User)
    {
        send(User->sockfd, &User->ID, sizeof(User->ID), MSG_NOSIGNAL);
    }

    void online_(struct user *User, map<int, user> *Users)
    {
        string online;
        online += CMD_RESPONSE;
        online += ':';
        online += "users online: \n";
        for (auto iter = Users->begin(); iter != Users->end(); iter++)
        {
            online += (iter->second).name;
            online += '\n';
        }
        online.pop_back();
        size_t size = online.length();
        send(User->sockfd, online.c_str(), size, MSG_NOSIGNAL);
    }

    void set_recip(struct user *User, users_struct *Users, char *name)
    {
        bool is_avaliable = false;
        for (auto iter = Users -> begin(); iter != Users -> end(); iter++)
            if (!strcmp(name, (iter -> second).name))
            {
                User -> ID_RECIP = (iter -> second).ID;
                is_avaliable = true;
            }
        if (is_avaliable)
        {
            User -> sent_notif = false;
            send (User -> sockfd, cmd_responses[IN_CHAT], strlen(cmd_responses[IN_CHAT]), MSG_NOSIGNAL);
        }
        else 
            send (User -> sockfd, cmd_responses[DOESNT_AVALIABLE], strlen(cmd_responses[DOESNT_AVALIABLE]), MSG_NOSIGNAL);
    }

    void leave(struct user *User)
    {
        User -> ID_RECIP = 0;
        send(User -> sockfd, cmd_responses[LEFT_CHAT], strlen(cmd_responses[LEFT_CHAT]), MSG_NOSIGNAL);
    }

    void disconnect_(struct user *User, users_struct &Users)
    {
        if (User -> ID_RECIP != 0)
        {
            int ID = User -> ID_RECIP;
            for (auto iter = Users.begin(); iter != Users.end(); iter++)
            {
                if (iter -> second.ID == ID)
                {
                    iter -> second.ID_RECIP = 0;
                    send(iter -> first, cmd_responses[RECIP_LEFT], sizeof(cmd_responses[RECIP_LEFT]), MSG_NOSIGNAL);
                    break;
                }
            }
        }
        User -> ID = 0;
    }
public:
    commandhandler(char *_command, struct user *User, users_struct  &Users)
    {
        command cmd = (command) (_command[0] - '0');
        switch (cmd)
        {
        case myid:
            my_id(User);
            break;
        case online:
            online_(User, &Users);
            break;
        case chat:
        {   
            char *name = &_command[2];
            set_recip(User, &Users, name);
            break;
        }
        case leave_chat:
            leave(User);
            break;
        case disconnect:
            disconnect_(User, Users);
        }
    }
};