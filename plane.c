#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#define MAX_PLANES 10
#define MAX_PASSENGERS 10
#define MAX_CARGO_ITEMS 100
#define AVG_CREW_WEIGHT 75
#define MAX_LUGGAGE_WEIGHT 25

// Define struct Passenger before it's used
struct Passenger
{
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
    char action;
    char cleanup;
};



void create_passenger(int plane_id, int pipe_fd[][2], struct Passenger *passengers, int i, int *crew_weight) {
    pid_t pid = fork();
    if (pid == -1) {
        perror("Error in creating fork");
        exit(-1);
    } else if (pid == 0) {
        // child process
        close(pipe_fd[i][0]); // Close read end of the pipe
        int pass_details[2] = {0}; // Array to hold luggage weight and body weight
        printf("Enter Weight of your Luggage (for Passenger) %d: ", i + 1);
        scanf("%d", &pass_details[0]); // Read luggage weight
        printf("Enter your Body Weight (for Passenger) %d: ", i + 1);
        scanf("%d", &pass_details[1]); // Read body weight
        write(pipe_fd[i][1], pass_details, sizeof(pass_details)); // Write passenger details to the pipe
        close(pipe_fd[i][1]); // Close write end of the pipe
        exit(0); // Child process exits
    } else {
        // parent process
        close(pipe_fd[i][1]); // Close write end of the pipe
        int pass_details[2] = {0}; // Array to hold received passenger details
        read(pipe_fd[i][0], pass_details, sizeof(pass_details)); // Read passenger details from the pipe
        passengers[i].body_weight = pass_details[1]; // Assign body weight to passenger
        passengers[i].luggage_weight = pass_details[0]; // Assign luggage weight to passenger
        *crew_weight += passengers[i].body_weight + passengers[i].luggage_weight;
        close(pipe_fd[i][0]); // Close read end of the pipe
        waitpid(pid, NULL, 0); // Wait for child process to exit
    }
}


int main()
{
    int plane_id, plane_type, num_seats;
    int num_cargo_items, avg_cargo_weight;
    int pipe_fd[MAX_PASSENGERS][2]; // Array of pipes for each passenger
    struct Passenger passengers[MAX_PASSENGERS];
    int crew_weight=0;
    int departure_airport, arrival_airport;

    // Message Queue variables
    key_t key;
    int msgid;
    struct Message message;

    key = ftok("plane.c", 'A');
    msgid = msgget(key, 0666 | IPC_CREAT);
    message.mtype = 1;

    printf("Enter Plane ID: ");
    scanf("%d", &plane_id);

    if (plane_id < 1 || plane_id > MAX_PLANES)
    {
        fprintf(stderr, "Invalid plane ID. Please enter a number between 1 and 10.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter Type of Plane (1 for Passenger, 0 for Cargo): ");
    scanf("%d", &plane_type);

    if (plane_type != 0 && plane_type != 1)
    {
        fprintf(stderr, "Invalid plane type. Please enter either 1 for Passenger or 0 for Cargo.\n");
        exit(EXIT_FAILURE);
    }

    if (plane_type == 1)
    { // Passenger plane
        printf("Enter Number of Occupied Seats: ");
        scanf("%d", &num_seats);
        if (num_seats < 1 || num_seats > MAX_PASSENGERS)
        {
            fprintf(stderr, "Invalid number of occupied seats. Please enter a number between 1 and 10.\n");
            exit(EXIT_FAILURE);
        }
        // Input luggage and body weights for each passenger

        for (int i = 0; i < num_seats; i++)
        {
            int pipefd[10][2];
            if (pipe(pipe_fd[i]) == -1)
            {
                perror("pipe");
                exit(-1);
            }
            create_passenger(plane_id, pipe_fd, passengers, i, &crew_weight);

             
        }


    }

    else
    { // Cargo plane
        printf("Enter Number of Cargo Items: ");
        scanf("%d", &num_cargo_items);
        if (num_cargo_items < 1 || num_cargo_items > MAX_CARGO_ITEMS)
        {
            fprintf(stderr, "Invalid number of cargo items. Please enter a number between 1 and 100.\n");
            exit(EXIT_FAILURE);
        }
        printf("Enter Average Weight of Cargo Items: ");
        scanf("%d", &avg_cargo_weight);
        if (avg_cargo_weight < 1 || avg_cargo_weight > 100)
        {
            fprintf(stderr, "Invalid average weight of cargo items. Please enter a number between 1 and 100.\n");
            exit(EXIT_FAILURE);
        }
        crew_weight=crew_weight+ (2 * AVG_CREW_WEIGHT) + (num_cargo_items * avg_cargo_weight); // 2 pilots only
    }

        crew_weight=crew_weight+(7 * AVG_CREW_WEIGHT);
        printf("total wt %d\n",crew_weight);

    printf("Enter Airport Number for Departure: ");
    scanf("%d", &departure_airport);
    if (departure_airport < 1 || departure_airport > 10)
    {
        fprintf(stderr, "Invalid departure airport number. Please enter a number between 1 and 10.\n");
        exit(EXIT_FAILURE);
    }

    printf("Enter Airport Number for Arrival: ");
    scanf("%d", &arrival_airport);
    if (arrival_airport < 1 || arrival_airport > 10 || arrival_airport == departure_airport)
    {
        fprintf(stderr, "Invalid arrival airport number. Please enter a number between 1 and 10 (excluding departure airport).\n");
        exit(EXIT_FAILURE);
    }

    // Prepare message to send to air traffic controller
    message.plane_id = plane_id;
    message.airport_departure = departure_airport;
    message.airport_arrival = arrival_airport;
    message.plane_type = plane_type;
    message.num_passengers = (plane_type == 1) ? num_seats : 0; // For cargo plane, num_passengers is 0
    message.total_weight = crew_weight;
    message.flag[0] = 'A';
    message.flag[1] = '1';
    message.action = 'x';
    message.mtype = 1;
    // Send message to air traffic controller
    msgsnd(msgid, &message, sizeof(struct Message), 0);
   // printf("message sent\n");

    // Display departure message
    

    // Simulate boarding/loading process
    sleep(3);

    // Simulate plane journey
    sleep(30);

    // Simulate deboarding/unloading process
    sleep(3);
 
    while (1)
    {
        struct Message message;
        if (msgrcv(msgid, &message, sizeof(struct Message), 500+plane_id, 0)==-1 )
        {
             continue;
        }
                //printf("message recieved\n");
                printf("Plane %d has successfully traveled from Airport %d to Airport %d!\n", plane_id, departure_airport, arrival_airport);
                message.mtype=600+plane_id;
                msgsnd(msgid, &message, sizeof(struct Message), 0);
                return 0;       
    }
    return 0;
}

