#include "../include/client.h"

int main() {
  int messageQueueID;
  key_t messageQueueKey;

  if ((messageQueueKey = ftok(PATHNAME, PROJ_ID)) == -1) {
    perror("Error generating key in ftok");
    exit(1);
  }

  if ((messageQueueID = msgget(messageQueueKey, PERMS)) == -1) {
    perror("Error connecting to message queue in msgget. Is the server on?");
    exit(1);
  }

  int clientID;
  printf("Enter Client-ID: ");
  scanf("%d", &clientID);
  clientID += 10;
  createClient(clientID, messageQueueID);

  while (1) {
    printf(
        "\n1. Enter 1 to contact the Ping Server\n2. Enter 2 to contact the "
        "File Search Server\n3. Enter 3 to contact the File Word Count "
        "Server\n4. Enter 4 if this Client wishes to exit\n");

    int choice;
    scanf("%d", &choice);

    switch (choice) {
      case 1:
        pingServer(clientID, messageQueueID);
        break;
      case 2: {
        printf("Enter the file name to search:\n");
        char fileName[BUFFER_SIZE];
        scanf(" %[^\n]s", fileName);
        fileSearch(clientID, messageQueueID, fileName);
      } break;
      case 3:
        printf("Enter the file path: ");
        char filePath[BUFFER_SIZE];
        scanf(" %[^\n]s", filePath);
        fileWordCount(clientID, messageQueueID, filePath);
        break;
      case 4:
        cleanupClient(clientID, messageQueueID);
        exit(0);
      default:
        printf("Invalid choice\n");
    }
  }
  return 0;
}

void createClient(int clientID, int messageQueueID) {
  struct MessageBuffer requestBuffer, responseBuffer;
  requestBuffer.mtype = 2;
  requestBuffer.clientID = clientID;

  if (msgsnd(messageQueueID, &requestBuffer,
             sizeof(requestBuffer) - sizeof(requestBuffer.mtype), 0) == -1) {
    perror("Error sending message in msgsnd");
    exit(1);
  }

  if (msgrcv(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), clientID,
             0) == -1) {
    perror("Error receiving message in msgrcv");
    exit(1);
  }

  printf("%s\n", responseBuffer.mtext);
  if (responseBuffer.clientID == 0) {
    exit(0);
  }
}

void pingServer(int clientID, int messageQueueID) {
  struct MessageBuffer requestBuffer, responseBuffer;
  requestBuffer.mtype = 4;
  requestBuffer.clientID = clientID;

  char* message = (char*)malloc(sizeof(char) * BUFFER_SIZE);
  sprintf(message, "%d", clientID);
  message = strcat(message, MESSSAGE_DELIMITER);
  message = strcat(message, "hi");

  sprintf(requestBuffer.mtext, "%s", message);
  free(message);

  if (msgsnd(messageQueueID, &requestBuffer,
             sizeof(requestBuffer) - sizeof(requestBuffer.mtype), 0) == -1) {
    perror("Error sending message in msgsnd");
    exit(1);
  }

  if (msgrcv(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), clientID,
             0) == -1) {
    perror("Error receiving message in msgrcv");
    exit(1);
  }

  printf("%s\n", responseBuffer.mtext);
}

void fileSearch(int clientID, int messageQueueID, char fileName[]) {
  struct MessageBuffer requestBuffer, responseBuffer;

  requestBuffer.mtype = 5;
  requestBuffer.clientID = clientID;
  snprintf(requestBuffer.mtext, sizeof(requestBuffer.mtext), "%s", fileName);

  if (msgsnd(messageQueueID, &requestBuffer,
             sizeof(requestBuffer) - sizeof(requestBuffer.mtype), 0) == -1) {
    perror("Error sending message in msgsnd");
    exit(1);
  }

  if (msgrcv(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), clientID,
             0) == -1) {
    perror("Error receiving message in msgrcv");
    exit(1);
  }

  printf("%s\n", responseBuffer.mtext);
}

void fileWordCount(int clientID, int messageQueueID, char filePath[]) {
  struct MessageBuffer requestBuffer, responseBuffer;

  requestBuffer.mtype = 6;
  requestBuffer.clientID = clientID;
  snprintf(requestBuffer.mtext, sizeof(requestBuffer.mtext), "%s", filePath);

  if (msgsnd(messageQueueID, &requestBuffer,
             sizeof(requestBuffer) - sizeof(requestBuffer.mtype), 0) == -1) {
    perror("Error sending message in msgsnd");
    exit(1);
  }

  if (msgrcv(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), clientID,
             0) == -1) {
    perror("Error receiving message in msgrcv");
    exit(1);
  }

  printf("%s\n", responseBuffer.mtext);
}

void cleanupClient(int clientID, int messageQueueID) {
  struct MessageBuffer requestBuffer, responseBuffer;
  requestBuffer.mtype = 3;
  requestBuffer.clientID = clientID;

  if (msgsnd(messageQueueID, &requestBuffer,
             sizeof(requestBuffer) - sizeof(requestBuffer.mtype), 0) == -1) {
    perror("Error sending message in msgsnd");
    exit(1);
  }

  if (msgrcv(messageQueueID, &responseBuffer,
             sizeof(responseBuffer) - sizeof(responseBuffer.mtype), clientID,
             0) == -1) {
    perror("Error receiving message in msgrcv");
    exit(1);
  }

  printf("%s\n", responseBuffer.mtext);
}
