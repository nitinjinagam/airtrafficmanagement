//Jai Siya Ram
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
#include <semaphore.h>


#define MAX_PLANES 10
#define MAX_AIRPORTS 10
#define MAX_PASSENGERS 10
#define MAX_CARGO_ITEMS 100
#define AVG_CREW_WEIGHT 75
#define MAX_LUGGAGE_WEIGHT 25
int msgid;

struct Passenger {
    int luggage_weight;
    int body_weight;
};

// Message structure for communication with air traffic controller

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
    char cleanup;
    char action;
};

// Global variables for runways
int runways;
int runwayArr[MAX_AIRPORTS + 1];
sem_t runway_lock[MAX_AIRPORTS + 1]; // Semaphores for runway synchronization

void *handlePlaneDept(void *message)
{
    struct Message* planedetails= message;
    int bestFitRunway=-1;
    int minExcessCapacity = 15000;
    int id=planedetails->plane_id;
    int apno=planedetails->airport_departure;
    for(int i=0;i<runways+1;i++)
    {
        if(runwayArr[i]>=planedetails->total_weight)
        {
            int excess = runwayArr[i]-planedetails->total_weight;
            if(excess<minExcessCapacity){
                minExcessCapacity=excess;
                bestFitRunway=i;
            }
        }
    }
    //lock runway
    sem_wait(&runway_lock[bestFitRunway]);
    //simulate boarding/loading
    sleep(3);
    
    sleep(2);
   
    printf("Plane %d had completed borading/loading and taken off from runway no. %d of Airport No. %d. \n", id,bestFitRunway+1,apno);

    //uncook runway
    sem_post(&runway_lock[bestFitRunway]);
    
    pthread_exit(NULL);
}

void *handlePlaneArrival(void *message)
{
    struct Message* planedetails= message;
    int bestFitRunway=-1;
    int minExcessCapacity = 15000;
    int id=planedetails->plane_id;
    int apno=planedetails->airport_arrival;
    for(int i=0;i<runways+1;i++)
    {
        if(runwayArr[i]>=planedetails->total_weight)
        {
            int excess = runwayArr[i]-planedetails->total_weight;
            if(excess<minExcessCapacity){
                minExcessCapacity=excess;
                bestFitRunway=i;
            }
        }
    }
    //lock runway
    sem_wait(&runway_lock[bestFitRunway]);
    //simulate landing
    sleep(2);
    //simuate debaording
    sleep(2);
    if (msgrcv(msgid, planedetails, sizeof(struct Message), 69, 0)!=-1 )
    printf("Plane %d had landed on runway no. %d of and has completed deboarding/unloading on Airport No. %d. \n", id,bestFitRunway+1,apno);
    
    sem_post(&runway_lock[bestFitRunway]);
    pthread_exit(NULL);
} 

int main() {
    
   
    int airport;
    key_t key;
    struct Message message;

    key = ftok("plane.c", 'A');
    msgid = msgget(key, 0666 | IPC_CREAT);
    

    printf("Enter Airport Number: ");
    scanf("%d", &airport);
    if (airport < 1 || airport > MAX_AIRPORTS) {
        fprintf(stderr, "Invalid Airport number. Please enter a number between 1 and 10.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter number of Runways: ");
    scanf("%d", &runways);
    if (runways < 1 || runways > MAX_AIRPORTS) {
        fprintf(stderr, "Invalid runway number. Please enter a number between 1 and 10.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter loadCapacity of Runways (give as a space separated list in a single line): ");
    for (int i = 0; i < runways; i++) {
        scanf("%d", &runwayArr[i]);
    }
    freopen("/dev/null","w",stderr);
    // Backup runway
    runwayArr[runways] = 15000;

    for(int i=0;i<runways+1;i++)
    {
        sem_init(&runway_lock[i], 0, 1); // Initialize semaphore for runway
    }

    while(1)
    {
            struct Message message;
            
            if (msgrcv(msgid, &message, sizeof(struct Message), 1000*airport, 0)==-1)
            {
                continue;
            }
            if(message.airport_departure==airport)
            {
                //Handle Plane dept.
                struct Message* msgptr = &message;
                pthread_t thread;
                if(pthread_create(&thread,NULL,handlePlaneDept,msgptr)!=0){
                    perror("pthread_create");
                    exit(1);
                }
                 message.mtype=11;
                    //sending msg to atc
                msgsnd(msgid, &message, sizeof(struct Message), 0);
                pthread_detach(thread);
            }
            else if(message.airport_arrival==airport)
            {
                //handle arrival
                struct Message* msgptr = &message;
                pthread_t thread;
                if(pthread_create(&thread,NULL,handlePlaneArrival,msgptr)!=0){
                    perror("pthread_create");
                    exit(1);
                }
                message.mtype=21;
                    //sending msg to atc
                msgsnd(msgid, &message, sizeof(struct Message), 0);
                pthread_detach(thread);
            }else if(message.airport_departure==0 && message.airport_arrival == 0){
                break;
            }
       }
        
        for(int i=0;i<runways+1;i++)
            {
                sem_destroy(&runway_lock[i]);
            }
        return 0;
    }
