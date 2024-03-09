#include "kernel/types.h"

#include "kernel/stat.h"
#include "user/user.h"



#define MAX_NUM_RECEIVERS 10
#define MAX_MSG_SIZE 256



struct msg_t {
// this is the ID of the receiver that is intended
    int receiverId;

    char content[MAX_MSG_SIZE];
};


void panic(char *s) {

    fprintf(2, "%s\n", s);

    exit(1);

}


int fork1(void) {

    int pid;
    pid = fork();
    if (pid == -1)
        panic("fork");
    return pid;

}


void pipe1(int fd[2]) {

    int rc = pipe(fd);

    if (rc < 0) {

        panic("pipe creation failed");

    }

}


int main(int argc, char *argv[]) {

    if (argc != 4) {
        panic("Usage: unicast <num_of_receivers> <receiver_id> <msg_to_send>");
    }


    int numReceiver = atoi(argv[1]);

    int receiverId = atoi(argv[2]);

    char *message = argv[3];



    if (receiverId < 0 || receiverId >= numReceiver) {

        panic("reciever ID is out of range");

    }


    int channelToReceivers[2], channelFromReceivers[2];


    pipe1(channelToReceivers);
    pipe1(channelFromReceivers);


    for (int i = 0; i < numReceiver; i++) {

        int retFork = fork1();
        if (retFork == 0) {
            int myId = i;
            printf("Child %d: start!\n", myId);
            
            struct msg_t msg;

            read(channelToReceivers[0], (void *)&msg, sizeof(struct msg_t));
            

            if (myId == msg.receiverId) {
                printf("Child %d: the msg is for me.\n", myId);

                printf("Child %d acknowledges to Parent: received!\n", myId);

                write(channelFromReceivers[1], "received!", 10);
            } else {
                printf("Child %d: the msg is not for me.\n", myId);

                write(channelToReceivers[1], &msg, sizeof(struct msg_t));
            }

            exit(0);
        } else {
            printf("Parent: creates child process with id: %d\n", i);
        }
        sleep(1);


    }


    struct msg_t msg;
    msg.receiverId = receiverId;


    strcpy(msg.content, message);

    write(channelToReceivers[1], &msg, sizeof(struct msg_t));

    printf("Parent sends to Child %d: %s\n", receiverId, message);

    char recvBuf[10];
    read(channelFromReceivers[0], &recvBuf, sizeof(recvBuf));


    printf("Parent receives: %s\n", recvBuf);

    exit(0);
}
