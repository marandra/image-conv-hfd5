#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

//#include "meta_format_manager.h"
#include "hdf5.h"

//#include "hdf5readwrite.hh"
//#include "imagereadwrite.hh"
#include "bimconvfunct.hh"
#include "chkerr.hh"

//Temporary workaround for sharing par  //marcelo
/*Delete when solved*/
//struct param glopar;

/******************************************************************************/
int main(int argc, char **argv) {

  int err;
  int nargs;
  int i;
  param par;

  /* parse input arguments */
  err = parse_cmdline(&nargs, argc, argv, &par);
  #ifdef DEBUG
  fprintf(stdout, "\n[DEBUG] Input (%d arguments, %d total):\n", nargs, argc);
  i = argc - nargs;
  do {
      if (nargs){
          CHKERR(parseiarg(argv[i], &par));
      }
      fprintf(stdout,"    formt, page: %s, %d\n",
                          par.formt, par.pagen);
      fprintf(stdout,"    pathi/conti/grupi/filei: %s/%s/%s/%s\n",
                          par.pathi, par.conti, par.grupi, par.filei);
      fprintf(stdout,"    patho/conto/grupo/fileo: %s/%s/%s/%s\n",
                          par.patho, par.conto, par.grupo, par.fileo);
      fprintf(stdout,"\n");
      i++;
  } while (i < argc); 
  #endif /* DEBUG */
  if (err) {
      printusage(argv[0]);
      return EXIT_FAILURE;
  }

  /* loop over input files */
  i = argc - nargs;
  do {
      if (nargs){
          err = parseiarg(argv[i], &par);
      }
    
      /* case single image file */
      if(par.filei[0]){
          err = convertimage(par);
      }
    
      /* case batch convert of a complete group container */
      else {
          err = convertimagebatch(par);
      }

      i++;

  } while (i < argc); 

  if (err) {
      #ifdef NDEBUG
      fprintf(stdout, "\n[DIAG] Input (%d arguments, %d total):\n", nargs, argc);
      for (i = argc - nargs; i < argc; i++) {
          err = parseiarg(argv[i], &par); CHKERR(err);
          fprintf(stdout,"    formt, page: %s, %d\n",
                              par.formt, par.pagen);
          fprintf(stdout,"    pathi/conti/grupi/filei: %s/%s/%s/%s\n",
                              par.pathi, par.conti, par.grupi, par.filei);
          fprintf(stdout,"    patho/conto/grupo/fileo: %s/%s/%s/%s\n",
                              par.patho, par.conto, par.grupo, par.fileo);
          fprintf(stdout,"\n");
      }
      #endif /* NDEBUG */

      return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
