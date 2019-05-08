struct message
{
    int id_from, id_recip;
    char Message[120];
    bool delivered;
    char *name;
};

struct user
{
    short int ID;
    short int ID_RECIP;
    int sockfd;
    bool authorized;
    char name[20], password[20];
};
