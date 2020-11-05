/* 
ARYAN MEHRA 
2017A7PS0077P
*/

#ifndef PACKET_H_
#define PACKET_H_

#include<stdio.h> 
#include<sys/types.h> 
#include<unistd.h> 
#include<sys/poll.h>
#include<string.h> 
#include<stdlib.h> 
#include<arpa/inet.h>
#include<sys/socket.h>
#include <fcntl.h>
#include<time.h>
#include<sys/time.h>


#define PACKET_SIZE 100     // Ammount of data in the packet
#define TIMEOUT_SECS 2      // This is for the Client
#define BUFFER_SIZE 5       // This is for the Server
#define WINDOW_PACKETS 5    // This is for the Client
#define RELAY_TIMEOUT 45    // This is for the Relays and Server too close much later than client
#define DROP_PERCENTAGE 10  // This is for the Relays
#define PORT1 8890          // Client
#define PORT2 8891          // Relay 1
#define PORT3 8892          // Relay 2
#define PORT4 8893          // Server

typedef struct packet{
    int bytes_in_data;
    int sequence_number;
    int last;
    int is_data;
    int even_odd;
    char data[PACKET_SIZE+1];
}packet;


#endif