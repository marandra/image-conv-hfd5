#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include "hdf5.h"

#include "hdf5readwrite.hh"
#include "bimconvfunct.hh"
#include "chkerr.hh"


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}


int writefile(std::string vTmpFile, const std::vector<char>& vDataSet)
{
  FILE* tmpfile = NULL;
  tmpfile = fopen(vTmpFile.c_str(),"w");

  // write the data to a temporary file
  size_t vWrittenBytes = 0;
  for (size_t vTry = 0; vTry < 128; ++vTry) {
      vWrittenBytes += fwrite(&vDataSet[vWrittenBytes], 1, (vDataSet.size()-vWrittenBytes), tmpfile);

      // error handling
      if (ferror(tmpfile) != 0) {
         fprintf(stderr, "Detected Error during writing to file %s", vTmpFile.c_str());
          fclose(tmpfile);
          return false;
    }

      if (vWrittenBytes == vDataSet.size()) break;
      #ifdef DEBUG
      fprintf(stderr,"Could not write the full file, trying again.\n");
      #endif
  }
  
  fclose(tmpfile);

  if (vWrittenBytes != vDataSet.size()) {
    fprintf(stderr,"Failed to write the full data");
    return false;
  }

  return true;
}


int extractfromh5(struct param par, std::string vTmpFile)
{

  hid_t vH5File = -1;
  hid_t vH5Dataset = -1;
  hid_t vH5Datatype = -1;
  hid_t vH5Dataspace = -1;
  hid_t vH5OPAQUETypeId = -1;

  std::string cpath, cname;
  std::string pathi = (std::string) par.pathi;
  std::string conti = (std::string) par.conti;
  std::string grupi = (std::string) par.grupi;
  std::string filei = (std::string) par.filei;

  cpath = pathi + "/" + conti;
  cname = grupi + "/" + filei;

  /* Open an existing file. */
  vH5File = H5Fopen(cpath.c_str(), H5F_ACC_RDONLY, H5P_DEFAULT);
  CHKRTN(vH5File < 0, FILEERR);

  /* Open an existing dataset. */
  vH5Dataset = H5Dopen(vH5File, cname.c_str(), H5P_DEFAULT);
  vH5Datatype = H5Dget_type(vH5Dataset);
  vH5Dataspace = H5Dget_space(vH5Dataset);

    // TODO: The rank is hardcoded to 1 (which is sufficient
    //       for openBIS), but it would be more generic to code
    //       for a dynamic rank!
    const int rank = H5Sget_simple_extent_ndims(vH5Dataspace);
    if (rank != 1) {
      if (vH5OPAQUETypeId >= 0) H5Tclose(vH5OPAQUETypeId);
      if (vH5Datatype     >= 0) H5Tclose(vH5Datatype);
      if (vH5Dataspace    >= 0) H5Sclose(vH5Dataspace);
      if (vH5Dataset      >= 0) H5Dclose(vH5Dataset);
      if (vH5File         >= 0) H5Fclose(vH5File);
      CHKRTNMSG((rank != 1), "Data space rank is not '1' as expected");
    }

    hsize_t vH5DataspaceDims[1];
    H5Sget_simple_extent_dims(vH5Dataspace, vH5DataspaceDims, NULL);

    // Define the memory dataspace.
    hid_t memspace = H5Screate_simple(1, vH5DataspaceDims, NULL);


    // allocate memory for the dataset:
    std::vector<char> vDataSet(vH5DataspaceDims[0]);
    if (vDataSet.size() != vH5DataspaceDims[0]) {
   fprintf(stderr, "Could not reserve memory");
      if (vH5OPAQUETypeId >= 0) H5Tclose(vH5OPAQUETypeId);
      if (vH5Datatype     >= 0) H5Tclose(vH5Datatype);
      if (vH5Dataspace    >= 0) H5Sclose(vH5Dataspace);
      if (vH5Dataset      >= 0) H5Dclose(vH5Dataset);
      if (vH5File         >= 0) H5Fclose(vH5File);
      return -1;
    }

    //hid_t vH5OPAQUETypeId = H5Tcreate(H5T_OPAQUE, 1);
    hid_t vDataTypeId = H5Dget_type(vH5Dataset);
    vH5OPAQUETypeId = H5Tget_native_type(vDataTypeId, H5T_DIR_ASCEND);

    // read the data from the dataset/dataspace:
    herr_t readstatus = H5Dread(vH5Dataset, vH5OPAQUETypeId, memspace, vH5Dataspace, H5P_DEFAULT, &vDataSet[0]);
    if (readstatus < 0) {
        fprintf(stderr, "Failed to read dataset from file");
        if (vH5OPAQUETypeId >= 0) H5Tclose(vH5OPAQUETypeId);
        if (vH5Datatype     >= 0) H5Tclose(vH5Datatype);
        if (vH5Dataspace    >= 0) H5Sclose(vH5Dataspace);
        if (vH5Dataset      >= 0) H5Dclose(vH5Dataset);
        if (vH5File         >= 0) H5Fclose(vH5File);
      return -1;
    }


    if (!writefile(vTmpFile, vDataSet)) {
   fprintf(stderr, "Failed to write dataset to new file");
      if (vH5OPAQUETypeId >= 0) H5Tclose(vH5OPAQUETypeId);
      if (vH5Datatype     >= 0) H5Tclose(vH5Datatype);
      if (vH5Dataspace    >= 0) H5Sclose(vH5Dataspace);
      if (vH5Dataset      >= 0) H5Dclose(vH5Dataset);
      if (vH5File         >= 0) H5Fclose(vH5File);
      return -1;
    }



  return 0;
}


int writetoh5(std::string tmpfileo, struct param par) {

  hid_t h5file_id;
  hid_t dtype_id;
  hid_t dspace_id;
  hid_t dset_id;
  char *buffer;
  int err;

  std::string cpath, cname;
  std::string patho = (std::string) par.patho;
  std::string conto = (std::string) par.conto;
  std::string grupo = (std::string) par.grupo;
  std::string fileo = (std::string) par.fileo;

  cpath = patho + "/" + conto;
  cname = grupo + "/" + fileo;

  /* open image */
  FILE *fi = fopen(tmpfileo.c_str(),"r"); CHKRTN(!fi, FILEERR);

  /* get file length */
  fseek(fi, 0, SEEK_END);
  hsize_t filen = ftell(fi);
  fseek(fi, 0, SEEK_SET);

  /* allocate memory */
  buffer = (char *)malloc(filen+1); CHKRTN(!buffer, MEMERR);

  /* read image */
  fread(buffer, filen, 1, fi);
  
  /* close file */
  fclose(fi);

  /* open container. it has to be created somewhere else. */
  h5file_id = H5Fopen(cpath.c_str(), H5F_ACC_RDWR, H5P_DEFAULT);
  CHKRTN(h5file_id < 0, FILEERR);

  /* Create the datatype */
  dtype_id = H5Tcreate(H5T_OPAQUE, 1);

  /* Set the datatype tag */
  err = H5Tset_tag(dtype_id, "Opaque Test Tag"); CHKERR(err);

  /* Create the dataspace */
  dspace_id = H5Screate_simple(1, &filen, NULL);

  /* Create the dataset */
  dset_id = H5Dcreate(h5file_id, cname.c_str(), dtype_id, dspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  /* Write the data out to the dataset */
  err = H5Dwrite(dset_id, dtype_id, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer); CHKERR(err);

  /* Close resource handles */
  err = H5Dclose(dset_id); CHKERR(err);
  err = H5Sclose(dspace_id); CHKERR(err);
  err = H5Tclose(dtype_id); CHKERR(err);
  err = H5Fclose(h5file_id); CHKERR(err);
  free(buffer);

  return EXIT_SUCCESS;
}

