/*******************************************************************************
  Matlab interface for bioImage formats library

  Input:
   fname    - string with file name of image to decode
   page_num - the number of the page to decode, 0 if ommited
              in time series pages are time points
              in the z series pages are depth points

  Output:
   im       - matrix of the image in the native format with channels in the 3d dimension
   format   - string with format name used to decode the image
   pages    - number of pages in the image
   xyzres   - pixel size on x,y,z dim in microns double[3]
   metatxt  - string with all meta-data extracted from the image

  ex:
   [im, format, pages, xyzr, metatxt] = bimreadh5(fname, page_num);

   [im] = bimreadh5(fname);

  Notes:
    Currently only creates 8 or 16 bit arrays!!!
    Extend if you want to read other type of data.


  Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    10/13/2006 16:00 - First creation

  Ver : 4
*******************************************************************************/
#include <libH5AR.hh>

// our interface to imgcnv includes:
#include <bimreadh5.hh>

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <vector>





//------------------------------------------------------------------------------
// function: bimreadh5
// INPUTS:
// OUTPUTS:
//------------------------------------------------------------------------------

int bimreadh5(bim::Image& aImage, std::string& aFormatName, int& aNumPages, double aPixelSize[], bim::TagMap& aMetaDataMap, const std::string aFileName, const int aPage)
{

  //-----------------------------------------------------------
  // handling of HDF5 containers:
  //-----------------------------------------------------------

  std::string vExtension = H5AR::GetExtension(aFileName);
  if (!vExtension.empty()) {
    vExtension = "." + vExtension;
  }

  // check if the file is inside a container
  std::string vTempFileName = aFileName;
  const bool vIsContainer = H5AR::IsInsideContainer(aFileName);

  // special treatment for containers: extract and later delete
  if (vIsContainer) {
    vTempFileName = H5AR::TempFileName(vExtension);

    if (!H5AR::ExtractFromH5(aFileName, vTempFileName)) {
      std::cerr << "Failed to extract image " << aFileName << " from container." << std::endl;
    }
  }
  //else {
  //  std::vector<char> vDataSet;
  //  if (!H5AR::ReadFile(aFileName, vDataSet)) {
  //    std::cerr << "Failed to read image from original filename." << std::endl;
  //  }
  //  if (!H5AR::WriteFile(vTempFileName, vDataSet)) {
  //    remove(vTempFileName.c_str());
  //    std::cerr << "Failed to write image to temporary filename." << std::endl;
  //  }
  //}




  //-----------------------------------------------------------
  // read image and metadata
  //-----------------------------------------------------------
  bim::MetaFormatManager vMetaFormatManager;

  if (vMetaFormatManager.sessionStartRead((bim::Filename)vTempFileName.c_str()) != 0) {
    std::cerr << "Input format is not supported." << std::endl;
    // delete the temporary file after usage:
    if (vIsContainer) {
      remove(vTempFileName.c_str());
    }

    return 1;
  }

  aNumPages = vMetaFormatManager.sessionGetNumberOfPages();
  aFormatName = vMetaFormatManager.sessionGetFormatName();

#if defined (DEBUG) || defined (_DEBUG)
  std::cerr << "Number of pages: " << aNumPages << std::endl;
  std::cerr << "Format name: " << aFormatName  << std::endl;
#endif

  int vPage = aPage;
  if (vPage<0) {
    vPage=0;
    std::cerr << "Requested page number is invalid, used " << vPage << std::endl;
  }

  if (vPage>=aNumPages) {
    vPage=aNumPages-1;
    std::cerr << "Requested page number is invalid, used " << vPage << std::endl;
  }

  if (vMetaFormatManager.sessionReadImage(aImage.imageBitmap(), vPage) != 0) {
    if (vIsContainer) {
      remove(vTempFileName.c_str());
    }

    return 1;
  }

  // getting metadata fields
  vMetaFormatManager.sessionParseMetaData(vPage);
  aPixelSize[0] = vMetaFormatManager.getPixelSizeX();
  aPixelSize[1] = vMetaFormatManager.getPixelSizeY();
  aPixelSize[2] = vMetaFormatManager.getPixelSizeZ();

#if defined (DEBUG) || defined (_DEBUG)
  std::cerr << "Pixel resolution: (" << aPixelSize[0] << ", " << aPixelSize[1] << ", " << aPixelSize[2] << ")" << std::endl;
#endif

  // get meta text if required
  aMetaDataMap = vMetaFormatManager.get_metadata();

  // add the filename to aMetaDataMap:
  const size_t vPos = aFileName.rfind('/');
  std::string fbasename;
  if (vPos != std::string::npos) fbasename = aFileName.substr(vPos+1);
  else                           fbasename = aFileName;
  aMetaDataMap["Filename"] = fbasename.c_str();


  vMetaFormatManager.sessionEnd();


  //-----------------------------------------------------------
  // pre-poc input image
  //-----------------------------------------------------------

  // make sure red image is in supported pixel format, e.g. will convert 12 bit to 16 bit
  aImage = aImage.ensureTypedDepth();


  // delete the temporary file after usage:
  if (vIsContainer)
    remove(vTempFileName.c_str());


  return 0;
}
