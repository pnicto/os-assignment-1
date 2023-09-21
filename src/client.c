#include "../include/client.h"

int main() {
  int messageQueueID;
  key_t messageQueueKey;

  if ((messageQueueKey = ftok(PATHNAME, PROJ_ID)) == -1) {
    perror("Error generating key in ftok");
    exit(1);
  }

  if ((messageQueueID = msgget(messageQueueKey, PERMS)) == -1) {
    perror("Error connecting to message queue in msgget. Is the server on?\n");
    exit(1);
  }

  int clientID;
  printf("Enter Client-ID: ");
  scanf("%d", &clientID);
  clientID += 10;

  while (1) {
    printf(
        "1. Enter 1 to contact the Ping Server\n2. Enter 2 to contact the File "
        "Search Server\n3. Enter 3 to contact the File Word Count Server\n4. "
        "Enter 4 if this Client wishes to exit\n");

    int choice;
    scanf("%d", &choice);

    switch (choice) {
      case 1:
        pingServer(clientID, messageQueueID);
        break;
        //   case 2:
        //     fileSearch(clientID, messageQueueID);
        //     break;
        //   case 3:
        //     fileWordCount(clientID, messageQueueID);
        //     break;
        //   case 4:
        //     cleanup(clientID, messageQueueID);
        //     exit(0);
      default:
        printf("Invalid choice\n");
    }
  }
  return 0;
}

void pingServer(int clientID, int messageQueueID) {
  struct MessageBuffer requestBuffer, responseBuffer;
  requestBuffer.mtype = 4;

  char* message = (char*)malloc(sizeof(char) * BUFFER_SIZE);
  sprintf(message, "%d", clientID);
  message = strcat(message, MESSSAGE_DELIMITER);
  message = strcat(message, "hi");

  size_t messageLength = (size_t)sprintf(requestBuffer.mtext, "%s", message);
  free(message);

  if (msgsnd(messageQueueID, &requestBuffer, messageLength + 1, 0) == -1) {
    perror("Error sending message in msgsnd");
    exit(1);
  }

  if (msgrcv(messageQueueID, &responseBuffer, sizeof(responseBuffer.mtext),
             clientID, 0) == -1) {
    perror("Error receiving message in msgrcv");
    exit(1);
  }

  printf("%s\n", responseBuffer.mtext);
}