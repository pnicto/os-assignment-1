#include "../include/cleanup.h"

int main() {
  char input;
  printf(
      "Do you want the server to terminate? Press Y for Yes and N for No.\n");
  scanf("%c", &input);

  if (input == 'Y' || input == 'y') {
    int messageQueueID;
    key_t messageQueueKey;

    if ((messageQueueKey = ftok(PATHNAME, PROJ_ID)) == -1) {
      perror("Error generating key in ftok");
      exit(1);
    }

    if ((messageQueueID = msgget(messageQueueKey, PERMS)) == -1) {
      perror(
          "Error connecting to message queue in msgget. Is the server on?\n");
      exit(1);
    }

    struct MessageBuffer requestBuffer;

    requestBuffer.mtype = 1;
    requestBuffer.clientID = -1;

    if (msgsnd(messageQueueID, &requestBuffer,
               sizeof(requestBuffer) - sizeof(requestBuffer.mtype), 0) == -1) {
      perror("Error sending message in msgsnd");
      exit(1);
    }
  } else if (input == 'N' || input == 'n') {
    return 0;
  } else {
    printf("Invalid input\n");
  }

  return 0;
}
