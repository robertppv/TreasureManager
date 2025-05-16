#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define DEFAULT_LENGTH 32

typedef struct GPSCoordinates
{
    float latitude;
    float longitude;
} GPSCoordinates;

typedef struct treasure
{
    char treasureId[DEFAULT_LENGTH];
    char userName[DEFAULT_LENGTH];
    GPSCoordinates coordinates;
    char clueText[DEFAULT_LENGTH];
    int Value;
} treasure;

typedef struct
{
    char userName[DEFAULT_LENGTH];
    long int sumValue;
} user;

user users[1024];
int nr_users;

void add_to_list(char *user, int value)
{
    for (int i = 0; i < nr_users; i++)
    {
        if (strcmp(user, users[i].userName) == 0)
        {
            users[i].sumValue += value;
            return;
        }
    }
    users[nr_users].sumValue = value;
    strcpy(users[nr_users++].userName, user);
}
int main(int argc, char **argv)
{
    int fd;
    int res;
    int ok = 0;
    char path[256] = "";
    treasure t;
    sprintf(path, "./Game/%s/%s_treasures.dat", argv[1], argv[1]);
    printf("%s:\n", argv[1]);

    if ((fd = open(path, O_RDONLY)) < 0)
    {
        perror("err");
        exit(-1);
    }
    while ((res = read(fd, &t, sizeof(treasure))) > 0)
    {
        add_to_list(t.userName, t.Value);
    }
    if (res < 0)
    {
        perror("Error reading from treasures file");
        exit(-1);
    }

    for (int i = 0; i < nr_users; i++)
    {
        printf("%s = %ld\n", users[i].userName, users[i].sumValue);
    }

    
    printf("\n");
    close(fd);
}