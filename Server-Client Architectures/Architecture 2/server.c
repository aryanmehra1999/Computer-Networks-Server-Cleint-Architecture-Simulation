/* 
ARYAN MEHRA 
2017A7PS0077P
*/



#include "packet.h"

char* time_now()
{
    char *str = (char*) malloc(sizeof(char)*20);
    time_t current_time;
    struct tm* time_pointer;
    int return_count;
    struct timeval time_value;
    gettimeofday(&time_value,NULL);

    current_time = time(NULL);
    time_pointer = localtime(&current_time);
    return_count = strftime(str,20, "%H:%M:%S",time_pointer);

    char milli[8];
    sprintf(milli,".%06ld",time_value.tv_usec);
    strcat(str,milli);
    return str;

}

int main()
{
    int my_socket;
    if((my_socket =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("Error while creating relay socket 1");
        exit(0);
    }

    struct sockaddr_in relay1_addr, relay2_addr, my_addr, other_addr;

    memset((char *) &relay1_addr, 0, sizeof(relay1_addr));
    relay1_addr.sin_family = AF_INET;
    relay1_addr.sin_port = htons(PORT2);
    relay1_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    
    memset((char *) &relay2_addr, 0, sizeof(relay2_addr));
    relay2_addr.sin_family = AF_INET;
    relay2_addr.sin_port = htons(PORT3);
    relay2_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset((char *) &my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(PORT4);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);


    if( bind(my_socket , (struct sockaddr*)&my_addr, sizeof(my_addr) ) == -1)
    {
        printf("Error binding my address");
        exit(0);
    }

    printf("Socket and Binding are done..\n");

    int fd = open("output.txt", O_CREAT);
    close(fd);
    fd = open("output.txt",O_WRONLY);
    if(fd<0)
    {
        printf("Error opening file");
        exit(0);
    }

    struct pollfd me[1];
    me[0].fd = my_socket;
    me[0].events = POLLIN;

    int timeout =  RELAY_TIMEOUT * 1000;

    int addr_len = sizeof(relay1_addr);

    packet buffer[BUFFER_SIZE];
    int buffer_count = 0;
    int buffer_begin = 0;
    int file_pos = 0;
    int buffer_mask[BUFFER_SIZE];
    int buffer_size = BUFFER_SIZE*PACKET_SIZE;
    memset(buffer_mask,0,sizeof(buffer_mask));
    memset(buffer,0,sizeof(buffer));

    printf("%8s | %10s | %20s | %8s | %10s | %8s | %8s |\n",
    "NODE","EVENT","TIME","TYPE","SEQ NUM","SOURCE","DEST");


    int done_condition=0;

    while(1)
    {
        int return_value = poll(me,1,timeout);

        if(return_value<0)
        {
            printf("error in polling");
            exit(0);
        }
        else if(return_value==0)
        {
            break;
        }
        else
        {
            if(me[0].revents==POLLIN)
            {
                packet p;
                memset(&p,0,sizeof(p));
                recvfrom(me[0].fd,&p,sizeof(p),0,(struct sockaddr *)&other_addr,&addr_len);
                
                if(ntohs(other_addr.sin_port)==PORT2)
                printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                "SERVER","RECV",time_now(),"DATA",p.sequence_number,"RELAY1","SERVER");
                else 
                printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                "SERVER","RECV",time_now(),"DATA",p.sequence_number,"RELAY2","SERVER");

                int index = (p.sequence_number - buffer_begin)/PACKET_SIZE;
                int last_buffer = p.last;
                
                //is it eligible for the buffer
                if(index<BUFFER_SIZE && buffer_mask[index]==0)
                {
                    //write to the buffer and send the ACK
                    buffer[index].sequence_number = p.sequence_number;
                    buffer[index].bytes_in_data = p.bytes_in_data;
                    buffer[index].last = p.last;
                    strcpy(buffer[index].data,p.data);     
                    buffer_mask[index] = 1;               
                    buffer_count++;    
                    
                    packet ack;
                    memset(&ack,0,sizeof(p));
                    ack.sequence_number = p.sequence_number;
                    ack.is_data = 0;
                    ack.last = p.last;

                    if(ntohs(other_addr.sin_port)==PORT2)
                    {
                        sendto(my_socket,&ack,sizeof(packet),0,(struct sockaddr *)&relay1_addr,addr_len);
                        // printf("Sending ACK packet to Relay 1: Seq Num %d\n",ack.sequence_number);
                        printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                        "SERVER","SEND",time_now(),"ACK",ack.sequence_number,"SERVER","RELAY1");
                    }
                    else if(ntohs(other_addr.sin_port)==PORT3)
                    {
                        sendto(my_socket,&ack,sizeof(packet),0,(struct sockaddr *)&relay2_addr,addr_len);
                        // printf("Sending ACK packet to Relay 2: Seq Num %d\n",ack.sequence_number);
                        printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                        "SERVER","SEND",time_now(),"ACK",ack.sequence_number,"SERVER","RELAY2");
                    }

                    int all_set = 1;
                    int met_last = 0;
                    for(int i=0;i<BUFFER_SIZE;i++)
                    {
                        if(buffer_mask[i]==0 && met_last==0)
                            all_set=0;
                        
                        if(buffer[i].last==1)
                            met_last=1;
                    }

                    if(met_last==1 && all_set==1)
                    done_condition=1;

                    //now see if you can write to the file
                    if(all_set==1)
                    {
                        //write to the file
                        for(int i=0;i<BUFFER_SIZE;i++)
                        {
                            write(fd,buffer[i].data,buffer[i].bytes_in_data);
                            if(buffer[i].last==1)
                                break;
                        }    

                        //empty the buffer, and the count to 0
                        memset(buffer,0,sizeof(buffer));
                        memset(buffer_mask,0,sizeof(buffer_mask));
                        buffer_count=0;
                        buffer_begin += buffer_size;
                    }
                    
                }

                
                //when to break
                if(done_condition==1)
                    break;
            }
            else
            {
                continue;
            }
            
        }
        
    }

    close(fd);

}