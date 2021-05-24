// Client.c
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAXIN 10000
#define MAXOUT 10000

void getreq(char *inbuf, int len) {
    /* Get request char stream */
    // printf("REQ: ");                 /* prompt */
    memset(inbuf, 0, len);           /* clear for good measure */
    fgets(inbuf, len, stdin); /* read up to a EOL */
}

typedef struct {
    char typeOfMsg[12];
    int lenOfPayload;
} header;

header makeHeader(char *typeOfMsg1, int lenOfPayload1)
{
    header newentry;
    int i = 0;
    while (i < strlen(typeOfMsg1)){
        newentry.typeOfMsg[i] = typeOfMsg1[i];
        i++;
    }
    while (i < 12){
        newentry.typeOfMsg[i++] = 'X';
    }
    newentry.lenOfPayload = lenOfPayload1;
    return newentry;
}

/* sending user-id */
void initialconnect(int sockfd)
{
    int n;
    char sndbuf[MAXIN];
    /* Get request char stream */
    printf("Enter User-id: "); /* prompt */
    memset(sndbuf, 0, MAXIN);  /* clear for good measure */
    char *user_id = (char *)malloc(188 * sizeof(char));
    fgets(user_id, 188, stdin);
    int i = 0;
    sndbuf[i++] = 'm';
    sndbuf[i++] = 'y';
    sndbuf[i++] = 'I';
    sndbuf[i++] = 'd';
    while (i < 12) sndbuf[i++] = 'X';
    sndbuf[i++] = ':';
    for (int j = 0; j < strlen(user_id); j++){
        sndbuf[i++] = user_id[j];
    }
    write(sockfd, sndbuf, strlen(sndbuf)); /* send */
    return;
}

// read and send
void* readAndSendData(void* sockfdarg){
    // printf("reached here1\n");
    int consockfd = *((int*)sockfdarg);
    char inpbuf[MAXIN-13];
    char sndbuf[MAXIN];
    int n;
    header head;
    while(1){
        getreq(inpbuf, MAXIN);   
        memset(sndbuf,0,MAXIN);
        snprintf(sndbuf,MAXIN, "responseXXXX:%s",inpbuf);
        write(consockfd, sndbuf, strlen(sndbuf));
    }
}

// display messages
void* displayMessage(void* sockfdarg){
    // printf("reached here2\n");
    int sockfd = *((int*)sockfdarg);
    int n;
    char rcvbuf[MAXOUT];
    while(1){
        memset(rcvbuf,0,MAXOUT); 
        n=read(sockfd, rcvbuf, MAXOUT-1);
        if(n<=0)break;
        printf("%s",rcvbuf);
    }       
}

// Server address
struct hostent *buildServerAddr(struct sockaddr_in *serv_addr,
                                char *serverIP, int portno)
{
    /* Construct an address for remote server */
    memset((char *)serv_addr, 0, sizeof(struct sockaddr_in));
    serv_addr->sin_family = AF_INET;
    inet_aton(serverIP, &(serv_addr->sin_addr));
    serv_addr->sin_port = htons(portno);
}

void client(int sockfd) {
  int n;
  char inpbuf[MAXIN],rcvbuf[MAXOUT],sndbuf[MAXIN];
  while (1) {
    memset(rcvbuf,0,MAXOUT);               /* clear */
    n=read(sockfd, rcvbuf, MAXOUT-1);      /* receive */
    printf("SERVER: %s",rcvbuf);	      /* echo */

    getreq(inpbuf, MAXIN);        /* prompt */
    memset(sndbuf,0,MAXIN);
    snprintf(sndbuf, MAXIN, "responseXXXX:%s",inpbuf);
    write(sockfd, sndbuf, strlen(sndbuf)); /* send */
  }
}

int main()
{
    //Client protocol
    char *serverIP = "127.0.0.1";
    int *sockfd = malloc(sizeof(int));
    int portno = 49152;
    struct sockaddr_in serv_addr;

    buildServerAddr(&serv_addr, serverIP, portno);

    /* Create a TCP socket */
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);

    /* Connect to server on port */
    connect(*sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    printf("Connected to %s:%d\n", serverIP, portno);

    /* Carry out Client-Server protocol */
    initialconnect(*sockfd);
    
    /* to carry out read and write */
    pthread_t snd,rcv;
    pthread_create(&snd,NULL,readAndSendData,(void*)sockfd);
    pthread_create(&rcv,NULL,displayMessage,(void*)sockfd);

    pthread_join(snd,NULL);
    pthread_join(rcv,NULL);
    // client(*sockfd);
    /* Clean up on termination */
    close(*sockfd);
}
