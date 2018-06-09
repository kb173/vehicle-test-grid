#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <signal.h>
#include <sys/stat.h>
#include "myqueue.h"

using namespace std;

FILE *fp;
string fifo_name = "displaypipe";
int fifo_mask = 0660;

int msgid = -1;  /* Message Queue ID */


// Handles SIGTERM (Ctrl C)
void my_handler(int s){
    printf("Caught signal %d\n", s);

    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        fprintf(stderr, "Message queue could not be deleted.\n");
        exit(EXIT_FAILURE);
    }

    printf("Message queue was deleted.\n");

    remove(fifo_name.c_str());

    printf("FIFO was deleted.\n");

    exit(1);
}

class Vehicle 
{
private:
    message_t msg;  /* Buffer fuer Message */

public:
    char sym;
    int pos_x;
    int pos_y;
    int pid;

    Vehicle () {}

    Vehicle (char n_sym, int x, int y, int p)
    {
        sym = n_sym;
        pos_x = x;
        pos_y = y;
        pid = p;

        msg.mType = (long)sym + 100;
    }

    void remove ()
    {
        if (kill(pid, SIGTERM) == -1)
        {
            printf("Error killing process with pid %i", pid);
        }
    }

    void send (string txt)
    {
        // Send with pid as type
        strncpy(msg.mText, txt.c_str(), MAX_DATA);

        if (msgsnd(msgid, &msg, sizeof(msg) - sizeof(long), 0) == -1)
        {
            /* error handling */
            fprintf(stderr,"Can't send message\n");
            return;
        }

    }
};

class Grid
{
private:
    char **grid;
    int size_x, size_y;

    Vehicle* vehicles[26];
public:
    Grid (int x, int y)
    {
        size_x = x;
        size_y = y;

        // Initialize vehicles
        for (int i = 0; i < 26; i++)
        {
            vehicles[i] = nullptr;
        }
    }

    bool new_vehicle (char v, int p)
    {
        // Check if vehicle with symbol already exists
        if (vehicles[v - 'A'] != nullptr)
        {
            cout << "There already is a vehicle with this character!" << endl;

            return false;
        }

        // Find first free field and put vehicle there
        for (int x = 0; x < size_x; x++) // DOESNT WORK YET!
        {
            for (int y = 0; y < size_y; y++)
            {
                //if (grid[x][y] == ' ')
                //{
                    bool can_place = true;

                    for (int i = 0; i < 26; i++)
                    {
                        if (vehicles[i] != nullptr &&
                            vehicles[i]->pos_x == x && vehicles[i]->pos_y == y)
                        {
                            can_place = false;
                        }
                    }

                    if (can_place)
                    {
                        vehicles[v - 'A'] = new Vehicle(v, x, y, p);

                        // Send coordinates
                        vehicles[v - 'A']->send("Start position: " + to_string(x) + ", " + to_string(y));

                        return true;
                    }
                //}
            }
        }

        cout << "Field is full!" << endl;
    }

    void remove_vehicle (char v)
    {
        vehicles[v - 'A']->remove();
        vehicles[v - 'A'] = nullptr;
        cout << "Removed vehicle " << v << endl;
    }

    void move_vehicle (char v, int x_dir, int y_dir)
    {
        int c_x = vehicles[v - 'A']->pos_x;
        int c_y = vehicles[v - 'A']->pos_y;

        int n_x = c_x + x_dir;
        int n_y = c_y + y_dir;

        // Out of bounds?
        if (n_x < 0 || n_x >= size_x || n_y < 0 || n_y >= size_y)
        {
            vehicles[v - 'A']->remove();
            vehicles[v - 'A'] = nullptr;
            return;
        }

        for (int i = 0; i < 26; i++)
        {
            if (vehicles[i] != nullptr && vehicles[i]->pos_x == n_x && vehicles[i]->pos_y == n_y) // Crash!
            {
                vehicles[v - 'A']->remove();
                vehicles[v - 'A'] = nullptr;
                vehicles[i]->remove();
                vehicles[i] = nullptr;

                return;
            } 
        }

        // Success!
        vehicles[v - 'A']->pos_x = n_x;
        vehicles[v - 'A']->pos_y = n_y;

        return;
    }

    void print_board ()
    {
        string board = "";

        for (int y = 0; y < size_y + 2; y++)
        {
            for (int x = 0; x < size_x + 2; x++)
            {
                char sym = ' ';

                // Border
                if (x == 0 || y == 0 || x == size_x + 1 || y == size_y + 1)
                {
                    sym = '#';
                }

                // Check whether a vehicle is here
                for (int i = 0; i < 26; i++)
                {
                    if (vehicles[i] != nullptr &&
                        vehicles[i]->pos_x == x - 1 && vehicles[i]->pos_y == y - 1)
                    {

                        sym = vehicles[i]->sym;
                    }
                }

                board += sym;
            }

            board += "\n";
        }

        board += "\0";

        fp = fopen(fifo_name.c_str(), "w");
        fprintf(fp, "%s", board.c_str());
        fclose(fp);
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
    struct msqid_ds msqid_ds, *buf;
    buf = & msqid_ds;

    if ((msgid = msgget(KEY,PERM | IPC_CREAT | IPC_EXCL )) == -1)
    {
        /* error handling */
        fprintf(stderr,"%s: Error creating message queue\n",argv[0]);
        return EXIT_FAILURE;
    }

    // Make FIFO
    if (mkfifo(fifo_name.c_str(), fifo_mask) == -1)
    {
        fprintf(stderr, "Error creating fifo\n");
    }

    /* In einer Endlosschleife Nachrichten empfangen */
    while (1)
    {
        if (msgrcv(msgid, &msg, sizeof(msg) - sizeof(long), -(long)'Z' , 0) == -1)
        {
            // Error
            fprintf(stderr,"%s: Can't receive from message queue\n",argv[0]);
            return EXIT_FAILURE;
        }

        // Message received!
        char snd = (char)msg.mType;
        msgctl(msgid, IPC_STAT, buf);

        cout << "Message received from " << snd << ": " << msg.mText << endl;

        if (msg.mText[0] == 'n') // New vehicle registration
        {
            cout << "New vehicle" << endl;

            gr.new_vehicle(snd, buf->msg_lspid);
        }
        else if (msg.mText[0] == 'T')
        {
            gr.remove_vehicle(snd);
        }
        else if (msg.mText[0] == 'N')
        {
            gr.move_vehicle(snd, 0, -1);
        }
        else if (msg.mText[0] == 'E')
        {
            gr.move_vehicle(snd, 1, 0);
        }
        else if (msg.mText[0] == 'S')
        {
            gr.move_vehicle(snd, 0, 1);
        }
        else if (msg.mText[0] == 'W')
        {
            gr.move_vehicle(snd, -1, 0);
        }

        gr.print_board();
    }

    return 0;
}