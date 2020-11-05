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
        printf("error while creating relay socket 1");
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
    my_addr.sin_port = htons(PORT1);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int addr_len = sizeof(relay1_addr);

    if( bind(my_socket , (struct sockaddr*)&my_addr, sizeof(my_addr) ) == -1)
    {
        printf("error binding my address");
        exit(0);
    }

    printf("Socket and Binding are done..\n");

    printf("%8s | %10s | %20s | %8s | %10s | %8s | %8s |\n",
    "NODE","EVENT","TIME","TYPE","SEQ NUM","SOURCE","DEST");
    
    int fd = open("input.txt", O_RDONLY);
    if(fd<0)
    {
        printf("Error opening file\n");
        exit(0);
    }

    int file_size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);

    packet p,ack;

    int window_begin = 0;
    int window_size = WINDOW_PACKETS * PACKET_SIZE;
    int ack_keeper[WINDOW_PACKETS]={0};
    packet window[WINDOW_PACKETS];
    
    struct pollfd me[1];
    me[0].fd = my_socket;
    me[0].events = POLLIN;
    
    int done=0;
    int timeout = TIMEOUT_SECS * 1000;

    int last_window=0;

    while(1)
    {
        //fill and send the data for the whole window because it is a new window
        for(int i=0;i<WINDOW_PACKETS;i++)
        {
            window[i].sequence_number = window_begin + i*PACKET_SIZE;
            
            if(window[i].sequence_number>=file_size)
                break;

            window[i].bytes_in_data = read(fd,window[i].data,PACKET_SIZE);
            window[i].even_odd = (window[i].sequence_number/PACKET_SIZE)%2;
            
            if(window[i].sequence_number>=file_size-PACKET_SIZE)
            window[i].last = 1;
            else window[i].last = 0;

            if(window[i].even_odd == 0) 
            {
                if(sendto(my_socket,&window[i],sizeof(packet),0,(struct sockaddr *)&relay2_addr,addr_len)==(-1))
                printf("Error in sending packet\n");
                else 
                printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                "CLIENT","SEND",time_now(),"DATA",window[i].sequence_number,"CLIENT","RELAY2");
            }
            else
            {
                if(sendto(my_socket,&window[i],sizeof(packet),0,(struct sockaddr *)&relay1_addr,addr_len)==(-1))
                printf("Error in sending packet\n");
                else 
                printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                "CLIENT","SEND",time_now(),"DATA",window[i].sequence_number,"CLIENT","RELAY1");
            }


        }
        
        int timeout_counter = 0;
        int window_done = 0;
        while(window_done == 0)
        {    
            // printf("Stuck");
            int return_value = poll(me,1,timeout);

            if(return_value<0)
            {
                printf("Error in polling\n");
                exit(0);
            }
            else if(return_value==0)
            {
                //timeout



                timeout_counter++;

                int all_set =0;

                int display = 0;

                for(int i=0; i<WINDOW_PACKETS;i++)
                {
                    if(ack_keeper[i]==0 && (window_begin+i*PACKET_SIZE)<file_size)
                    {
                        
                        if(display==0)
                        {
                            printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                            "CLIENT","TIMEOUT",time_now(),"DATA",window[i].sequence_number,"-----","-----");
                            display=1;
                        }

                        all_set=1;
                        //re-send selected non acked packets
                        if(window[i].even_odd == 0) 
                        {
                            if(sendto(my_socket,&window[i],sizeof(packet),0,(struct sockaddr *)&relay2_addr,addr_len)==(-1))
                            printf("Error in sending packet\n");
                            else 
                            printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                            "CLIENT","RE",time_now(),"DATA",window[i].sequence_number,"CLIENT","RELAY2");
                        }
                        else
                        {
                            if(sendto(my_socket,&window[i],sizeof(packet),0,(struct sockaddr *)&relay1_addr,addr_len)==(-1))
                            printf("Error in sending packet\n");
                            else 
                            printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                            "CLIENT","RE",time_now(),"DATA",window[i].sequence_number,"CLIENT","RELAY1");
                        }
                    }
                    else
                    {
                        // printf("here!");
                        continue;
                    }
                    
                }

                if(all_set==0 || timeout_counter==15)
                break;
            }
            else
            {
                timeout_counter=0;
                if(me[0].revents==POLLIN)
                {
                    memset(&ack,0,sizeof(ack));
                    recvfrom(me[0].fd,&ack,sizeof(ack),0,(struct sockaddr *)&other_addr,&addr_len);

                    if(ntohs(other_addr.sin_port)==PORT2)
                    printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                            "CLIENT","RECV",time_now(),"ACK",ack.sequence_number,"RELAY1","CLIENT");
                    else 
                    printf("%8s | %10s | %20s | %8s | %10d | %8s | %8s |\n",
                            "CLIENT","RECV",time_now(),"ACK",ack.sequence_number,"RELAY2","CLIENT");

                    if(ack.sequence_number>=window_begin)
                    {
                        int index = ((ack.sequence_number - window_begin)/PACKET_SIZE);
                        ack_keeper[index] = 1;

                        int all_set =1;
                        int met_last =0;

                        // for(int i=0;i<WINDOW_PACKETS;i++)
                        // printf("ack %d ",ack_keeper[i]);
                        // printf("\n");

                        // if(ack.last == 1)
                        // printf("Seq is last : %d\n",ack.sequence_number);

                        for (int i=0;i<WINDOW_PACKETS;i++)
                        {
                            if(ack_keeper[i]==0 && met_last==0)
                            {
                                all_set=0;
                                // printf("Got you\n");
                            }

                            if(window[i].last==1)
                            {
                                met_last=1;
                            }    
                        }

                        if(all_set==1)
                        {
                            
                            window_begin += window_size;
                            // printf("Move on to next window %d!\n",window_begin);
                            window_done = 1;
                            memset(ack_keeper,0,sizeof(ack_keeper));
                            memset(window,0,sizeof(window));
                        }

                    }
                
                }
                else
                {
                    // printf("Continuing..\n");
                    continue;
                }

            }
        }

        if(window_begin >=file_size)
            break;
        
    }

    close(my_socket);
    

}


