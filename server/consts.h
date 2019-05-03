#define WHATID 0
#define REAUTHOR 1
#define CHANGERECIP 2
#define WRONGCMD 3
#define REDEFID 4

#define CHANGEID 7
#define MYID 12
#define DISCONNECT 13
#define RECIPIENTID 14

#define USER 10
#define RECIP 20

#define AUTHORIZATION 1
#define ID_AUTH 2
#define NAME 3

const char *notifications[] = {"Authorize using the pattern: \"YOUR_NICKNAME YOUR_ID\". \n",
                          "Wrong format. Use : \"YOUR_NICKNAME YOUR_ID\".\n",
                          "You've changed the recipient.\n", "Wrong command.\n",
                          "The ID is already used, please enter new id.\n"};

