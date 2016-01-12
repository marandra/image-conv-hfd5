#define CHKERR(err)  if (err) return flagerr(err, __FILE__, __LINE__)
#define CHKRTN(err,ferr)  if (err) return flagerr(ferr, __FILE__, __LINE__)
#define CHKRTNMSG(err,msg)  if (err) return flagmsg(msg, __FILE__, __LINE__)
#define RTNERR(err) return flagerr(err, __FILE__, __LINE__)
#define FNAME FUNCNAME": "

enum err_codes {
  NOERR=EXIT_SUCCESS,
  ERR=EXIT_FAILURE,
  MEMERR=2,
  FILEERR=3,
  ZERODIVERR=4,
  NANERR=5
};

int flagerr(int err, const char *file, int line);
int flagmsg(const char* msg, const char *file, int line);
