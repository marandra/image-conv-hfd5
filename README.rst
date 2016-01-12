image-conv-hfd5
===============
Image format convertion tool for images from and to HDF5 containers

Description
-----------
Converts an image to format. Image can be inside H5containter/group (it is assumed that images are inside a group different from root). With the -b flag, the program creates (or overwrites) a container with the group structure from the input H5 container

Usage
-----
./bimconv_debug [-p page] [-f format] -i /path/to/image [-d /dest/path/]
./bimconv_debug [-p page] [-f format] -i /path/to/image [-d /dest/path/] [-O H5/group]
./bimconv_debug [-p page] [-f format] -I /path/to/H5in/groupin/image [-d /dest/path/]
./bimconv_debug [-p page] [-f format] -I /path/to/H5in/groupin/image [-d /dest/path/] [-O H5/group]
./bimconv_debug [-p page] [-f format] -b H5in [-d /dest/path/]

Arguments
---------
-i input path + file (output image has original name with extension replaced)
-I input path + H5 + group + file
-d output directory (output image has original name with extension replaced)
-O H5 + group
-b input path + H5 (output is H5-convrt)
-p page (default: 0)
-f format (default: jp2)

Performance
-----------
Execution time for 100 images in a directory is 34s

Technical characteristics
-------------------------
1. Uses imgcnv library for the general image handling
2. Uses openjpeg for the reading / writing
3. Saves Metadata string in JP2 image (openjpeg doesn't read these comments from JP2 images, it shouldn't be much work to ask them for help implementing it. As a workaround it can be accessed by Matlab's imfinfo)
4. Read images from HDF5 container
5. Write images to HDF5 container
6. Uses ram-disk for the conversion process
7. Can convert from and to other image formats

