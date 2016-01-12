#include <stdlib.h>
#include <string.h>
#include "meta_format_manager.h"
#include "imagereadwrite.hh"
#include "chkerr.hh"


//Temporary workaround for sharing the commets outside library //marcelo
#include "jp2/globalvariables.h"
//char commentline[8192];
//int commentlinelen;
/////////////////////

/*****************************************************************/
int mopen(char *cmt, int* plenc, int bufsize) {

  char data[] = "<metadata>";
  int lenc = *plenc;
  int lenl = strlen(data);

  if (lenc + lenl >= bufsize) return MEMERR;

  strncat(cmt, data, lenl);
  *plenc = lenc + lenl;

  return EXIT_SUCCESS;
}

int mclose(char *cmt, int* plenc, int bufsize) {

  char data[] = "</metadata>";
  int lenc = *plenc;
  int lenl = strlen(data);

  if (lenc + lenl >= bufsize) return MEMERR;

  strncat(cmt, data, lenl);
  *plenc = lenc + lenl;

  return EXIT_SUCCESS;
}

int mappend(char *cmt, int* plenc, int bufsize, const char *data) {

  char otag[] = "<item>";
  char ctag[] = "</item>";
  int lenc = *plenc;
  int lenl = strlen(otag) + strlen(data) + strlen(ctag);

  if (lenc + lenl >= bufsize) return MEMERR;

  strncat(cmt, otag, strlen(otag));
  strncat(cmt, data, strlen(data));
  strncat(cmt, ctag, strlen(ctag));
  *plenc = lenc + lenl;

  return EXIT_SUCCESS;
}

/*****************************************************************/
//int appendmetadata(char *cmt, int* plen, int bufsize, char *type, const char *data) {
//
//  int lenc = *plen;
//  int lent = strlen(type);
//  int lend = strlen(data);
//  int lenl = lent + 1 + lend + 1;
//
//  if (lenc + lenl >= bufsize) return MEMERR;
//
//  char* pline = (char*)malloc(lenl * sizeof(char));
//  char psp[] = " ";
//  strncpy(pline, type, lent);
//  strncpy(pline + lent, psp, strlen(psp));
//  strncpy(pline + lent + strlen(psp), data, lend);
//  pline[lenl - 1] = '\0';
//
//  memcpy(cmt + lenc + 1, pline, lenl);
//
//  *plen = lenc + lenl;
//
//  free(pline);
//
//  return EXIT_SUCCESS;
//}
//
/*****************************************************************/
//int displaymetadata(char *cmt, int cmtsz) {
//
//  if (cmtsz > BUFFERSIZE) {
//      fprintf(stderr, "error: %d > %d Increase metadata buffersize\n", cmtsz, BUFFERSIZE);
//      return EXIT_FAILURE;
//  }
//
//  char *s;
//  unsigned short r;
//  unsigned long len;
//  
//  s = cmt;
//  
//  while(*s) {
//      while(*s == ' ') ++s;
//      r = (unsigned short)atoi(s); 
//      while(isdigit(*s)) ++s;
//      while(*s == ' ') ++s;
//      
//      len = strlen(s);
//      
//      if(len > 0 && len < 65532)//65531 + 4 --> 65535
//      {
//	  fprintf(stdout, "    R(%d) T[%lu](%s)\n",r, len, s);
///*-------------------------
//	write(0xff64, 2);//COM
//	write(len+4, 2);
//	write(r, 2);
//	for(i = 0; i < len; ++i)
//	 write(s[i], 1);
//-------------------------*/
//      }
//      s += len + 1;
//  }
//
//  return EXIT_SUCCESS;
//}
//
/*****************************************************************/
int imageread(bim::Image img, bim::TagMap *pmetadata, char *fname, int page){

  int err;
  int num_pages;
  char *fmt_name;
  bim::MetaFormatManager fm;
  double data[4];
  const char *chardata;
  std::string text_metadata;

  err = fm.sessionStartRead( fname ); CHKRTN(err, FILEERR);

  err = fm.sessionReadImage( img.imageBitmap(), page );
  CHKRTNMSG(err, "Error reading image");

  num_pages = fm.sessionGetNumberOfPages();
  fmt_name = fm.sessionGetFormatName();

  /* getting metadata */
  fm.sessionParseMetaData(page);
  data[0] = fm.getPixelSizeX();
  data[1] = fm.getPixelSizeY();
  data[2] = fm.getPixelSizeZ();
  data[3] = fm.getPixelSizeT();
  chardata = fm.getImagingTime();
  *pmetadata = fm.get_metadata();
  text_metadata = fm.get_text_metadata();

  #ifdef DEBUG
  fprintf(stdout, "\n[DEBUG] Parameters from format manager:\n");
  fprintf(stdout, "    Nr pages: %d \n", num_pages);
  fprintf(stdout, "    Format: %s \n", fmt_name);
  fprintf(stdout, "    PixelSize X: %.8f\n", data[0]);
  fprintf(stdout, "    PixelSize Y: %.8f\n", data[1]);
  fprintf(stdout, "    PixelSize Z: %.8f\n", data[2]);
  fprintf(stdout, "    PixelSize T: %.8f\n", data[3]);
  fprintf(stdout, "    ImagingTime %s\n", chardata);
  fprintf(stdout, "    Text metadata: %s\n", text_metadata.c_str());
  #endif /* DEBUG */

  fm.sessionEnd();

  // make sure red image is in supported pixel format,
  // e.g. will convert 12 bit to 16 bit
  img = img.ensureTypedDepth();

  return EXIT_SUCCESS;
}

/*****************************************************************/
int imagewrite(bim::Image img, bim::TagMap metadata, char *filename, const char *format){

  int err;
  int page = 0;
  const char* options = "";
  bim::MetaFormatManager fm;

  err = fm.sessionStartWrite(filename, format, options); CHKRTN(err, FILEERR);

  fm.sessionWriteSetMetadata(metadata);

  /********************************/
  char mdata[BUFFERSIZE];
  char cmt[BUFFERSIZE];
  int cmtsz = 0;
  //int cmtsz = -1;
  std::map<std::string, std::string>::const_iterator vMapIt;

  mopen(cmt, &cmtsz, BUFFERSIZE);
  for (vMapIt = metadata.begin(); vMapIt != metadata.end(); ++vMapIt) {
      sprintf(mdata, "%s=%s", vMapIt->first.c_str(), vMapIt->second.c_str());
      mappend(cmt, &cmtsz, BUFFERSIZE, mdata);
      //char type[] = "1";
      //appendmetadata(cmt, &cmtsz, BUFFERSIZE, type, mdata);
  }
  mclose(cmt, &cmtsz, BUFFERSIZE);

  //memcpy(commentline, cmt, cmtsz);
  strncpy(commentline, cmt, cmtsz);
  commentlinelen = cmtsz;
  /********************************/

  err = fm.sessionWriteImage(img.imageBitmap(), page);
  CHKRTNMSG(err,"Error reading image");

  fm.sessionEnd();

  return EXIT_SUCCESS;
}
