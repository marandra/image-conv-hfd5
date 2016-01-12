// boost.Python includes
#include "boost/python/module.hpp"
#include "boost/python/def.hpp"
#include "boost/python/object.hpp"
#include "boost/python/tuple.hpp"
#include "boost/python/list.hpp"

// our interface to imgcnv includes:
#include <bimreadh5.hh>

// imgcnv includes:
#include <xstring.h>

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <string.h>
#include <stdio.h>

boost::python::list bmp_wrapper(const bim::uint8 * pbmp, size_t nelem) {
    boost::python::list result;
    for (int i = 0; i < nelem; i++){
         bim::uint8 bmp = pbmp[i];
         result.append(bmp);    
    }
    return result;
}

/********************************************************************
 * filename        
 * page            
 ********************************************************************/
boost::python::object boost_bimreadh5(const char* vFileName, int vPageNumber=0)
{
  /********************************************************************
   * Input Parameter Checking
   ********************************************************************/

  //-----------------------------------------------------------
  // if page is provided
  //-----------------------------------------------------------

#ifdef _DEBUG
  fprintf(stdout, "Filename to load: %s\n", vFileName);
  fprintf(stdout, "Page to load: %d\n", vPageNumber);
#endif


  // return values:
  bim::Image  vImage;
  std::string vFormatName = "";
  int         vNumPages=0;
  double      vPixelSize[3] = {0.0, 0.0, 0.0};
  bim::TagMap vMetaDataMap;


  int vStatus = bimreadh5(vImage, vFormatName, vNumPages, vPixelSize, vMetaDataMap, vFileName, vPageNumber);


  if (vStatus == 0) {
    //-----------------------------------------------------------
    // create output image
    //-----------------------------------------------------------


    size_t size  = vImage.samples() * vImage.width() * vImage.height();
    bim::uint8 *plhs = new bim::uint8[ size * vImage.depth()/8 ];

    // UNSIGNED
    if      (vImage.depth()==8  && vImage.pixelType()==bim::FMT_UNSIGNED) { copy_data<bim::uint8> (vImage, (bim::uint8*) plhs); }
    else if (vImage.depth()==16 && vImage.pixelType()==bim::FMT_UNSIGNED) { copy_data<bim::uint16>(vImage, (bim::uint16*)plhs); }
    else if (vImage.depth()==32 && vImage.pixelType()==bim::FMT_UNSIGNED) { copy_data<bim::uint32>(vImage, (bim::uint32*)plhs); }
    else // SIGNED
    if      (vImage.depth()==8  && vImage.pixelType()==bim::FMT_SIGNED)   { copy_data<bim::int8>  (vImage, (bim::int8*)  plhs); }
    else if (vImage.depth()==16 && vImage.pixelType()==bim::FMT_SIGNED)   { copy_data<bim::int16> (vImage, (bim::int16*) plhs); }
    else if (vImage.depth()==32 && vImage.pixelType()==bim::FMT_SIGNED)   { copy_data<bim::int32> (vImage, (bim::int32*) plhs); }
    else // FLOAT
    if      (vImage.depth()==32 && vImage.pixelType()==bim::FMT_FLOAT)    { copy_data<float>      (vImage, (float*)      plhs); }
    else if (vImage.depth()==64 && vImage.pixelType()==bim::FMT_FLOAT)    { copy_data<double>     (vImage, (double*)     plhs); }
    else {
      // TODO FIXME:
      // We are missing error handling here, what happens if the type was not
      // found??? FIX THIS!
    }

    //-----------------------------------------------------------
    // create output meta-data
    //-----------------------------------------------------------




    //-----------------------------------------------------------
    // create python object
    //-----------------------------------------------------------

    boost::python::object bmp;
    size_t osize  = vImage.samples() * vImage.width() * vImage.height();
    // UNSIGNED
    if      (vImage.depth()==8  && vImage.pixelType()==bim::FMT_UNSIGNED)
        bmp = boost::python::object(boost::python::handle<>(boost::python::borrowed (PyBuffer_FromMemory ((bim::uint8 *) &plhs, osize))));
    else if (vImage.depth()==16 && vImage.pixelType()==bim::FMT_UNSIGNED)
        bmp = boost::python::object(boost::python::handle<>(boost::python::borrowed (PyBuffer_FromMemory ((bim::uint16 *) &plhs, osize))));
    else if (vImage.depth()==32 && vImage.pixelType()==bim::FMT_UNSIGNED)
        bmp = boost::python::object(boost::python::handle<>(boost::python::borrowed (PyBuffer_FromMemory ((bim::uint32 *) &plhs, osize))));
    else // SIGNED
    if      (vImage.depth()==8  && vImage.pixelType()==bim::FMT_SIGNED)
        bmp = boost::python::object(boost::python::handle<>(boost::python::borrowed (PyBuffer_FromMemory ((bim::int8 *) &plhs, osize))));
    else if (vImage.depth()==16 && vImage.pixelType()==bim::FMT_SIGNED)
        bmp = boost::python::object(boost::python::handle<>(boost::python::borrowed (PyBuffer_FromMemory ((bim::int16 *) &plhs, osize))));
    else if (vImage.depth()==32 && vImage.pixelType()==bim::FMT_SIGNED)
        bmp = boost::python::object(boost::python::handle<>(boost::python::borrowed (PyBuffer_FromMemory ((bim::int32 *) &plhs, osize))));
    else // FLOAT
    if      (vImage.depth()==32 && vImage.pixelType()==bim::FMT_FLOAT) 
        bmp = boost::python::object(boost::python::handle<>(boost::python::borrowed (PyBuffer_FromMemory ((float *) &plhs, osize))));
    else if (vImage.depth()==64 && vImage.pixelType()==bim::FMT_FLOAT)
        bmp = boost::python::object(boost::python::handle<>(boost::python::borrowed (PyBuffer_FromMemory ((float *) &plhs, osize))));
  

   return bmp_wrapper(plhs, osize);
//    return boost::python::make_tuple(
//      bmp,
//      vFormatName,
//      vNumPages,
//      vPixelSize[0], vPixelSize[1], vPixelSize[2],
//      "dummy_metadata");
  }


  else {
    //mexErrMsgTxt("Failed to read image with mex_bimreadh5.");
  }

//   return bmp_wrapper(plhs, osize);
//  return boost::python::make_tuple(
//    0,
//    "",
//    0,
//    0, 0, 0,
//    "");
}

BOOST_PYTHON_MODULE(libbimreadh5)
{
    def("bimreadh5", boost_bimreadh5);
   // def("names", &names_wrapper);
}

