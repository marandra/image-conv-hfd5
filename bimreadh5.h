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
   [im, format, pages, xyzr, metatxt] = bimreadh5( fname, page_num );

   [im] = bimreadh5( fname );

  Notes:
    Currently only creates 8 or 16 bit arrays!!!
    Extend if you want to read other type of data.


  Author: Dima V. Fedorov <mailto:dima@dimin.net> <http://www.dimin.net/>

  History:
    10/13/2006 16:00 - First creation

  Ver : 4
*******************************************************************************/
#ifndef BIMREAD_HH
#define BIMREAD_HH BIMREAD_HH


// imgcnv includes:
#include <bim_img_format_interface.h>
#include <bim_img_format_utils.h>
#include <bim_image.h>
#include <bim_format_manager.h>
#include <meta_format_manager.h>


// declaration of the function(s):
int bimreadh5(bim::Image& aImage, std::string& aFormatName, int& aNumPages, double aPixelSize[], bim::TagMap& aMetaDataMap, const std::string aFileName, const int aPage);

template <typename T>
void copy_data(const bim::Image& aImage, T* aOutPointer) {
int count=0;

  for (unsigned int c=0; c<aImage.samples(); ++c) {
    for (unsigned int x=0; x<aImage.width(); ++x) {
      for (unsigned int y=0; y<aImage.height(); ++y) {
        T* vImageScanlinePointer = (T*)aImage.scanLine(c, y);
        *aOutPointer = vImageScanlinePointer[x];
if (count<100){ fprintf(stdout,"%d ",*aOutPointer); ++count;}
        ++aOutPointer;
      }
    }
  }
fprintf(stdout,"\n");
}

#endif
