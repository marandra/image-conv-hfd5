int appendmetadata(char*, int*, int, char*, const char *);
int displaymetadata(char*, int);
int imageread(bim::Image, bim::TagMap*, char*, int);
int imagewrite(bim::Image, bim::TagMap, char*, const char*);

#define BUFFERSIZE 8192

