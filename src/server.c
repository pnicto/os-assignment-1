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
    if (msgrcv(messageQueueID, &messageBuffer, BUFFER_SIZE + sizeof(int), -9,
               0) == -1) {
      perror("Error receiving message in msgrcv");
      exit(1);
    }

    if (messageBuffer.mtype == 1) {
      // perform cleanup
    } else if (messageBuffer.mtype == 2) {
      // add client to existing clients list
    } else {
      printf("Serving request of type %ld\n", messageBuffer.mtype - 3);
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
        case 5:
          fileSearch(messageQueueID, messageBuffer);
          break;
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
  struct MessageBuffer responseBuffer;
  responseBuffer.mtype = requestBuffer.clientID;
  sprintf(responseBuffer.mtext, "hello");

  if (msgsnd(messageQueueID, &responseBuffer, BUFFER_SIZE + sizeof(int), 0) ==
      -1) {
    perror("Error responding to client request of type 1 in msgsnd");
    exit(1);
  }

  exit(0);
}

void fileSearch(int messageQueueID, struct MessageBuffer requestBuffer) {
  pid_t pid = fork();
  struct MessageBuffer responseBuffer;
  responseBuffer.mtype = requestBuffer.clientID;
  responseBuffer.clientID = -1;

  if (pid < 0) {
    perror("Error creating child process in fork");
    exit(1);
  }

  if (pid == 0) {
    // child
    execlp("/usr/bin/find", "find", requestBuffer.mtext, NULL);
    exit(1);
  } else {
    // parent
    int childStatus;
    wait(&childStatus);
    if (childStatus == 0) {
      // file exists
      snprintf(responseBuffer.mtext, sizeof(responseBuffer.mtext),
               "Found the requested file");
    } else {
      // file does not exist
      snprintf(responseBuffer.mtext, sizeof(responseBuffer.mtext),
               "File does not exist");
    }
  }

  if (msgsnd(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), 0) == -1) {
    perror("Error responding to client request of type 2 in msgsnd");
    exit(1);
  }
  exit(0);
}
