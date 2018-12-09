#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <ctype.h>

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"
#define MAX_SIZE 50

typedef struct
{
    char *user;
    char *password;
    char *host;
    char *path;
    char *filename;
    char *ip;
    unsigned int port;
} URL;

void initURL(URL *url);
int parseURL(char *arg, URL *url);
void showURLInfo(URL *url);
void getIp(URL *url);
void receiveResponse(int sockedfd);

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in server_addr;

    if (argc != 2)
    {
        printf("Usage: %s ftp://[<user>:<password>@]<host>/<url-path>\n", argv[0]);
        exit(-1);
    }

    URL url;
    initURL(&url);

    parseURL(argv[1], &url);
    //    getIp(&url); //Dont know how to test it yet
    showURLInfo(&url);

    /*server address handling*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(SERVER_PORT);            /*server TCP port must be network byte ordered */

    /*open an TCP socket*/
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(0);
    }
    /*connect to the server*/
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect()");
        exit(0);
    }

    receiveResponse(sockfd);

    close(sockfd);
    exit(0);
}

void initURL(URL *url)
{
    url->user = malloc(MAX_SIZE);
    memset(url->user, 0, MAX_SIZE);
    
    url->password = malloc(MAX_SIZE);
    memset(url->password, 0, MAX_SIZE);
    
    url->host = malloc(MAX_SIZE);
    memset(url->host, 0, MAX_SIZE);
    
    url->path = malloc(MAX_SIZE);
    memset(url->path, 0, MAX_SIZE);
    
    url->filename = malloc(MAX_SIZE);
    memset(url->filename, 0, MAX_SIZE);
    
    url->ip = malloc(MAX_SIZE);
    memset(url->ip, 0, MAX_SIZE);
    
    //    url->port = FTP_PORT;
}

int parseURL(char *arg, URL *url)
{
    char prefix[7] = "ftp://";
    unsigned int length = strlen(arg), state = 0, index = 0, i = 0;

    while (index < length)
    {
        switch (state)
        {
        //Prefix
        case 0:
        {

            if (arg[index] == prefix[index])
            {
                if (index < 5) //Prefix is still being read
                    break;

                else if (index == 5) //Prefix was read correctly
                    state = 1;
            }

            else
            {
                printf("Error reading ftp://\n");
                exit(1);
            }

            break;
        }

        //user
        case 1:
        {

            if (arg[index] == ':') //Finished
            {
                i = 0;
                state = 2;
            }

            else
            {
                url->user[i] = arg[index];
                i++;
            }

            break;
        }

        //password
        case 2:
        {
            if (arg[index] == '@') //Finished
            {
                i = 0;
                state = 3;
            }

            else
            {
                url->password[i] = arg[index];
                i++;
            }

            break;
        }

        //host
        case 3:
        {
            if (arg[index] == '/') //Finished
            {
                i = 0;
                state = 4;
            }

            else
            {
                url->host[i] = arg[index];
                i++;
            }

            break;
        }

        //path
        case 4:
        {

            url->path[i] = arg[index];
            i++;

            break;
        }
        }

        index++;
    }

    //filename is going to be the string after the last '/' in path
    unsigned int indexPath = 0, indexFilename = 0;

    for (indexPath = 0; indexPath < strlen(url->path); indexPath++)
    {
        if (url->path[indexPath] == '/') // new directory
        {
            indexFilename = 0;
            memset(url->filename, 0, MAX_SIZE); //resets filename
            continue;
        }

        url->filename[indexFilename] = url->path[indexPath];
        indexFilename++;
    }

    return 0;
}

void showURLInfo(URL *url)
{
    printf("\nUser: %s\n", url->user);
    printf("Password: %s\n", url->password);
    printf("Host: %s\n", url->host);
    printf("Ip address: %s\n", url->ip);
    printf("Path: %s\n", url->path);
    printf("Filename = %s\n", url->filename);
}

void getIp(URL *url)
{
    struct hostent *h;

    if ((h = gethostbyname(url->host)) == NULL)
    {
        herror("gethostbyname");
        exit(1);
    }

    url->ip = inet_ntoa(*((struct in_addr *)h->h_addr));
}

void receiveResponse(int sockedfd)
{
    /* Example:
    123- First line
    Second line
    234 A line beginning with numbers
    123 The last line*/

    unsigned int state = 0, index = 0;
    char reader;
    unsigned char code[3]; 

    while (state < 3)
    {
        read(sockedfd, &reader, 1);
        printf("%c", reader);

        switch(state)
        {
            //3 digits code followed by space(one line) or hiphen(multi line) (RFC959 p.35)
            case 0:
            {
                //one line response
                if(reader == ' ')
                {
                    if(index != 3)
                    {
                        printf("Error reading response 3-digit code\n");
                        exit(1);
                    }

                    index = 0;
                    state = 1;
                }

                //multi line
                else if (reader == '-')
                {
                    index = 0;
                    state = 2;
                }

                //code still not finished
                else if(isdigit(reader))
                {
                    code[index] = reader;
                    index++; 
                }
            }

            //Last line is being read
            case 1:
            {
                //If line ends, state machine also ends
                if(reader == '\n')
                    state = 3;
            }

            //Multi line codes
            case 2:
            {
                //Since response code was found, that means that this is the last line
                if(index==3 && reader == ' ')
                    state = 1;

                //Code being read can be the response code 
                else if(reader == code[index])
                    index++;

                //Response code was found but the line is not the last one
                else if(index == 3 && reader == ' ')
                    index = 0;

            }
        }
    }



}