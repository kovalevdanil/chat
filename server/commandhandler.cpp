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

class commandhandler
{
    void my_id(struct user *User)
    {
        send(User->sockfd, &User->ID, sizeof(User->ID), MSG_NOSIGNAL);
    }

  public:
    commandhandler(char *str, struct user *User)
    {
        command cmd = (command)atoi(str);
        switch (cmd)
        {
        case myid:
            my_id(User);
            break;
        }
    }
};