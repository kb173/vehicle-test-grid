#include <sys/stat.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <string.h>
#include <fcntl.h>

using namespace std;

#define BUFFER 200

FILE *fp;
string fifo_name = "displaypipe";
int fifo_mask = 0660;

int main ()
{
	int fd = open(fifo_name.c_str(), O_RDONLY);
	char in[BUFFER];

	while (1)
	{
		if (read(fd, in, BUFFER) > 0)
		{
			printf("%s", in);
		}
	}

	return 0;
}