#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_INPUT_LEN 10
#define MSG_KEY 1234  // Key for the message queue
struct Message
{
    long mtype; // Message type
    int plane_id;
    int airport_departure;
    int airport_arrival;
    int total_weight;
    int plane_type; // 1 for passenger, 0 for cargo
    int num_passengers;
    char flag[2];
    char action;
    char cleanup;
};

int main()
{
    int msgid;
    char choice;
    key_t key;
    struct Message message;

    key = ftok("plane.c", 'A');
    msgid = msgget(key, 0666 | IPC_CREAT);
    while(1){
        printf("Do you want the Air traffic control system to terminate (Y for YES /N for NO)?\n");
        scanf("%c",&choice);

        if(choice =='Y' || choice =='y')
        {
            message.cleanup ='y';
            message.mtype=51;
            msgsnd(msgid, &message, sizeof(struct Message), 0);
            break;
        }else
            continue;
    }
    return 0;
}
