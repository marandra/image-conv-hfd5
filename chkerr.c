#include <stdio.h>
#include <stdlib.h>
#include "chkerr.hh"

int flagerr(int err, const char *file, int line){
  switch(err) {
    case ERR: fprintf(stderr,"[ERROR] At %s:%d\n",file,line); break;
    case MEMERR: fprintf(stderr,"[ERROR] Allocation error at %s:%d\n",file,line); break;
    case ZERODIVERR: fprintf(stderr,"[ERROR] Division by zero at %s:%d\n",file,line); break;
    case NANERR: fprintf(stderr,"[ERROR] NaN at %s:%d\n",file,line); break;
    case FILEERR: fprintf(stderr,"[ERROR] Accessing file at %s:%d\n",file,line); break;
  }

  return ERR;
}

int flagmsg(const char* msg, const char *file, int line){
    fprintf(stderr,"[ERROR] %s at %s:%d\n", msg, file, line);

  return ERR;
}
