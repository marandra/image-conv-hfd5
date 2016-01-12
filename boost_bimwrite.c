/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 *         Title:                                                          *
 *   Description:                                                          *
 *                                                                         *
 *                                                                         *
 *  Input:                                                                 *
 *   fname    - string with file name of image to decode                   *
 *   page_num - the number of the page to decode, 0 if ommited             *
 *              in time series pages are time points                       *
 *              in the z series pages are depth points                     *
 *                                                                         *
 *  Output:                                                                *
 *   im       - matrix of the image in the native format with channels     *
 *              in the 3d dimension                                        *
 *   format   - string with format name used to decode the image           *
 *   pages    - number of pages in the image                               *
 *   xyzres   - pixel size on x,y,z dim in microns double[3]               *
 *   metatxt  - string with all meta-data extracted from the image         *
 *                                                                         *
 *  ex:                                                                    *
 *   [] = bimwriteh5( );    *
 *                                                                         *
 *   [] = bimwriteh5( );                                            *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/
// boost.Python includes
#include "boost/python/module.hpp"
#include "boost/python/def.hpp"
//#include "boost/python/object.hpp"
//#include "boost/python/tuple.hpp"

// our interface to imgcnv includes:
#include <bimwriteh5.hh>

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <string.h>
#include <stdio.h>



/********************************************************************
 *
 *
 ********************************************************************/
boost::python::object boost_bimwriteh5(const char* aFileName, const char* frmt)
{
  /********************************************************************
   * Input Parameter Checking
   ********************************************************************/
  const mwSize* vImageDimensionArray = mxGetDimensions(prhs[0]);
  const mwSize vImageNumDimensions = mxGetNumberOfDimensions(prhs[0]);


  if ((vImageNumDimensions < 2) || (vImageNumDimensions > 3))
    mexErrMsgTxt("The number of image dimensions is not supported.");

  int vChannels;
  if (vImageNumDimensions > 2) {
    vChannels = (int)vImageDimensionArray[2];
  } else {
    vChannels = 1;
  }


  int vStatus = -1;
  if (mxGetClassID(prhs[0]) == mxDOUBLE_CLASS){
    /* If the class is double, we assume its in the [0,1] range and multiply
     * it with 255 to convert it afterwards into an 8bit image */
    double* vImage = (double*)mxGetData(prhs[0]);
    std::vector<unsigned char> vImageRescaled(vWidth*vHeight);
    for (int vImageIdx = 0; vImageIdx < vWidth*vHeight; ++vImageIdx)
      vImageRescaled[vImageIdx] = (unsigned char)(255.0 * vImage[vImageIdx] + 0.5);
    vStatus = bimwriteh5<unsigned char>(&vImageRescaled[0], vFileName, vFileFormat, vWidth, vHeight, vChannels, 8);
  }
  else if (mxGetClassID(prhs[0]) == mxSINGLE_CLASS){
    /* If the class is single, we assume its in the [0,1] range and multiply
     * it with 255 to convert it afterwards into an 8bit image */
    float* vImage = (float*)mxGetData(prhs[0]);
    std::vector<unsigned char> vImageRescaled(vWidth*vHeight);
    for (int vImageIdx = 0; vImageIdx < vWidth*vHeight; ++vImageIdx)
      vImageRescaled[vImageIdx] = (unsigned char)(255.0 * vImage[vImageIdx] + 0.5);
    vStatus = bimwriteh5<unsigned char>(&vImageRescaled[0], vFileName, vFileFormat, vWidth, vHeight, vChannels, 8);
  }
  else if (mxGetClassID(prhs[0]) == mxINT32_CLASS){
    int* vImage = (int*)mxGetData(prhs[0]);
    vStatus = bimwriteh5<int>(vImage, vFileName, vFileFormat, vWidth, vHeight, vChannels, 32);
  }
  else if (mxGetClassID(prhs[0]) == mxUINT32_CLASS){
    unsigned int* vImage = (unsigned int*)mxGetData(prhs[0]);
    vStatus = bimwriteh5<unsigned int>(vImage, vFileName, vFileFormat, vWidth, vHeight, vChannels, 32);
  }
  else if (mxGetClassID(prhs[0]) == mxINT16_CLASS){
    short* vImage = (short*)mxGetData(prhs[0]);
    vStatus = bimwriteh5<short>(vImage, vFileName, vFileFormat, vWidth, vHeight, vChannels, 16);
  }
  else if (mxGetClassID(prhs[0]) == mxUINT16_CLASS){
    unsigned short* vImage = (unsigned short*)mxGetData(prhs[0]);
    vStatus = bimwriteh5<unsigned short>(vImage, vFileName, vFileFormat, vWidth, vHeight, vChannels, 16);
  }
  else if (mxGetClassID(prhs[0]) == mxINT8_CLASS){
    char* vImage = (char*)mxGetData(prhs[0]);
    vStatus = bimwriteh5<char>(vImage, vFileName, vFileFormat, vWidth, vHeight, vChannels, 8);
  }
  else if (mxGetClassID(prhs[0]) == mxUINT8_CLASS){
    unsigned char* vImage = (unsigned char*)mxGetData(prhs[0]);
    vStatus = bimwriteh5<unsigned char>(vImage, vFileName, vFileFormat, vWidth, vHeight, vChannels, 8);
  } else {
    // TODO FIXME:
    // error, bit depth not understood
  }

  // TODO FIXME:
  // error handling:

}

BOOST_PYTHON_MODULE(libbimreadh5)
{
    def("bimwriteh5", boost_bimwriteh5);
}
