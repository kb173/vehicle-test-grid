#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <string.h>
#include "myqueue.h"

using namespace std;


int msgid = -1;  /* Message Queue ID */
message_t msg;  /* Buffer fuer Message */
char vehicle_name;


// Handles SIGTERM (Ctrl C)
void my_handler(int s){
    printf("Caught signal %d\n", s);

    msg.mType = vehicle_name;

    // Send terminate message
    strncpy(msg.mText, "T", MAX_DATA);

    if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
    {
        /* error handling */
        fprintf(stderr,"Can't send message\n");
    }

    printf("Message sent: %s\n", msg.mText);
}

void term_handler(int s)
{
    cout << "Vehicle has been eliminated." << endl;
}

int main (int argc, char* argv[])
{
    // Ctrl C handling
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // Termination handling
    struct sigaction sigTermHandler;

    sigTermHandler.sa_handler = term_handler;
    sigemptyset(&sigTermHandler.sa_mask);
    sigTermHandler.sa_flags = 0;

    sigaction(SIGTERM, &sigTermHandler, NULL);


    char* programm_name = argv[0];
    vehicle_name = argv[1][0];

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

    // Get coordinates
    if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), (long)vehicle_name + 100, 0) == -1)
    {
        // Error
        fprintf(stderr,"%s: Can't receive from message queue\n",argv[0]);
        return EXIT_FAILURE;
    }

    // Message received!
    cout << msg.mText << endl;


    msg.mType = (long)vehicle_name; // Needs to be set again, messages aren't received by server otherwise

    string input;

    while (cin >> input)
    {
        // Send message
        strncpy(msg.mText, input.c_str(), MAX_DATA);

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