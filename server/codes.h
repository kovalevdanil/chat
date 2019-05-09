#define ERROR '0'
#define MESSAGE '1'
#define COMMAND '2'
#define CMD_RESPONSE '3'
#define NOTIFICATION '4'

// errors/requests
#define NICK_IS_ALREADY_USED 0
#define WRONG_PASSW 1
#define PASS_DOESNT_MATCH 2
#define CHOOSE_DIALOGUE 3
#define AUTHORIZE 4

// cmd response
#define DOESNT_AVALIABLE 0
#define IN_CHAT 1

const char *responses_text[] = {"the nickname is already used, please enter other: ", "wrong password, please try again: ",
                        "passwords doesn't match, try again: ", "you haven't chose a dialogue",
                        "please authorize using form: \"NICKNAME:PASSWORD\""};
const char *cmd_responses_text[] = {"user isn't avaliable right now", "done, you're in chat"};
const char *responses[] = {"0:0", "0:1", "0:2", "0:3", "0:4"};
const char *cmd_responses[] = {"3:0", "3:1"};

enum command{error, online, chat, myid, disconnect, help};

