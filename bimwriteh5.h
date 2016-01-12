/***************************************************************************
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                  *
 *                                                                         *
 *         Title:                                                          *
 *   Description:                                                          *
 *                                                                         *
 ***************************************************************************/
#ifndef BIMWRITEH5_HH
#define BIMWRITEH5_HH BIMWRITEH5_HH

// our libH5AR include:
#include <libH5AR.hh>

// imgcnv includes:
#include <bim_img_format_interface.h>
#include <bim_img_format_utils.h>
#include <bim_image.h>
#include <bim_format_manager.h>
#include <meta_format_manager.h>

#include <iostream>
#include <vector>
#include <cstdlib>
#include <cmath>
#include <string.h>
#include <stdio.h>



//------------------------------------------------------------------------------
// function: bimwriteh5 - entry point from Matlab via mexFucntion()
// INPUTS:
//------------------------------------------------------------------------------

template <class T>
int bimwriteh5(const T* aImage, const std::string aFileName, const std::string aFileFormat, const unsigned int aWidth, const unsigned int aHeight, const int nbrOfChannels, const int depth)
{
  bim::MetaFormatManager fm;
  bim::ImageBitmap* bmp = new bim::ImageBitmap;

  (bmp->i).width = aWidth;
  (bmp->i).height = aHeight;
  (bmp->i).samples = nbrOfChannels;
  (bmp->i).depth = depth;
  (bmp->i).number_pages = 0;
  (bmp->i).number_levels = 0;

  int vRotatedImageIdx = 0;
  for (size_t i = 0; i < nbrOfChannels; i++){
    bmp->bits[i] = new T[aWidth*aHeight];

    std::vector<T> vRotatedImage(aWidth*aHeight);
    for (int y = 0; y < aHeight; ++y) {
      for (int x = 0; x < aWidth; ++x) {
        vRotatedImage[vRotatedImageIdx] = aImage[x*aHeight + y + i*(aWidth*aHeight)];
        ++vRotatedImageIdx;
      }
    }
    memcpy(bmp->bits[i], &vRotatedImage[0], (aWidth*aHeight)*sizeof(T));
  }

  // TODO FIXME:
  // this is missing error handling?

  const char* options = "";
  fm.sessionStartWrite((bim::Filename)aFileName.c_str(), aFileFormat.c_str(), options);

  int status = fm.sessionWriteImage(bmp, 0);

  fm.sessionEnd();

  free(bmp);

  return status;
}

#endif
