#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include "myqueue.h"

using namespace std;

int main (int argc, char* argv[])
{
    char* programm_name = argv[0];
    char vehicle_name = argv[1][0];

    cout << "Started client with vehicle " << vehicle_name << endl;

    message_t msg;  /* Buffer fuer Message */
    int msgid = -1;  /* Message Queue ID */

    if ((msgid = msgget(KEY,PERM)) == -1)
    {
        /* error handling */
        fprintf(stderr,"%s: Can't access message queue\n",argv[0]);
        return EXIT_FAILURE;
    }

    msg.mType = (long)vehicle_name;

    // Send message
    strncpy(msg.mText, "Test", MAX_DATA);

    if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
    {
        /* error handling */
        fprintf(stderr,"%s: Can't send message\n",argv[0]);
        return EXIT_FAILURE;
    }

    printf("Message sent: %s\n", msg.mText);

    return 0;
}