#ifndef CLIENT_H
#define CLIENT_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>

#include "utils.h"

void createClient(int clientID, int messageQueueID);
void pingServer(int clientID, int messageQueueID);
void fileSearch(int clientID, int messageQueueID, char fileName[]);
void cleanupClient(int clientID, int messageQueueID);

#endif
