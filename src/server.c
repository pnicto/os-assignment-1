#include "../include/server.h"

int main() {
  printf("Initializing server...\n");
  // bool existingClients[MAX_CLIENTS];

  // for (int i = 0; i < MAX_CLIENTS; i++) {
  //   existingClients[i] = false;
  // }

  int messageQueueID;
  key_t messageQueueKey;

  if ((messageQueueKey = ftok(PATHNAME, PROJ_ID)) == -1) {
    perror("Error generating key in ftok");
    exit(1);
  }

  if ((messageQueueID = msgget(messageQueueKey, PERMS | IPC_CREAT)) == -1) {
    perror("Error creating message queue in msgget");
    exit(1);
  }

  printf("Server initialized. Listening for requests.\n");

  while (1) {
    struct MessageBuffer messageBuffer;
    if (msgrcv(messageQueueID, &messageBuffer, sizeof(messageBuffer.mtext), -9,
               0) == -1) {
      perror("Error receiving message in msgrcv");
      exit(1);
    }

    if (messageBuffer.mtype == 1) {
      // perform cleanup
    } else if (messageBuffer.mtype == 2) {
      // add client to existing clients list
    } else {
      pid_t pid = fork();

      if (pid < 0) {
        perror("Error creating child process in fork");
        exit(1);
      }

      if (pid == 0) {
        switch (messageBuffer.mtype) {
            // case 3:
            // remove client from existing clients list
            // break;
          case 4:
            pingResponse(messageQueueID, messageBuffer);
            break;
          // case 5:
          //   fileSearch(messageQueueID, messageBuffer);
          //   break;
          // case 6:
          //   fileWordCount(messageQueueID, messageBuffer);
          //   break;
          default:
            break;
        }
      }
    }
  }

  return 0;
}

void pingResponse(int messageQueueID, struct MessageBuffer requestBuffer) {
  char *message = requestBuffer.mtext;
  char *clientIDString = strtok(message, MESSSAGE_DELIMITER);

  int clientID = atoi(clientIDString);

  struct MessageBuffer messageBuffer;
  messageBuffer.mtype = clientID;
  size_t messageLength = (size_t)sprintf(messageBuffer.mtext, "hello");

  if (msgsnd(messageQueueID, &messageBuffer, messageLength + 1, 0) == -1) {
    perror("Error responding to client request of type 1 in msgsnd");
    exit(1);
  }

  exit(0);
}