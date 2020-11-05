NAME : ARYAN MEHRA
ID : 2017A7PS0077P
COMPUTER NETWORKS ASSIGNMENT QUESTION 1

EXECUTION INSTRUCTIONS

ON TERMINAL 1 RUN THE FOLLOWING 2 COMMANDS
    $ gcc -o server server.c
    $ ./server

You should see something like this on terminal:
    Server socket is created
    Binding is done
    Server is listening now

ON TERMINAL 2 RUN THE FOLLOWING 2 COMMANDS
    $ gcc -o client client.c
    $ ./client

Then the whole client-server system will start running now. 
Binary files for the server and client are also provided so the first commands on each terminal
to compile the client and server can be skipped as well. 

STATUS OF CODE:

All requirements are complete. Code compiles, and executes well. 

METHODOLOGY I USED:

I am using a polling based technique to poll the two connections on both the server and the client side. 
The client side timer is able to detect the loss on both chanels. The timer will go off (trigger a timeout)
when both chanels are blocked. In case only one chanel is blocked due to packet loss, the second chanel will
not be able to transfer all the file by itself, because the server side buffer will overflow and stop giving
ACK packets for the second chanel as well. This means even if one chanel gets blocked, the timer is bound to 
go off no matter what happens with the second chanel. Hence this helps with using sigle timer to give functionality
of two timers without explicitly forking. Two connections, since are a part of the same process, are able to know
the current file position and are able to transmit correctly and terminate when job is done. 

MACROS USED IN "packet.h" FILE:
#define PACKET_SIZE 100     // Ammount of data in the packet
#define TIMEOUT_SECS 2      // Timeout is for the client side
#define BUFFER_SIZE 5       // Buffer on the server side
#define SERVER_PORT 12388   // This is for the server
#define MAX_CHANELS 10      // on the server side
#define SERVER_TIMEOUT 10   // This is for the server
#define DROPOUT_RATE 10     // For the server

EXTRA THING DISPLAYED:
One extra thing is displayed in server, "DROPPING PACKET with seq number xxx" to show that ppacket has been dropped. That makes easier to visualise why 
client is resending the packets. Also resending is marked as "(RE)SENDING" instead of just "SENDING" for convinience.


INITIALLY WHAT I DID AND HAD GOT STUCK: 

Forking based approach was used at client side to make ideal 2 timers. 
Since ftell, fread, raad, write, lseek are all atomic operations, but still have situation where I can ftell or 
lseek the file for the position, but process switches context before I read from that location I just fetched. 
So semaphore based approach was used. The code ran successfully almost 90% of the time. Since I am using 
windows based gcc in Ubuntu App, rest of time I received "stack smashing" error because of semaphores and systemm calls to shared memory.
On suggestion of instructor-in-charge Ashutosh Sir, I avoided submitting this file as coding is too complicated. 
I have, however, attached the code in reference.txt (not for absolute evaluation, but only reference to show I did the work).    