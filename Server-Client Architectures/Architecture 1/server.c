/* 
ARYAN MEHRA 
2017A7PS0077P
*/

#include "packet.h"
int main()
{
    
    srand(time(0));

    int listening_socket =  socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listening_socket < 0) 
    { 
        printf ("Error in opening a socket"); 
        exit (0);
    }
    printf ("Server Socket is created\n");

    struct sockaddr_in server_address, client_address;
    memset (&server_address,0,sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT); 
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); 

    // Binding the port 
    if(bind(listening_socket, (struct sockaddr*) &server_address,sizeof(server_address))<0)
    {
        printf("Error in binding the server socket");
        exit(0);
    }
    printf ("Binding is done \n");

    // Listening 
    if(listen(listening_socket,10)<0)
    {
        printf("Error while listening at the server");
        exit(0);
    }
    printf ("Server is Listening now \n");

    int fd = open("output.txt",O_CREAT);
    close(fd);
    fd = open("output.txt",O_WRONLY);

    struct pollfd me[MAX_CHANELS];
    int num_fds =1;
    me[0].fd = listening_socket;
    me[0].events = POLLIN;

    int timeout = SERVER_TIMEOUT * 1000;

    packet buffer[BUFFER_SIZE];
    int buffer_begin = 0;
    int buffer_count = 0;
    int buffer_mask[BUFFER_SIZE]={0};

    int done=0;
    while(1)
    {
        int return_value = poll(me,num_fds,timeout);

        if(return_value<0)
        {
            printf("Error in polling function\n");
            exit(0);
        }
        else if(return_value==0)
        {
            printf("Server timed out, Job must be over. Goodbye.\n");
            exit(0);
        }
        else
        {
            //someone wants to be read
            for(int i=0;i<num_fds;i++)
            {
                if(me[i].revents==0)
                    continue;
                
                if(me[i].fd == listening_socket)
                {
                    int new_socket = accept(listening_socket, NULL, NULL);

                    if(new_socket<0)
                        printf("Error in Accepting new connection\n");
                    
                    me[num_fds].fd = new_socket;
                    me[num_fds].events = POLLIN;
                    me[num_fds].revents = 0;
                    num_fds++;
                }
                else
                {
                    //data is coming
                    
                    packet data;
                    memset(&data,0,sizeof(data));

                    int received_value = recv(me[i].fd,&data,sizeof(data),0);

                    if(received_value==0)
                    {
                        if(i==0)printf("The chanel 1 to client terminated\n");
                        else printf("The chanel 2 to client terminated\n");
                        me[i].events = 0;
                    }
                    else if(received_value<0)
                    {
                        printf("Error in receiving");
                        exit(0);
                    }
                    else
                    {
                        //genuine read in data
                        printf("RECEIVED PACKET: Sequence number %d of size %d bytes from chanel %d \n",
                        data.sequence_number, data.bytes_in_data, data.chanel);


                        if(rand()%100>=DROPOUT_RATE)
                        {
                            //check whether space in buffer and that should I ACK the packet
                            int index = (data.sequence_number - buffer_begin)/PACKET_SIZE;
                            if(index>=0 && index<BUFFER_SIZE && buffer_mask[index]==0)
                            {
                                //new valid storable packet
                                buffer[index].sequence_number = data.sequence_number;
                                buffer[index].chanel = data.chanel;
                                buffer[index].last = data.last;
                                buffer[index].is_data = 1;
                                strcpy(buffer[index].data,data.data);
                                buffer[index].bytes_in_data = data.bytes_in_data;

                                buffer_mask[index]=1;
                                buffer_count++;

                                packet ack;
                                memset(&ack,0,sizeof(ack));
                                ack.sequence_number = data.sequence_number;
                                ack.last = data.last;
                                ack.chanel = data.chanel;
                                ack.is_data = 0;

                                if(send(me[i].fd,&ack,sizeof(ack),0)<0)
                                {
                                    printf("Error in sending\n");
                                    exit(0);
                                }

                                printf("SENT ACK: Sequence number %d from chanel %d \n",
                                ack.sequence_number,  ack.chanel);
                                
                                //see if you can empty the buffer now that a new entry is made
                                int all_set = 1;
                                int met_last = 0;
                                for(int i=0;i<BUFFER_SIZE;i++)
                                {
                                    if(buffer_mask[i]==0 && met_last==0)
                                        all_set=0;

                                    if(buffer[i].last==1)
                                        met_last=1;
                                }

                                // printf("all_set is %d \n",all_set);

                                if(all_set==1)
                                {
                                    buffer_count=0;
                                    memset(buffer_mask,0,sizeof(buffer_mask));
                                    buffer_begin += BUFFER_SIZE*PACKET_SIZE;

                                    for(int i=0;i<BUFFER_SIZE;i++)
                                    {
                                        write(fd,buffer[i].data,buffer[i].bytes_in_data);

                                        if(buffer[i].last==1)
                                        {
                                            done=1;
                                            break;
                                        }    
                                    }

                                    memset(buffer,0,sizeof(buffer));
                                }   
                            
                            }
                            
                            
                        }
                        else
                        {
                            printf("Packet Dropped\n");
                        }
                        
                        
                        
                        
                    }
                }
            }
        }

        if(done==1)
        break;

    }

    for(int i=0;i<num_fds;i++)
        close(me[i].fd);


}