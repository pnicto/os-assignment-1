#ifndef UTILS_H
#define UTILS_H

#define PERMS 0644
#define PATHNAME "./src/server.c"
#define PROJ_ID 'C'
#define MESSSAGE_DELIMITER ";"
#define BUFFER_SIZE 200

struct MessageBuffer {
  long mtype;
  char mtext[BUFFER_SIZE];
};

#endif