#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include "myqueue.h"

using namespace std;


int msgid = -1;  /* Message Queue ID */


// Handles SIGTERM (Ctrl C)
void my_handler(int s){
    printf("Caught signal %d\n", s);

    exit(1);
}

int main (int argc, char* argv[])
{
    // Ctrl C handling
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);


    char* programm_name = argv[0];
    char vehicle_name = argv[1][0];

    cout << "Started client with vehicle " << vehicle_name << endl;

    message_t msg;  /* Buffer fuer Message */

    if ((msgid = msgget(KEY,PERM)) == -1)
    {
        /* error handling */
        fprintf(stderr,"%s: Can't access message queue\n",argv[0]);
        return EXIT_FAILURE;
    }

    msg.mType = (long)vehicle_name;

    // Send initialize message
    strncpy(msg.mText, "new", MAX_DATA);

    if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
    {
        /* error handling */
        fprintf(stderr,"%s: Can't send message\n",argv[0]);
        return EXIT_FAILURE;
    }

    printf("Message sent: %s\n", msg.mText);


    char* input;

    while (1)
    {
        cin >> input;

        // Send message
        strncpy(msg.mText, input, MAX_DATA);

        if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
        {
            /* error handling */
            fprintf(stderr,"%s: Can't send message\n",argv[0]);
            return EXIT_FAILURE;
        }

        printf("Message sent: %s\n", msg.mText);
    }

    return 0;
}