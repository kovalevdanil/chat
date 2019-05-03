struct message
{
    int id_from, id_recip;
    char Message[100];
    bool delivered;
};

struct user
{
    short int ID;
    short int ID_adr;
    int sockfd;
    bool authorized;
    char *name;
};
