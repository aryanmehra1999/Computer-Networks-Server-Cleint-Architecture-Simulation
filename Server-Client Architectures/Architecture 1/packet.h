/* 
ARYAN MEHRA 
2017A7PS0077P
*/

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <time.h>
#include <fcntl.h>
#include <sys/poll.h>

#define PACKET_SIZE 100     // Ammount of data in the packet
#define TIMEOUT_SECS 2      // Timeout is for the client side
#define BUFFER_SIZE 5       // Buffer on the server side
#define SERVER_PORT 12388   // This is for the server
#define MAX_CHANELS 10      // on the server side
#define SERVER_TIMEOUT 10   // This is for the server
#define DROPOUT_RATE 10     // For the server

typedef struct packet{
    int bytes_in_data;
    int sequence_number;
    int last;
    int is_data;
    int chanel;
    char data[PACKET_SIZE+1];
}packet;
