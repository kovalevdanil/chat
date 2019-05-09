struct message
{
    int id_from, id_recip;
    char Message[100];
    bool delivered;
    char name[20];
};

struct user
{
    short int ID;
    short int ID_RECIP;
    int sockfd;
    bool sent_notif;
    bool authorized;
    char name[20], password[20];
};
