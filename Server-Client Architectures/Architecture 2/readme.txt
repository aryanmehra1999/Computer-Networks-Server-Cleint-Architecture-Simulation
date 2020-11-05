NAME: ARYAN MEHRA
ID: 2017A7PS0077P
COMPUTER NETWORKS ASSIGNMENT QUESTION 2

STATUS OF CODE

All required features are complete. Coding is running and well.

COMMANDS TO EXECUTE (IN ORDER):

ON TERMINAL 1 EXECUTE THE FOLLOWING COMMANDS:
    $ gcc -o server server.c
    $ gcc -o client client.c
    $ gcc -o relay relay.c

ON TERMINAL 1 EXECUTE THE FOLLOWING COMMANDS:
    $ ./server

ON TERMINAL 2 EXECUTE THE FOLLOWING COMMANDS:
    $ ./relay 1

ON TERMINAL 3 EXECUTE THE FOLLOWING COMMANDS:
    $ ./relay 2

ON TERMINAL 4 EXECUTE THE FOLLOWING COMMANDS:
    $ ./client

Binary files are also included for reference and direct running without compilation and extra libraries download.

METHODOLOGY USED:

I have made a client, 2 relay and 1 server node. The client simply maintains a window of particular size
that can be controlled by a macro. So when a timeout occurs, only the un-ACKED packets of the window are resent.
Since the timeout window for client is on an average 2 seconds for submission, all packets whose ACK is supposed to 
arrive must have arrived by that time, else they are genuinely lost. So a single per-window timer will help to 
do a selective repeat (only unACKED packets are resent). On the server side, there is a buffer which is written to
the file only when it is full, thus reducing write operations. Server does not ACK the packets beyond it's buffer reach
because it cant  buffer them and same way no ACK for below it's buffer reach because I must have already ACKed them, and
ACKs don't get lost or delayed. Relay nodes are simple. They simply take something and forward it with random delay,
or they drop it altogether. So no buffer or queue is required as such, because anyway the delay has to be introduced.

MACROS USED IN "packet.h":

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




