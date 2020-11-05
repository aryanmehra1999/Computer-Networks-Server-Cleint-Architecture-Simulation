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


int main(int argc, char *argv[])
{

    int number;
    srand(time(0));

    if(atoi(argv[1])==1)
        number = 1;
    else number = 2;

    int my_socket;
    if((my_socket =socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        printf("Error while creating relay socket 1");
        exit(0);
    }

    struct sockaddr_in my_addr,other_addr,server_addr,client_addr;

    memset((char *) &my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = (number==1)?htons(PORT2):htons(PORT3);
    my_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset((char *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT4);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    memset((char *) &client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(PORT1);
    client_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if(bind(my_socket , (struct sockaddr*)&my_addr, sizeof(my_addr) ) == -1)
    {
        printf("Error binding my address");
        exit(0);
    }

    printf("Socket and Binding are done..\n");

    printf("%8s | %10s | %20s | %8s | %10s | %8s | %8s |\n",
    "NODE","EVENT","TIME","TYPE","SEQ NUM","SOURCE","DEST");

    struct pollfd me[1];
    me[0].fd = my_socket;
    me[0].events = POLLIN;

    int timeout =  RELAY_TIMEOUT * 1000;

    while(1)
    {
        int value = poll(me,1,timeout);

        if(value==0)
            break;
        else if(value<0)
        {
            printf("polling error");
            exit(0);
        }
        else
        {
            if(me[0].revents==POLLIN)
            {
                packet ack;
                int addr_len = sizeof(other_addr);
                
                memset(&ack,0,sizeof(ack));
                recvfrom(me[0].fd,&ack,sizeof(ack),0,(struct sockaddr *)&other_addr,&addr_len);

                if(ntohs(other_addr.sin_port)==PORT1)
                {
                    // printf("Received data from Client: Seq Num %d\n",ack.sequence_number);
                    printf("%7s%1s | %10s | %20s | %8s | %10d | %8s | %7s%1s |\n",
                    "RELAY",argv[1],"RECV",time_now(),"DATA",ack.sequence_number,"CLIENT","RELAY",argv[1]);
                    
                    if(rand()%100>=DROP_PERCENTAGE)
                    {
                        int addr_len = sizeof(server_addr);
                        sendto(my_socket,&ack,sizeof(ack),0,(struct sockaddr *)&server_addr,addr_len);

                        usleep((rand()%2000));

                        printf("%7s%1s | %10s | %20s | %8s | %10d | %7s%1s | %8s |\n",
                        "RELAY",argv[1],"SEND",time_now(),"DATA",ack.sequence_number,"RELAY",argv[1],"SERVER");
                    }
                    else 
                    printf("%7s%1s | %10s | %20s | %8s | %10d | %8s | %7s%1s |\n",
                        "RELAY",argv[1],"DROP",time_now(),"DATA",ack.sequence_number,"------","------",argv[1]);

                }
                else if(ntohs(other_addr.sin_port)==PORT4)
                {
                    printf("%7s%1s | %10s | %20s | %8s | %10d | %8s | %7s%1s |\n",
                    "RELAY",argv[1],"RECV",time_now(),"ACK",ack.sequence_number,"SERVER","RELAY",argv[1]);
                    int addr_len = sizeof(server_addr);
                    sendto(my_socket,&ack,sizeof(ack),0,(struct sockaddr *)&client_addr,addr_len);
                    printf("%7s%1s | %10s | %20s | %8s | %10d | %7s%1s | %8s | \n",
                    "RELAY",argv[1],"SEND",time_now(),"ACK",ack.sequence_number,"RELAY",argv[1],"CLIENT");
                }
            }
            else
            {
                printf("Some other function other than pollin reading\n");
                break;
            }
            
        }
        
        
    }

    close(my_socket);




}