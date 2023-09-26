#ifndef SERVER_H
#define SERVER_H

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "utils.h"

#define MAX_CLIENTS 90

void createClient(int messageQueueID, bool existingClients[],
                  struct MessageBuffer requestBuffer);
void pingResponse(int messageQueueID, struct MessageBuffer requestBuffer);
void fileSearch(int messageQueueID, struct MessageBuffer requestBuffer);
void fileWordCount(int messageQueueID, struct MessageBuffer requestBuffer);
void cleanupClient(int messageQueueID, bool existingClients[],
                  struct MessageBuffer requestBuffer);

#endif
