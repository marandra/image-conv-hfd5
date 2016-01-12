#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "boost/python/module.hpp"
#include "boost/python/def.hpp"

#include "meta_format_manager.h"
#include "hdf5.h"

#include "hdf5readwrite.hh"
#include "imagereadwrite.hh"
#include "bimconvfunct.hh"
#include "chkerr.hh"

//Temporary workaround for sharing par  //marcelo
/*Delete when solved*/
struct param glopar;

/******************************************************************************/
void printusage(char * name){
  fprintf(stdout,"\n");
  fprintf(stdout,"Usage:\n");
  fprintf(stdout,"  %s [-p page] [-f format] -i /path/to/image [-d /dest/path/]\n", name);
  fprintf(stdout,"  %s [-p page] [-f format] -i /path/to/image [-d /dest/path/] [-O H5/grup]\n", name);
  fprintf(stdout,"  %s [-p page] [-f format] -I /path/to/H5in/groupin/image [-d /dest/path/]\n", name);
  fprintf(stdout,"  %s [-p page] [-f format] -I /path/to/H5in/groupin/image [-d /dest/path/] [-O H5/group]\n", name);
  fprintf(stdout,"  %s [-p page] [-f format] -b H5in [-d /dest/path/]\n", name);
  fprintf(stdout,"\n");
  fprintf(stdout,"Arguments:\n");
  fprintf(stdout,"  -i input path + file (output image has original name with extension replaced)\n");
  fprintf(stdout,"  -I input path + H5 + group + file\n");
  fprintf(stdout,"  -d output directory (output image has original name with extension replaced)\n");
  fprintf(stdout,"  -O H5 + group\n");
  fprintf(stdout,"  -b input path + H5 (output is H5-convrt)\n");
  fprintf(stdout,"  -p page (default: 0)\n");
  fprintf(stdout,"  -f format (default: jp2)\n");
  fprintf(stdout,"\n");
  fprintf(stdout,"Description:\n");
  fprintf(stdout,"Converts an image to format. ");
  fprintf(stdout,"Image can be inside H5containter/group ");
  fprintf(stdout,"(it is assumed that images are inside a group different from root). ");
  fprintf(stdout,"With the -b flag, the program creates (or overwrites) a container ");
  fprintf(stdout,"with the group structure from the input H5 container\n");

  return; 
}

/******************************************************************************/
int parseiarg(char *argi, struct param *par) {

    /* split path and file */
    std::string arg = argi;
    unsigned found = arg.find_last_of("/");
    std::string pathi = (std::string) par->pathi;
    if (found != (unsigned) -1) pathi = arg.substr(0, found);
    std::string filei = arg.substr(found + 1);
 
    /* generate output name */
    found = filei.find_last_of(".");
    std::string fileo = filei.substr(0, found) + "." + (std::string) par->formt; 
 
    strncpy(par->pathi, pathi.c_str(), sizeof(par->pathi) - 1);
    strncpy(par->filei, filei.c_str(), sizeof(par->filei) - 1);
    strncpy(par->fileo, fileo.c_str(), sizeof(par->fileo) - 1);
 
    return EXIT_SUCCESS;
}

/******************************************************************************/
int parse_cmdline(int *pnargs, int argc, char **argv, param *par) {

  int opt;
  int flagb = 0, flagi = 0, flagI = 0, flagO = 0;
  std::string argb, argi, argI, argO;

  strncpy(par->formt, "jp2", sizeof(par->formt) - 1);
  par->pagen    = 0;
  par->filei[0] = 0;
  par->fileo[0] = 0;
  par->conti[0] = 0;
  par->conto[0] = 0;
  par->grupi[0] = 0;
  par->grupo[0] = 0;
  strncpy(par->pathi, ".", sizeof(par->pathi) - 1);
  strncpy(par->patho, ".", sizeof(par->patho) - 1);

  while ((opt = getopt(argc, argv, "f:p:iI:d:O:b:")) != -1) {
      switch (opt) {
          case 'p':
              par->pagen = atoi(optarg);
              break;

          case 'f':
              strncpy(par->formt, optarg, sizeof(par->formt) - 1);
              break;

          case 'b':
              argb = std::string(optarg);
              flagb = 1;
              break;

          case 'i':
              flagi = 1;
              break;

          case 'I':
              argI = std::string(optarg);
              flagI = 1;
              break;

          case 'd':
              strncpy(par->patho, optarg, sizeof(par->patho) - 1);
              break;

          case 'O':
              argO = std::string(optarg);
              flagO = 1;
              break;

          default:
             return EXIT_FAILURE;
      }
  }

  /* check consistency of input */
  if (!flagb) {
      if (flagi == flagI) return EXIT_FAILURE;
      }
  else if (flagO || flagi || flagI) return EXIT_FAILURE;

  /* ignore extra arguments if not 'i' option */
  *pnargs = 0;
  if (flagi) {
      *pnargs = argc - optind;
  }

  /* case b, bd */
  if (flagb) {
      std::string arg = argb;
      std::string pathi = (std::string) par->pathi;
      unsigned found = arg.find_last_of("/");
      if (found != (unsigned) -1) pathi = arg.substr(0, found);
      std::string conti = arg.substr(found + 1);
      std::string conto = conti + "_cnvtd";

      strncpy(par->pathi, pathi.c_str(), sizeof(par->pathi) - 1);
      strncpy(par->conti, conti.c_str(), sizeof(par->conti) - 1);
      strncpy(par->conto, conto.c_str(), sizeof(par->conto) - 1);

      return EXIT_SUCCESS;
  }

  /* case O*/
  if (flagO) {
      std::string arg = argO;
      unsigned found = arg.find_last_of("/");
      std::string conto = arg.substr(0, found);
      std::string grupo = arg.substr(found + 1);
      strncpy(par->conto, conto.c_str(), sizeof(par->conto) - 1);
      strncpy(par->grupo, grupo.c_str(), sizeof(par->grupo) - 1);
  } 

  /* case i, id*/
  if (flagi) {
      return EXIT_SUCCESS;
  }

  /* case I, Id*/
  if (flagI) {
      /* split path and file */
      unsigned found;
      std::string arg = argI;
      /* get file */
      found = arg.find_last_of("/");
      std::string filei = arg.substr(found + 1);
      arg = arg.substr(0, found);
      /* get group */
      found = arg.find_last_of("/");
      std::string grupi = arg.substr(found + 1);
      arg = arg.substr(0, found);
      /* get container */
      found = arg.find_last_of("/");
      std::string conti = arg.substr(found + 1);
      /* get path */
      std::string pathi = (std::string) par->pathi;
      if (found != (unsigned) -1) pathi = arg.substr(0, found);
      /* generate output name */
      found = filei.find_last_of(".");
      std::string fileo = filei.substr(0, found) + "." + (std::string) par->formt; 

      strncpy(par->filei, filei.c_str(), sizeof(par->filei) - 1);
      strncpy(par->fileo, fileo.c_str(), sizeof(par->fileo) - 1);
      strncpy(par->grupi, grupi.c_str(), sizeof(par->grupi) - 1);
      strncpy(par->conti, conti.c_str(), sizeof(par->conti) - 1);
      strncpy(par->pathi, pathi.c_str(), sizeof(par->pathi) - 1);

      return EXIT_SUCCESS;
  }

  return EXIT_FAILURE;
}

/******************************************************************************/
int convertimageh5(struct param par) {

  int         err;
  bim::Image   img;
  bim::TagMap     metadata;
  struct stat stat_buf;
  bool        flagtmpfilei = false;
  bool        flagtmpfileo = false;
  char        tmpfilei[40];
  char        tmpfileo[40];


  if (par.conti[0]){
      /* read file in a h5 container */
      srand(time(0));
      sprintf(tmpfilei, "/dev/shm/bimconv-%d%d%d",rand(),rand(),rand());
      // (A better way for temporary filenames should be using mkstemp(), the catch
      // is that TMetaFormatManager needs to receive a fd instead of a filename.
      // It is possible to modify the FM (the struct uses a fd) but is not worth it)
      err = extractfromh5(par, (std::string) tmpfilei ); CHKERR(err);
      err = imageread(img, &metadata, tmpfilei, par.pagen); CHKERR(err);
      flagtmpfilei = true;
  }
  else {
      /* read image not from container */
      char buff[4096]="";
      std::string name = (std::string) par.pathi + "/" + (std::string) par.filei; 
      strncpy(buff, name.c_str(), strlen(name.c_str()));
      err = imageread(img, &metadata, buff, par.pagen); CHKERR(err);
  }


  if (par.conto[0]) {
      /* write image into h5 container, we need a tmpfile */
      sprintf(tmpfileo, "/dev/shm/bimconv-%d%d%d",rand(),rand(),rand());
      err = imagewrite(img, metadata, tmpfileo, par.formt); CHKERR(err);
      flagtmpfileo = true;
      /* copy outfile to new container */
      err = writetoh5( (std::string) tmpfileo, par); CHKERR(err);
  }
  else{
      /* write image not to container */
      char buff[4096]="";
      std::string name = (std::string) par.patho + "/" + (std::string) par.fileo; 
      strncpy(buff, name.c_str(), strlen(name.c_str()));
      err = imagewrite(img, metadata, buff, par.formt); CHKERR(err);
  }


  /* removes temporary file if exists */
  if (flagtmpfilei && !stat(tmpfilei, &stat_buf))
      remove(tmpfilei);
  if (flagtmpfileo && !stat(tmpfileo, &stat_buf))
      remove(tmpfileo);

  return EXIT_SUCCESS;
}

/******************************************************************************/
int convertimage(struct param par) {

  int err; 

  /* if image goes into h5 container, create container and group if needed
   * before starting conversion */

  if (par.conto[0]) {

      struct stat stat_buf;
      hid_t h5fid = -1; 

      std::string patho = std::string(par.patho);
      std::string conto = std::string(par.conto);
      std::string cpath = patho + "/" + conto;

      /* create containter if it doesn't exist */
      if (stat(cpath.c_str(), &stat_buf)){
          /* it doesn't exist, create empty container. TRUNC overwrites it if exists. */
          h5fid = H5Fcreate(cpath.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
          CHKRTN(h5fid < 0, FILEERR);
      }
      else {
          /* it exists, open an existing file. */
          h5fid = H5Fopen(cpath.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
          CHKRTN(h5fid < 0, FILEERR);
      } 

      err = H5Lexists(h5fid, par.grupo, H5P_DEFAULT ); CHKRTN(err < 0, ERR);
      if (!err){
          /* create empty group */
          hid_t gid;
          gid = H5Gcreate(h5fid, par.grupo, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
          CHKRTN(gid < 0, ERR);
      }
       
      H5Fclose(h5fid);

  }

  err = convertimageh5(par); CHKERR(err);

  return EXIT_SUCCESS;  
}

/******************************************************************************/
herr_t op_func_dset (hid_t loc_id, const char* objname, const H5L_info_t* /*info*/, void* /*par*/) {

  //struct param glopar;
  struct param* ppar;
  ppar = &glopar;

  int err;
  H5G_stat_t statbuf;
  std::string objname_s (objname);
  
  err = H5Gget_objinfo (loc_id, objname, 0, &statbuf); CHKERR(err);

  /* check element is dataset */
  if (statbuf.type == H5G_DATASET) {
 
     /* check dataset is image */
     unsigned found  = objname_s.find_last_of(".");
     std::string base = objname_s.substr(0, found);
     std::string exten = objname_s.substr(found + 1);

     if (exten == "tif"  || exten == "tiff" || exten == "jp2"  || exten == "png"  ) {
         /* fill par structure */
         std::string name = base + "." + std::string(ppar->formt);
         strncpy(((struct param *)ppar)->filei, objname_s.c_str(), sizeof(((struct param *)ppar)->filei) - 1);
         strncpy(((struct param *)ppar)->fileo, name.c_str(), sizeof(((struct param *)ppar)->fileo) - 1);

         err = convertimageh5(*((struct param *)ppar)); CHKERR(err);
     }

     /* object is anything alse */
     else {

         /* TODO copy object with no modifications to out container */

     }
  }

  return EXIT_SUCCESS;
}

/******************************************************************************/
herr_t op_func_group (hid_t loc_id, const char* objname, const H5L_info_t* /*info*/, void* /*ppar*/) {

  //struct param glopar;
  struct param* ppar;
  ppar = &glopar;

  int err;
  hid_t h5fi;
  hid_t h5fo;
  hid_t gid;
  H5G_stat_t statbuf;

  err = H5Gget_objinfo (loc_id, objname, 0, &statbuf); CHKERR(err);

  /* if object is group, open containers, create group, iterate over datasets, close */
  if (statbuf.type == H5G_GROUP) {
      h5fi = H5Fopen(((struct param *)ppar)->conti, H5F_ACC_RDONLY, H5P_DEFAULT); CHKRTN(h5fi < 0, FILEERR);
      h5fo = H5Fopen(((struct param *)ppar)->conto, H5F_ACC_RDWR, H5P_DEFAULT); CHKRTN(h5fo < 0, FILEERR);
      gid = H5Gcreate(h5fo, objname, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT); CHKRTN(gid < 0, ERR);
      strncpy(((struct param *)ppar)->grupi, objname, sizeof(((struct param *)ppar)->grupi) - 1);
      strncpy(((struct param *)ppar)->grupo, objname, sizeof(((struct param *)ppar)->grupo) - 1);
      err = H5Literate_by_name (h5fi, objname, H5_INDEX_NAME, H5_ITER_NATIVE, NULL, op_func_dset, (void *) ppar, NULL); CHKERR(err);
      err = H5Fclose(h5fo); CHKERR(err);
      err = H5Fclose(h5fi); CHKERR(err);
  }

  return EXIT_SUCCESS;
}

/******************************************************************************/
int convertimagebatch(param par) {

      //struct param glopar;
      int err;
      hid_t h5fi = -1;
      hid_t h5fo = -1;

      /* open source container */
      h5fi = H5Fopen(par.conti, H5F_ACC_RDONLY, H5P_DEFAULT); CHKRTN(h5fi < 0, FILEERR);

      /* create empty container. TRUNC overwrite it if exists. */
      h5fo = H5Fcreate(par.conto, H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);

      /* iterate over groups */
      glopar = par;
      err = H5Literate_by_name (h5fi, ".", H5_INDEX_NAME, H5_ITER_NATIVE, NULL, op_func_group, (void *) &par, NULL); CHKERR(err);

      /* close containers */
      err = H5Fclose(h5fi); CHKERR(err);

   return EXIT_SUCCESS;
}

BOOST_PYTHON_MODULE(libbimcnvh5)
{
    using namespace boost::python;
    def("printusage", printusage);
}

