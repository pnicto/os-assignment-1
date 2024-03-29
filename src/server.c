#include "../include/server.h"

int main() {
  printf("Initializing server...\n");
  bool existingClients[MAX_CLIENTS] = {0};

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
    if (msgrcv(messageQueueID, &messageBuffer,
               sizeof(messageBuffer) - sizeof(messageBuffer.mtype), -9,
               0) == -1) {
      perror("Error receiving message in msgrcv");
      if (errno != EIDRM) {
        printf("Attempting to cleanup...\n");
        cleanupServer(messageQueueID);
      }
      exit(1);
    }

    if (messageBuffer.mtype == 1) {
      // perform cleanup
      cleanupServer(messageQueueID);

      printf("Exiting...\n");
      exit(0);
    } else if (messageBuffer.mtype == 2) {
      createClient(messageQueueID, existingClients, messageBuffer);
    } else if (messageBuffer.mtype == 3) {
      printf("Serving request of type 4 from client %d\n",
             messageBuffer.clientID - 10);
      cleanupClient(messageQueueID, existingClients, messageBuffer);
    } else {
      printf("Serving request of type %ld from client %d\n",
             messageBuffer.mtype - 3, messageBuffer.clientID - 10);
      pid_t pid = fork();

      if (pid < 0) {
        perror("Error creating child process in fork");
        exit(1);
      }

      if (pid == 0) {
        switch (messageBuffer.mtype) {
          case 4:
            pingResponse(messageQueueID, messageBuffer);
            break;
          case 5:
            fileSearch(messageQueueID, messageBuffer);
            break;
          case 6:
            fileWordCount(messageQueueID, messageBuffer);
            break;
          default:
            break;
        }
      }
    }
  }

  return 0;
}

void createClient(int messageQueueID, bool existingClients[],
                  struct MessageBuffer requestBuffer) {
  struct MessageBuffer responseBuffer;
  responseBuffer.mtype = requestBuffer.clientID;
  responseBuffer.clientID = 0;
  int invalidMessageType = 0;

  printf("Serving create client request for clientID %d\n",
         requestBuffer.clientID - 10);

  if (requestBuffer.clientID > MAX_CLIENTS - 11 ||
      requestBuffer.clientID < 11) {
    invalidMessageType = 1;
  } else if (existingClients[requestBuffer.clientID - 11] == true) {
    invalidMessageType = 2;
  }

  if (invalidMessageType == 0) {
    responseBuffer.clientID = -1;
    sprintf(responseBuffer.mtext, "Client connected to server successfully");
    existingClients[requestBuffer.clientID - 11] = true;
  } else if (invalidMessageType == 1) {
    sprintf(responseBuffer.mtext,
            "Invalid ClientID, client ID needs to range between 1 and %d",
            MAX_CLIENTS);
  } else {
    sprintf(responseBuffer.mtext,
            "Another client with the same ClientID already exists, please use "
            "another ClientID");
  }

  if (msgsnd(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), 0) == -1) {
    perror("Error responding to create client request in msgsnd");
  }
}

void pingResponse(int messageQueueID, struct MessageBuffer requestBuffer) {
  printf("Client %d says %s\n", requestBuffer.clientID - 10,
         requestBuffer.mtext);

  struct MessageBuffer responseBuffer;
  responseBuffer.mtype = requestBuffer.clientID;
  responseBuffer.clientID = -1;
  sprintf(responseBuffer.mtext, "hello");

  if (msgsnd(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), 0) == -1) {
    perror("Error responding to client request of type 1 in msgsnd");
    exit(1);
  }

  exit(0);
}

void fileSearch(int messageQueueID, struct MessageBuffer requestBuffer) {
  pid_t pid = fork();

  if (pid < 0) {
    perror("Error creating child process in fork");
    exit(1);
  }

  if (pid == 0) {
    // child
    int null = open("/dev/null", O_WRONLY);
    dup2(null, 1);
    dup2(null, 2);
    close(null);
    execlp("/usr/bin/find", "find", requestBuffer.mtext, NULL);
    exit(1);
  }
  // parent
  struct MessageBuffer responseBuffer;
  responseBuffer.mtype = requestBuffer.clientID;
  responseBuffer.clientID = -1;
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

  if (msgsnd(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), 0) == -1) {
    perror("Error responding to client request of type 2 in msgsnd");
    exit(1);
  }
  exit(0);
}

void fileWordCount(int messageQueueID, struct MessageBuffer requestBuffer) {
  int fd[2];
  if (pipe(fd) == -1) {
    perror("Error creating pipe");
    exit(1);
  }

  pid_t pid = fork();

  if (pid < 0) {
    perror("Error creating child process in fork");
    exit(1);
  }

  if (pid == 0) {
    // child
    fflush(stdout);
    fflush(stderr);

    int devNull = open("/dev/null", O_WRONLY);

    dup2(fd[1], STDOUT_FILENO);
    dup2(devNull, STDERR_FILENO);

    close(fd[0]);
    close(fd[1]);
    execlp("/usr/bin/wc", "wc", "-w", requestBuffer.mtext, NULL);
    exit(1);
  }
  // parent
  struct MessageBuffer responseBuffer;
  responseBuffer.mtype = requestBuffer.clientID;
  responseBuffer.clientID = -1;
  int childStatus;
  wait(&childStatus);
  close(fd[1]);
  if (childStatus == 0) {
    // file exists
    char buf[BUFFER_SIZE];
    read(fd[0], &buf, sizeof(buf));
    close(fd[0]);

    for (int i = 0; i < BUFFER_SIZE; i++)
      if (buf[i] == ' ') {
        buf[i] = 0;
        break;
      }

    int numOfWords = atoi(buf);

    snprintf(responseBuffer.mtext, sizeof(responseBuffer.mtext),
             "Number of words in the file: %d", numOfWords);

  } else {
    // file does not exist
    close(fd[0]);
    snprintf(responseBuffer.mtext, sizeof(responseBuffer.mtext),
             "File does not exist");
  }
  if (msgsnd(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), 0) == -1) {
    perror("Error responding to client request of type 3 in msgsnd");
    exit(1);
  }
  exit(0);
}

void cleanupClient(int messageQueueID, bool existingClients[],
                   struct MessageBuffer requestBuffer) {
  struct MessageBuffer responseBuffer;
  responseBuffer.mtype = requestBuffer.clientID;
  responseBuffer.clientID = -1;

  sprintf(responseBuffer.mtext, "Client with clientID %d shutting down",
          requestBuffer.clientID - 10);
  existingClients[requestBuffer.clientID - 11] = false;

  if (msgsnd(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), 0) == -1) {
    perror("Error responding to create client request in msgsnd");
  }
}

void cleanupServer(int messageQueueID) {
  // wait for all the children to terminate
  printf("Cleaning up...\n");

  while (1) {
    pid_t childPid = wait(NULL);
    if (childPid > 0) {
      printf("Waited for child process with pid %d\n", childPid);
    } else if (childPid == -1) {
      if (errno == ECHILD) {
        printf("No more children to wait for\n");
        break;
      } else {
        perror("Error waiting for child process");
        exit(1);
      }
    }
  }

  printf("Removing message queue...\n");
  if (msgctl(messageQueueID, IPC_RMID, NULL) == -1) {
    perror("Removing queue failed");
    exit(1);
  }
}