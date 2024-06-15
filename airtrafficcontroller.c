#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdbool.h>
#include<limits.h>
#define MAX_AIRPORTS 10
#define MAX_PLANES 10

// Message structure for communication with airport and plane processes
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
    freopen("/dev/null","w",stderr);
    int fd;
    char filename[] = "airtrafficcontroller.txt";
    char buffer[10000];
    fd=open(filename, O_WRONLY | O_CREAT,S_IRUSR | O_APPEND| S_IWUSR, 0644);
    if(fd==-1){
    perror("file open");
    return 1;
    }
    int active_planes = 0;
    int active_airports = 0;
    int num_airports;
    // Ask user for the number of airports
    printf("Enter the number of airports to be handled/managed: ");
    scanf("%d", &num_airports);
    if (num_airports < 2 || num_airports > MAX_AIRPORTS)
    {
        fprintf(stderr, "Invalid number of airports. Please enter a number between 2 and 10.\n");
        exit(EXIT_FAILURE);
    }

    // Message Queue variables
    key_t key;
    int msgid;

    // Create a message queue
    key = ftok("plane.c", 'A');
    msgid = msgget(key, 0666 | IPC_CREAT);
    if (msgid == -1)
    {
        perror("Error creating message queue");
        exit(EXIT_FAILURE);
    }

    //printf("Air Traffic Controller process is running...\n");
  
  
    int flags[10] = {};
    int cleanup_received = 0;
    struct Message messages[10];
  
    while (1)
    {
        struct Message message;
        
        for(int i=0;i<10;i++){
	if((flags[i]==3 || flags[i]==2) && msgrcv(msgid, &message, sizeof(struct Message), 11, IPC_NOWAIT)!=-1){ //msg from dept airport
	 	flags[i] -= 2;
	 	if(flags[i] == 0){
	     		flags[i] =4;
	     		message.mtype = 500+message.plane_id;
			msgsnd(msgid, &message, sizeof(struct Message), 0);
	     	}
	}
	else if((flags[i]==3 ||flags[i]==1) && msgrcv(msgid, &message, sizeof(struct Message), 21, IPC_NOWAIT)!=-1 ){ //msg from arrival airport
	    flags[i] -= 1;
	    if(flags[i] == 0){
	     	flags[i] =4;
	     	message.mtype = 500+message.plane_id;
		msgsnd(msgid, &message, sizeof(struct Message), 0);
	     }
	}
	else if(flags[i] == 4 && msgrcv(msgid, &message, sizeof(struct Message), 600+messages[i].plane_id,IPC_NOWAIT)!=-1){
		active_planes--;
		flags[i] = 0;
        messages[i].mtype=69;
        msgsnd(msgid, &messages[i], sizeof(struct Message), 0);	
	}
	}
        
        if (msgrcv(msgid, &message, sizeof(struct Message), 1, IPC_NOWAIT)==-1 )
        {
            if (cleanup_received == 1 || msgrcv(msgid, &message, sizeof(struct Message), 51, IPC_NOWAIT)!=-1 )
        {
        	cleanup_received = 1;
        	
            if(active_planes==0)
            { 
                for(int i=1000;i<=10000;i+=1000){
                    message.mtype = i;
                    message.airport_arrival = 0;
                    message.airport_departure = 0;
                    msgsnd(msgid, &message, sizeof(struct Message), 0);
                }
                break;
            }
        }
        continue;
        }
        
            if (message.plane_type == 1)
            { // Passenger plane
                //printf("Air Traffic Controller: Received message for passenger plane %d\n", message.plane_id);
                // Departure logic
                
                message.mtype = 1000*message.airport_departure;
                message.flag[0] = 'a';
                message.flag[1] = message.airport_departure + '0';
                message.action = 'd';
                //printf("Informing departure airport to begin boarding/loading and takeoff process for plane %d\n", message.plane_id);
                msgsnd(msgid, &message, sizeof(struct Message), 0); //msg to dept. airport
               // printf("message sent to departure airport\n");
                active_planes++;
                
                message.flag[1] = message.airport_arrival+'0';
                message.action = 'a';
                message.mtype = 1000*message.airport_arrival;
                printf("Plane %d has departed from airport %d and will land at airport %d\n",message.plane_id,message.airport_departure,message.airport_arrival);
                dprintf(fd, "Plane %d has departed from airport %d and will land at airport %d\n",message.plane_id,message.airport_departure,message.airport_arrival);
                msgsnd(msgid, &message, sizeof(struct Message), 0);//message sent to arrival airport
                //printf("message sent to arrival airport\n");
                
		flags[message.plane_id - 1] = 3;
		messages[message.plane_id - 1] = message;
            }else if (message.plane_type == 0)
            { // csargo plane
                //printf("Air Traffic Controller: Received message for cargo plane %d\n", message.plane_id);
                // Departure logic
                
                message.mtype = 1000*message.airport_departure;
                message.flag[0] = 'a';
                message.flag[1] = message.airport_departure + '0';
                message.action = 'd';
                //printf("Informing departure airport to begin boarding/loading and takeoff process for plane %d\n", message.plane_id);
                msgsnd(msgid, &message, sizeof(struct Message), 0); //msg to dept. airport
                //printf("message sent to departure airport\n");
                active_planes++;
                
                message.flag[1] = message.airport_arrival+'0';
                message.action = 'a';
                message.mtype = 1000*message.airport_arrival;
                printf("Plane %d has departed from airport %d and will land at airport %d\n",message.plane_id,message.airport_departure,message.airport_arrival);
                dprintf(fd, "Plane %d has departed from airport %d and will land at airport %d\n",message.plane_id,message.airport_departure,message.airport_arrival);
                msgsnd(msgid, &message, sizeof(struct Message), 0);//message sent to arrival airport
               // printf("message sent to arrival airport\n");
                
		flags[message.plane_id - 1] = 3;
		messages[message.plane_id - 1] = message;
            }
	
        }
    close(fd);
    msgctl(msgid, IPC_RMID, NULL);
    return 0;
}

