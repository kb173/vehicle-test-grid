#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include "myqueue.h"

using namespace std;


int msgid = -1;  /* Message Queue ID */


// Handles SIGTERM (Ctrl C)
void my_handler(int s){
    printf("Caught signal %d\n", s);

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Message queue could not be deleted.\n");
        exit(EXIT_FAILURE);
    }

    printf("Message queue was deleted.\n");

    exit(1);
}

class Vehicle
{
public:
    char sym;
    int pos_x;
    int pos_y;

    Vehicle () {}

    Vehicle (char n_sym, int x, int y)
    {
        sym = n_sym;
        pos_x = x;
        pos_y = y;
    }
};

class Grid
{
private:
    char **grid;
    int size_x, size_y;

    Vehicle vehicles[26];
public:
    Grid (int x, int y)
    {
        size_x = x;
        size_y = y;

        // Initialize grid
        char **grid = new char*[x];

        for (int cx = 0; cx < x; cx++)
        {
            grid[cx] = new char[y];

            // Set all fields to empty
            for (int cy = 0; cy < y; cy++)
            {
                grid[cx][cy] = ' ';
            }
        }
    }

    bool new_vehicle (char v)
    {
        // Find first free field and put vehicle there
        for (int y = 0; y < size_y; y++)
        {
            for (int x = 0; x < size_x; x++)
            {
                if (grid[x][y] == ' ')
                {
                    vehicles[v - 'A'] = Vehicle(v, x, y);
                    return true;
                }
            }
        }

        return false; // Field is full
    }

    char move_vehicle (char v, int x_dir, int y_dir)
    {
        int c_x = vehicles[v - 'A'].pos_x;
        int c_y = vehicles[v - 'A'].pos_y;

        int n_x = c_x + x_dir;
        int n_y = c_y + y_dir;

        // Out of bounds?
        if (n_x < 0 || n_x > size_x || n_y < 0 || n_y > size_y)
        {
            return '!'; // Player dies
        }

        if (grid[n_x][n_y] == ' ')
        {
            vehicles[v - 'A'].pos_x = n_x;
            vehicles[v - 'A'].pos_y = n_y;

            return ' '; // Success!
        }

        return grid[n_x][n_y]; // Return the player that dies with the moved player
    }
};


int main (int argc, char* argv[])
{
    // Ctrl C handling
    struct sigaction sigIntHandler;

    sigIntHandler.sa_handler = my_handler;
    sigemptyset(&sigIntHandler.sa_mask);
    sigIntHandler.sa_flags = 0;

    sigaction(SIGINT, &sigIntHandler, NULL);

    // Read size and create grid
    int c;
    int x, y;

    while ((c = getopt( argc, argv, "x:y:" )) != EOF ) {
        switch (c) {
            case 'x':
                x = atoi(optarg);
                break;
            case 'y':
                y = atoi(optarg);
                break;
            case '?':
                fprintf( stderr, "Error: Unknown option.\n");
                exit(1);
                break;
            // default:
                /* assert() dient nur zu Debugzwecken und sollte nur dort eingesetzt sein,
                wo etwas sicher niemals passieren darf. 0 ist in C immer gleich "logisch falsch". */
        }
    }

    // TODO: Check if variables were entered and aren't just random stuff at address

    cout << "Started server with x = " << x << ", y = " << y << endl;

    Grid gr(x, y);

    // Message queue stuff
    message_t msg;  /* Buffer fuer Message */

    /* Message Queue neu anlegen */
    if ((msgid = msgget(KEY,PERM | IPC_CREAT | IPC_EXCL )) == -1)
    {
        /* error handling */
        fprintf(stderr,"%s: Error creating message queue\n",argv[0]);
        return EXIT_FAILURE;
    }

    /* In einer Endlosschleife Nachrichten empfangen */
    while (1)
    {
        if (msgrcv(msgid,&msg,sizeof(msg)-sizeof(long), 0 , 0) == -1)
        {
            // Error
            fprintf(stderr,"%s: Can't receive from message queue\n",argv[0]);
            return EXIT_FAILURE;
        }

        // Message received!
        cout << "Message received from " << (char)msg.mType << ": " << msg.mText << endl;
    }

    return 0;
}