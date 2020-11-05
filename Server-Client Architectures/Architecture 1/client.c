/* 
ARYAN MEHRA 
2017A7PS0077P
*/

#include"packet.h"

int main()
{

    int chanel1,chanel2;
    if((chanel1 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
    {
        printf("error in socket\n");
        exit(0);
    }

    printf("Chanel 1 is established..\n");

    if((chanel2 = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP))<0)
    {
        printf("error in socket\n");
        exit(0);
    }

    printf("Chanel 2 is established..\n");

    struct sockaddr_in server_address;
    memset (&server_address,0,sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT); 
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1"); 

    if(connect(chanel1, (struct sockaddr*) &server_address , sizeof(server_address)) < 0 )
    {
        printf("error in connecting chanel 1");
        exit(0);
    }

    printf("Connection 1 is Established...\n");

    if(connect(chanel2, (struct sockaddr*) &server_address , sizeof(server_address)) < 0 )
    {
        printf("error in connecting chanel 2");
        exit(0);
    }

    printf("Connection 2 is Established...\n");

    int fd = open("input.txt",O_RDONLY);
    int file_size = lseek(fd,0,SEEK_END);
    lseek(fd,0,SEEK_SET);
    // int file_pointer = 0;

    //loop till the end of the file
    int done=0;
    while(1)
    {
        int legit1=0, legit2=0,last_acked=0;
        
        packet p1,p2;
        memset(&p1,0,sizeof(p1));
        p1.chanel = 1;
        p1.sequence_number = lseek(fd,0,SEEK_CUR);
        p1.is_data =1;
        p1.bytes_in_data = read(fd,p1.data,PACKET_SIZE);
        
        if(p1.sequence_number<file_size-PACKET_SIZE)
        p1.last=0;
        else p1.last=1;

        // file_pointer += p1.bytes_in_data;

        if(send(chanel1,&p1,sizeof(p1),0)<0)
        {
            printf("Error while sending on Chanel 1\n");
            exit(0);
        }

        printf("SENT PACKET: Sequence Number %d  of size %d bytes on chanel %d \n",
        p1.sequence_number, p1.bytes_in_data, p1.chanel);

        legit1=1;

        if(lseek(fd,0,SEEK_CUR)<file_size)
        {
            memset(&p2,0,sizeof(p2));
            p2.chanel = 2;
            p2.sequence_number = lseek(fd,0,SEEK_CUR);
            p2.is_data =1;
            p2.bytes_in_data = read(fd,p2.data,PACKET_SIZE);
            
            if(p2.sequence_number<file_size-PACKET_SIZE)
            p2.last=0;
            else p2.last=1;

            // file_pointer += p2.bytes_in_data;

            if(send(chanel2,&p2,sizeof(p2),0)<0)
            {
                printf("Error while sending on Chanel 2\n");
                exit(0);
            }

            printf("SENT PACKET: Sequence Number %d  of size %d bytes on chanel %d \n",
            p2.sequence_number, p2.bytes_in_data, p2.chanel);

            legit2=1;
        }

        struct pollfd me[2];
        memset(me,0,sizeof(me));
        me[0].fd = chanel1;
        me[0].events = POLLIN;
        me[1].fd = chanel2;
        me[1].events = POLLIN;

        int timeout = TIMEOUT_SECS * 1000;

        int closed1=0;
        int closed2=0;

        while(1)
        {
            int return_value = poll(me,2,timeout);

            if(return_value<0)
            {
                printf("Error in polling\n");
                exit(0);
            }
            else if(return_value==0)
            {
                //timeout
                //see legit and resend packets
                if(legit1==1)
                {
                    if(send(chanel1,&p1,sizeof(p1),0)<0)
                    {
                        printf("Error while sending on Chanel 1\n");
                        exit(0);
                    }

                    printf("(RE)SENT PACKET: Sequence Number %d  of size %d bytes on chanel %d \n",
                    p1.sequence_number, p1.bytes_in_data, p1.chanel);

                }
                
                if(legit2==1)
                {
                    if(send(chanel2,&p2,sizeof(p2),0)<0)
                    {
                        printf("Error while sending on Chanel 1\n");
                        exit(0);
                    }

                    printf("(RE)SENT PACKET: Sequence Number %d  of size %d bytes on chanel %d \n",
                    p2.sequence_number, p2.bytes_in_data, p2.chanel);

                }

                if(legit1==0 && legit2==0)
                {
                    done=1;
                    break;
                }

            }
            else
            {
                if(me[0].revents==POLLIN)
                {
                    //chanel1

                    packet ack;
                    memset(&ack,0,sizeof(ack));

                    int receive_value = recv(chanel1,&ack,sizeof(ack),0);

                    if(receive_value==0)
                    {
                        printf("Connection 1 is closed by server.\n");
                        closed1 = 1;
                        me[0].revents = 0;
                    }
                    else if(receive_value<0)
                    {
                        printf("Error in the receive function");
                        exit(0);
                    }
                    else
                    {
                        printf("RECEIVED ACK: Sequence number %d from chanel %d\n",
                        ack.sequence_number,ack.chanel);


                        if(ack.sequence_number == p1.sequence_number)
                        {
                            legit1=0;
                            if(lseek(fd,0,SEEK_CUR)<file_size)
                            {

                                memset(&p1,0,sizeof(p1));
                                p1.chanel = 1;
                                p1.sequence_number = lseek(fd,0,SEEK_CUR);
                                p1.is_data =1;
                                p1.bytes_in_data = read(fd,p1.data,PACKET_SIZE);
                                
                                if(p1.sequence_number<file_size-PACKET_SIZE)
                                p1.last=0;
                                else p1.last=1;

                                // file_pointer += p1.bytes_in_data;

                                if(send(chanel1,&p1,sizeof(p1),0)<0)
                                {
                                    printf("Error while sending on Chanel 1\n");
                                    exit(0);
                                }

                                printf("SENT PACKET: Sequence Number %d  of size %d bytes on chanel %d \n",
                                p1.sequence_number, p1.bytes_in_data, p1.chanel);

                                legit1=1;

                            }
                        }
                    }
                    

                }
                if(me[1].revents==POLLIN)
                {
                    //chanel2

                    packet ack;
                    memset(&ack,0,sizeof(ack));

                    int receive_value = recv(chanel2,&ack,sizeof(ack),0);

                    if(receive_value==0)
                    {
                        printf("Connection 2 is closed by server.\n");
                        closed2=1;
                        me[1].revents=0;
                    }
                    else if(receive_value<0)
                    {
                        printf("Error in the receive function");
                        exit(0);
                    }
                    else
                    {
                        printf("RECEIVED ACK: Sequence number %d from chanel %d\n",
                        ack.sequence_number,ack.chanel);

                        if(ack.sequence_number == p2.sequence_number)
                        {
                            
                            legit2=0;
                            if(lseek(fd,0,SEEK_CUR)<file_size)
                            {
                                // printf("here");
                                memset(&p2,0,sizeof(p2));
                                p2.chanel = 2;
                                p2.sequence_number = lseek(fd,0,SEEK_CUR);
                                p2.is_data =1;
                                p2.bytes_in_data = read(fd,p2.data,PACKET_SIZE);
                                
                                if(p2.sequence_number<file_size-PACKET_SIZE)
                                p2.last=0;
                                else p2.last=1;

                                // file_pointer += p2.bytes_in_data;

                                if(send(chanel2,&p2,sizeof(p2),0)<0)
                                {
                                    printf("Error while sending on Chanel 2\n");
                                    exit(0);
                                }

                                printf("SENT PACKET: Sequence Number %d  of size %d bytes on chanel %d \n",
                                p2.sequence_number, p2.bytes_in_data, p2.chanel);

                                legit2=1;

                            }
                        }
                    }
                }
                
            }

            if(closed1==1 && closed2==1)
            break;
            
        }


        if(lseek(fd,0,SEEK_CUR)>=file_size || done==1)
            break;

    }

    close(fd);
    close(chanel1);
    close(chanel2);

}