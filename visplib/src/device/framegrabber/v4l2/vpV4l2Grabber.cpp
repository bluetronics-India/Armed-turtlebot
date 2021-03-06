/****************************************************************************
 *
 * $Id: vpV4l2Grabber.cpp 3726 2012-05-11 17:39:07Z fspindle $
 *
 * This file is part of the ViSP software.
 * Copyright (C) 2005 - 2012 by INRIA. All rights reserved.
 * 
 * This software is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * ("GPL") version 2 as published by the Free Software Foundation.
 * See the file LICENSE.txt at the root directory of this source
 * distribution for additional information about the GNU GPL.
 *
 * For using ViSP with software that can not be combined with the GNU
 * GPL, please contact INRIA about acquiring a ViSP Professional 
 * Edition License.
 *
 * See http://www.irisa.fr/lagadic/visp/visp.html for more information.
 * 
 * This software was developed at:
 * INRIA Rennes - Bretagne Atlantique
 * Campus Universitaire de Beaulieu
 * 35042 Rennes Cedex
 * France
 * http://www.irisa.fr/lagadic
 *
 * If you have questions regarding the use of this file, please contact
 * INRIA at visp@inria.fr
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 *
 * Description:
 * Framegrabber based on Video4Linux2 driver.
 *
 * Authors:
 * Fabien Spindler
 *
 *****************************************************************************/

/*!
  \file vpV4l2Grabber.cpp
  \brief class for the Video For Linux 2 video device framegrabbing.
  \ingroup libdevice
*/

#include <visp/vpConfig.h>

#ifdef VISP_HAVE_V4L2

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <errno.h>
#include <iostream>

#include <visp/vpV4l2Grabber.h>
#include <visp/vpFrameGrabberException.h>
#include <visp/vpImageIo.h>

const unsigned int vpV4l2Grabber::DEFAULT_INPUT = 2;
const unsigned int vpV4l2Grabber::DEFAULT_SCALE = 2;
const __u32 vpV4l2Grabber::MAX_INPUTS    = 16;
const __u32 vpV4l2Grabber::MAX_NORM      = 16;
const __u32 vpV4l2Grabber::MAX_FORMAT    = 32;
const unsigned int vpV4l2Grabber::MAX_CTRL      = 32;
const unsigned int vpV4l2Grabber::MAX_BUFFERS   = 32;
const unsigned int vpV4l2Grabber::FRAME_SIZE    = 288;
#define vpCLEAR(x) memset (&(x), 0, sizeof (x))

/*!
  Default constructor.

  Setup the Video For Linux Two (V4L2) driver in streaming mode.

  Default settings are:

  - Device name: /dev/video0: To change it use setDevice()

  - Number of ring buffers: 3. To change this value use setNBuffers(). For non
    real-time applications the number of buffers should be set to 1. For
    real-time applications to reach 25 fps or 50 fps a good compromise is to
    set the number of buffers to 3.

  - Framerate acquisition: 25 fps. Use setFramerate() to set 25 fps or 50
    fps. These framerates are reachable only if enought buffers are set.

  - Input board: vpV4l2Grabber::DEFAULT_INPUT. Video input port. Use setInput()
    to change it.

  - Image size acquisition: vpV4l2Grabber::DEFAULT_SCALE. Use either setScale()
    or setWidth() and setHeight to change it.

    \code
    vpImage<unsigned char> I; // Grey level image

    vpV4l2Grabber g;
    g.setInput(2);    // Input 2 on the board
    g.setWidth(768);  // Acquired images are 768 width
    g.setHeight(576); // Acquired images are 576 height
    g.setNBuffers(3); // 3 ring buffers to ensure real-time acquisition
    g.open(I);        // Open the grabber

    g.acquire(I);     // Acquire a 768x576 grey image

    \endcode

*/
vpV4l2Grabber::vpV4l2Grabber()
{
  fd        = -1;
  streaming = false;
  verbose   = false;
  field     = 0;
  width     = 0;
  height    = 0;
  queue     = 0;
  waiton_cpt= 0;
  index_buffer = 0;

  inp       = NULL;
  std       = NULL;
  fmt       = NULL;
  ctl       = NULL;
  buf_v4l2  = NULL;
  buf_me    = NULL;

  setDevice("/dev/video0");
  setNBuffers(3);
  setFramerate(vpV4l2Grabber::framerate_25fps);
  setInput(vpV4l2Grabber::DEFAULT_INPUT);
  setScale(vpV4l2Grabber::DEFAULT_SCALE);
  setPixelFormat(V4L2_YUYV_FORMAT);

  init = false;
}

/*!
  Default constructor.

  Setup the Video For Linux Two (V4L2) driver in streaming mode.

  \param verbose : If true activates the verbose mode.

  Default settings are:

  - Device name: /dev/video0: To change it use setDevice()

  - Number of ring buffers: 3. To change this value use setNBuffers(). For non
    real-time applications the number of buffers should be set to 1. For
    real-time applications to reach 25 fps or 50 fps a good compromise is to
    set the number of buffers to 3.

  - Framerate acquisition: 25 fps. Use setFramerate() to set 25 fps or 50
    fps. These framerates are reachable only if enought buffers are set.

  - Input board: vpV4l2Grabber::DEFAULT_INPUT. Video input port. Use setInput()
    to change it.

  - Image size acquisition: vpV4l2Grabber::DEFAULT_SCALE. Use either setScale()
    or setWidth() and setHeight to change it.

    \code
    vpImage<unsigned char> I; // Grey level image

    vpV4l2Grabber g(true); // Activates the verbose mode
    g.setInput(2);    // Input 2 on the board
    g.setWidth(768);  // Acquired images are 768 width
    g.setHeight(576); // Acquired images are 576 height
    g.setNBuffers(3); // 3 ring buffers to ensure real-time acquisition
    g.open(I);        // Open the grabber

    g.acquire(I);     // Acquire a 768x576 grey image

    \endcode

*/
vpV4l2Grabber::vpV4l2Grabber(bool verbose)
{
  fd        = -1;
  streaming = false;
  this->verbose   = verbose;
  field     = 0;
  width     = 0;
  height    = 0;
  queue     = 0;
  waiton_cpt= 0;
  index_buffer = 0;

  inp       = NULL;
  std       = NULL;
  fmt       = NULL;
  ctl       = NULL;
  buf_v4l2  = NULL;
  buf_me    = NULL;

  setDevice("/dev/video0");
  setNBuffers(3);
  setFramerate(vpV4l2Grabber::framerate_25fps);
  setInput(vpV4l2Grabber::DEFAULT_INPUT);
  setScale(vpV4l2Grabber::DEFAULT_SCALE);
  setPixelFormat(V4L2_YUYV_FORMAT);

  init = false;
}


/*!
  Constructor.

  Setup the Video For Linux Two (V4L2) driver in streaming mode.

  \param input : Video input port.
  \param scale : Decimation factor.

  Default settings are:

  - Device name: /dev/video0: To change it use setDevice()

  - Number of ring buffers: 3. To change this value use setNBuffers(). For non
    real-time applications the number of buffers should be set to 1. For
    real-time applications to reach 25 fps or 50 fps a good compromise is to
    set the number of buffers to 3.

  - Framerate acquisition: 25 fps. Use setFramerate() to set 25 fps or 50
    fps. These framerates are reachable only if enought buffers are set.

    \code
    vpImage<unsigned char> I; // Grey level image

    vpV4l2Grabber g(1, 2); // Select input 1, and half full size resolution images.
    g.open(I);             // Open the grabber

    g.acquire(I);          // Acquire a 384x288 grey image

    \endcode
*/
vpV4l2Grabber::vpV4l2Grabber(unsigned input, unsigned scale)
{
  fd        = -1;
  streaming = false;
  verbose   = false;
  field     = 0;
  width     = 0;
  height    = 0;
  queue     = 0;
  waiton_cpt= 0;
  index_buffer = 0;

  inp       = NULL;
  std       = NULL;
  fmt       = NULL;
  ctl       = NULL;
  buf_v4l2  = NULL;
  buf_me    = NULL;

  setDevice("/dev/video0");
  setNBuffers(3);
  setFramerate(vpV4l2Grabber::framerate_25fps);
  setInput(input);
  setScale(scale);

  init = false;
}

/*!
  Constructor.

  Setup the Video For Linux Two (V4L2) driver in streaming mode.

  \param I : Image data structure (grey 8 bits image)
  \param input : Video input port.
  \param scale : Decimation factor.

  Default settings are:

  - Device name: /dev/video0: To change it use setDevice()

  - Number of ring buffers: 3. To change this value use setNBuffers(). For non
    real-time applications the number of buffers should be set to 1. For
    real-time applications to reach 25 fps or 50 fps a good compromise is to
    set the number of buffers to 3.

  - Framerate acquisition: 25 fps. Use setFramerate() to set 25 fps or 50
    fps. These framerates are reachable only if enought buffers are set.

    \code
    vpImage<unsigned char> I; // Grey level image

    vpV4l2Grabber g(I, 1, 2); // Select input 1, and half full size resolution
                              // images and open the grabber

    g.acquire(I);             // Acquire a 384x288 grey image

    \endcode
*/
vpV4l2Grabber::vpV4l2Grabber(vpImage<unsigned char> &I,
			     unsigned input, unsigned scale )
{
  fd        = -1;
  streaming = false;
  verbose   = false;
  field     = 0;
  width     = 0;
  height    = 0;
  queue     = 0;
  waiton_cpt= 0;
  index_buffer = 0;

  inp       = NULL;
  std       = NULL;
  fmt       = NULL;
  ctl       = NULL;
  buf_v4l2  = NULL;
  buf_me    = NULL;

  setDevice("/dev/video0");
  setNBuffers(3);
  setFramerate(vpV4l2Grabber::framerate_25fps);
  setInput(input);
  setScale(scale);

  init = false;

  open(I);
}

/*!
  Constructor.

  Setup the Video For Linux Two (V4L2) driver in streaming mode.

  \param I : Image data structure (Color RGB32 bits image)
  \param _input : Video input port.
  \param _scale : Decimation factor.

  Default settings are:

  - Device name: /dev/video0: To change it use setDevice()

  - Number of ring buffers: 3. To change this value use setNBuffers(). For non
    real-time applications the number of buffers should be set to 1. For
    real-time applications to reach 25 fps or 50 fps a good compromise is to
    set the number of buffers to 3.

  - Framerate acquisition: 25 fps. Use setFramerate() to set 25 fps or 50
    fps. These framerates are reachable only if enought buffers are set.

    \code
    vpImage<vpRGBa> I;        // Color image

    vpV4l2Grabber g(I, 1, 2); // Select input 1, and half full size resolution
                              // images and open the grabber

    g.acquire(I);             // Acquire a 384x288 color image

    \endcode

*/
vpV4l2Grabber::vpV4l2Grabber(vpImage<vpRGBa> &I, unsigned _input, unsigned _scale )
{
  fd        = -1;
  streaming = false;
  verbose   = false;
  field     = 0;
  width     = 0;
  height    = 0;
  queue     = 0;
  waiton_cpt= 0;
  index_buffer = 0;

  inp       = NULL;
  std       = NULL;
  fmt       = NULL;
  ctl       = NULL;
  buf_v4l2  = NULL;
  buf_me    = NULL;

  setDevice("/dev/video0");
  setNBuffers(3);
  setFramerate(vpV4l2Grabber::framerate_25fps);
  setInput(_input);
  setScale(_scale);

  init = false;

  open(I);
}

/*!
  Destructor.

  \sa close()
*/
vpV4l2Grabber::~vpV4l2Grabber()
{
  close() ;
}

/*!
  Set the video input port on the board.
*/
void
vpV4l2Grabber::setInput(unsigned input)
{
  this->input = input;
}

/*!
  Set the decimation factor applied to full resolution images (768x576).

  \exception vpFrameGrabberException::settingError : Wrong scale (shoud be
  between 1 and 16).

  \param scale : Decimation factor. If scale is set to 2, 384x288 images will
  be acquired.

  An other way to specify the image size is to use setWidth() and setHeight().

*/
void
vpV4l2Grabber::setScale(unsigned scale)
{
  if ((scale <1) || (scale >16))
  {
    close();

    vpERROR_TRACE("Wrong scale %d, scale shoud be between 1 and 16",scale) ;
    throw (vpFrameGrabberException(vpFrameGrabberException::settingError,
				   "Wrong scale") );
  }

  setWidth(640/scale);
  setHeight(480/scale);
}

/*!
  Initialize image acquisition in grey format.
  Set the pixel format acquisition to vpV4l2Grabber::V4L2_GREY_FORMAT.

  \param I : Image data structure (8 bits image). Once the device is open,
  the image is resized to the current acquisition size.

  \exception vpFrameGrabberException::settingError : Wrong input channel.

*/
void
vpV4l2Grabber::open(vpImage<unsigned char> &I)
{
  open();

  if( v4l2_ioctl (fd, VIDIOC_S_INPUT, &input) == -1 )
  {
    std::cout << "Warning: cannot set input channel to " << input << std::endl;
  }
  
  vpV4l2PixelFormatType req_pixelformat = getPixelFormat();

  try {
    setFormat();

    startStreaming();
  }
  catch(...) {
    if (verbose) {
      std::cout << "Requested pixel format [" << req_pixelformat 
		<< "] not compatible with camera" << std::endl;
      std::cout << "Try to found a compatible pixel format..." << std::endl;
    }
    
    // try to fing a compatible format
    for (int format=0; format< (int)V4L2_MAX_FORMAT; format ++) {
      if (format == req_pixelformat) {
	continue;
      }
      try {
	setPixelFormat((vpV4l2PixelFormatType)format);
	setFormat();
	startStreaming();
	if (verbose)
	  std::cout << "This format [" << pixelformat 
		    << "] is compatible with camera" << std::endl;
	
	break;
      }
      catch (...) {
	if (verbose)
	  std::cout << "This format [" << pixelformat 
		    << "] is not compatible with camera" << std::endl;
	if (format == (int)V4L2_MAX_FORMAT) {
	  std::cout << "No pixel format compatible with the camera was found" << std::endl;
	  close();

	  throw (vpFrameGrabberException(vpFrameGrabberException::settingError,
				   "No pixel format compatible with the camera was found"));
	}
      }
    }
  }

  I.resize(height, width) ;

  init = true;
}

/*!
  Initialize image acquisition in color RGB32 format.
  Set the pixel format acquisition to vpV4l2Grabber::V4L2_RGB32_FORMAT.


  \param I : Image data structure (RGB32 bits image). Once the device is open,
  the image is resized to the current acquisition size.

*/
void
vpV4l2Grabber::open(vpImage<vpRGBa> &I)
{
  open();

  if( v4l2_ioctl (fd, VIDIOC_S_INPUT, &input) == -1 )
  {
    std::cout << "Warning: cannot set input channel to " << input << std::endl;
  }

  vpV4l2PixelFormatType req_pixelformat = getPixelFormat();

  try {
    setFormat();

    startStreaming();
  }
  catch(...) {
    if (verbose) {
      std::cout << "Requested pixel format [" << pixelformat 
		<< "] not compatible with camera" << std::endl;
      std::cout << "Try to found a compatible pixel format..." << std::endl;
    }
    
    // try to fing a compatible format
    for (int format=0; format< (int)V4L2_MAX_FORMAT; format ++) {
      if (format == req_pixelformat) {
	continue;
      }
      try {
	setPixelFormat((vpV4l2PixelFormatType)format);
	setFormat();
	startStreaming();
	if (verbose)
	  std::cout << "This format [" << pixelformat 
		    << "] is compatible with camera" << std::endl;
	
	break;
      }
      catch (...) {
	if (verbose)
	  std::cout << "This format [" << pixelformat 
		    << "] is not compatible with camera" << std::endl;

      }
    }
  }

  I.resize(height, width) ;

  init = true;
}



/*!
  Acquire a grey level image.

  \param I : Image data structure (8 bits image)

  \exception vpFrameGrabberException::initializationError : Frame grabber not
  initialized.

  \sa getField()
*/
void
vpV4l2Grabber::acquire(vpImage<unsigned char> &I)
{
  struct timeval timestamp;

  acquire(I, timestamp);
}

/*!
  Acquire a grey level image.

  \param I : Image data structure (8 bits image).

  \param timestamp : Timeval data structure providing the unix time
  at which the frame was captured in the ringbuffer.

  \exception vpFrameGrabberException::initializationError : Frame grabber not
  initialized.

  \sa getField()
*/
void
vpV4l2Grabber::acquire(vpImage<unsigned char> &I, struct timeval &timestamp)
{

  if (init==false)
  {
    close();

    throw (vpFrameGrabberException(vpFrameGrabberException::initializationError,
				   "V4l2 frame grabber not initialized") );
  }

  unsigned  char *bitmap ;
  bitmap = waiton(index_buffer, timestamp);

  if ((I.getWidth() != width)||(I.getHeight() != height))
    I.resize(height, width) ;

  switch(pixelformat) {
  case V4L2_GREY_FORMAT:
    memcpy(I.bitmap, bitmap, height * width*sizeof(unsigned char));
    break;
  case V4L2_RGB24_FORMAT:
    vpImageConvert::RGBToGrey((unsigned char *) bitmap, I.bitmap, width*height);

    break;
  case V4L2_RGB32_FORMAT:
    vpImageConvert::RGBaToGrey((unsigned char *) bitmap, I.bitmap, width*height);

    break;
  case V4L2_BGR24_FORMAT:
    vpImageConvert::BGRToGrey( (unsigned char *) bitmap, I.bitmap, width, height, false);

    break;
  case V4L2_YUYV_FORMAT:
    vpImageConvert::YUYVToGrey( (unsigned char *) bitmap, I.bitmap, width*height);

    break;
  default:
    std::cout << "V4L2 conversion not handled" << std::endl;
    break;
  }

  queueAll();
}

/*!
  Acquire a color image.

  \param I : Image data structure (32 bits image)

  \exception vpFrameGrabberException::initializationError : Frame grabber not
  initialized.

  \sa getField()
*/
void
vpV4l2Grabber::acquire(vpImage<vpRGBa> &I)
{
  struct timeval timestamp;

  acquire(I, timestamp);
}

/*!
  Acquire a color image.

  \param I : Image data structure (32 bits image).

  \param timestamp : Timeval data structure providing the unix time
  at which the frame was captured in the ringbuffer.

  \exception vpFrameGrabberException::initializationError : Frame grabber not
  initialized.

  \sa getField()
*/
void
vpV4l2Grabber::acquire(vpImage<vpRGBa> &I, struct timeval &timestamp)
{

  if (init==false)
  {
    close();

    throw (vpFrameGrabberException(vpFrameGrabberException::initializationError,
				   "V4l2 frame grabber not initialized") );
  }

  unsigned  char *bitmap ;
  bitmap = waiton(index_buffer, timestamp);

  if ((I.getWidth() != width)||(I.getHeight() != height))
    I.resize(height, width) ;

  // The framegrabber acquire aRGB format. We just shift the data from 1 byte all the data and initialize the last byte

  switch(pixelformat) {
  case V4L2_GREY_FORMAT:
    vpImageConvert::GreyToRGBa((unsigned char *) bitmap, (unsigned char *) I.bitmap, width*height);
    break;
  case V4L2_RGB24_FORMAT:
    vpImageConvert::RGBToRGBa((unsigned char *) bitmap, (unsigned char *) I.bitmap, width*height);

    break;
  case V4L2_RGB32_FORMAT:
    // The framegrabber acquire aRGB format. We just shift the data
    // from 1 byte all the data and initialize the last byte
    memcpy(I.bitmap, bitmap + 1, height * width * sizeof(vpRGBa) - 1);
    I[height-1][width-1].A = 0;
    break;
  case V4L2_BGR24_FORMAT:
    vpImageConvert::BGRToRGBa((unsigned char *) bitmap, (unsigned char *) I.bitmap, width, height, false);
    break;
  case V4L2_YUYV_FORMAT:
    vpImageConvert::YUYVToRGBa( (unsigned char *) bitmap, (unsigned char *) I.bitmap, width, height); 
    break;
  default:
    std::cout << "V4l2 conversion not handled" << std::endl;
    break;
  }

  queueAll();
}
/*!

  Return the field (odd or even) corresponding to the last acquired
  frame.

  This method is to call after acquire() and has only a mean if the acquisition
  framerate is set to 50 fps.

  \return Field of the acquired frame (0 if odd field, 1 if even field).

  \exception vpFrameGrabberException::otherError : Video device returns a bad
  frame field.

  \sa acquire(), setFramerate()

*/
bool
vpV4l2Grabber::getField()
{
  if(field == 2) return 0; //top field
  else if (field == 3) return 1; //bottom field;
  else {
    close();

    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "V4l2 returns a bad frame field") );
    return false;
  }
}
/*!

  Set the frame format depending on the framerate acquisition.

  \param framerate : The framerate for the acquisition.
  - If vpV4l2Grabber::framerate_25fps use vpV4l2Grabber::V4L2_IMAGE_FORMAT,
  - else if vpV4l2Grabber::framerate_50fps use vpV4l2Grabber::V4L2_FRAME_FORMAT.
  \warning If you want to acquire frames at 25 fps or 50 fps, you have to be
  aware of the number of buffers required for the streaming. A typical value
  could be 3 (see setNBuffers()).

  \sa getFramerate(), setNBuffers()

*/
void
vpV4l2Grabber::setFramerate(vpV4l2Grabber::vpV4l2FramerateType framerate)
{
   this->framerate = framerate;

   if (framerate == vpV4l2Grabber::framerate_25fps)
     setFrameFormat(V4L2_IMAGE_FORMAT);
   else
     setFrameFormat(V4L2_FRAME_FORMAT);
}

/*!

  Return the framerate of the acquisition.

  \return The actual framerate of the framegrabber.

  \sa setFramerate()
*/


vpV4l2Grabber::vpV4l2FramerateType
vpV4l2Grabber::getFramerate()
{
  return framerate;
}


/*!
  Close the video device.
*/
void
vpV4l2Grabber::close()
{
  stopStreaming();
  streaming = false;

  if (fd >= 0){
    //vpTRACE("v4l2_close()");
    v4l2_close (fd);
    fd = -1;
  }

  if (inp != NULL) { delete [] inp; inp = NULL; }
  if (std != NULL) { delete [] std; std = NULL; }
  if (fmt != NULL) { delete [] fmt; fmt = NULL; }
  if (ctl != NULL) { delete [] ctl; ctl = NULL; }
  if (buf_v4l2 != NULL) { delete [] buf_v4l2; buf_v4l2 = NULL; }
  if (buf_me != NULL)   { delete [] buf_me; buf_me = NULL; }
}

/*!

  Open the Video For Linux Two device.

  \exception vpFrameGrabberException::initializationError : Can't access to
  video device.

  \exception vpFrameGrabberException::otherError : Can't query video
  capabilities.

*/
void
vpV4l2Grabber::open()
{
  /* Open Video Device */
  struct stat st; 
  
  if (-1 == stat (device, &st)) {
    fprintf (stderr, "Cannot identify '%s': %d, %s\n",
	     device, errno, strerror (errno));
    throw (vpFrameGrabberException(vpFrameGrabberException::initializationError,
				   "Cannot identify video device") );

  }
  
  if (!S_ISCHR (st.st_mode)) {
    fprintf (stderr, "%s is no device\n", device);
    throw (vpFrameGrabberException(vpFrameGrabberException::initializationError,
				   "No device") );
    
  }
  fd = v4l2_open (device, O_RDWR | O_NONBLOCK, 0);
  if (fd < 0) {
    close();

    vpERROR_TRACE ("No video device \"%s\"\n", device);
    throw (vpFrameGrabberException(vpFrameGrabberException::initializationError,
				   "Can't access to video device") );

  }

  if (inp != NULL) { delete [] inp; inp = NULL; }
  if (std != NULL) { delete [] std; std = NULL; }
  if (fmt != NULL) { delete [] fmt; fmt = NULL; }
  if (ctl != NULL) { delete [] ctl; ctl = NULL; }
  if (buf_v4l2 != NULL) { delete [] buf_v4l2; buf_v4l2 = NULL; }
  if (buf_me != NULL)   { delete [] buf_me; buf_me = NULL; }

  inp      = new struct v4l2_input     [vpV4l2Grabber::MAX_INPUTS];
  std      = new struct v4l2_standard  [vpV4l2Grabber::MAX_NORM];
  fmt      = new struct v4l2_fmtdesc   [vpV4l2Grabber::MAX_FORMAT];
  ctl      = new struct v4l2_queryctrl [vpV4l2Grabber::MAX_CTRL*2];
  buf_v4l2 = new struct v4l2_buffer    [vpV4l2Grabber::MAX_BUFFERS];
  buf_me   = new struct ng_video_buf   [vpV4l2Grabber::MAX_BUFFERS];

  /* Querry Video Device Capabilities */
  if ( v4l2_ioctl (fd, VIDIOC_QUERYCAP, &cap) == -1 ) {
    close();
    fprintf (stderr, "%s is no V4L2 device\n", device);
    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Is not a V4L2 device") );
  }
  if (verbose) {
    fprintf(stdout, "v4l2 info:\n"
	    "     device: %s\n"
	    "     %s %d.%d.%d / %s @ %s\n",
	    device,
	    cap.driver,
	    (cap.version >> 16) & 0xff,
	    (cap.version >>  8) & 0xff,
	    cap.version         & 0xff,
	    cap.card, cap.bus_info);
    if (cap.capabilities & V4L2_CAP_VIDEO_OVERLAY)
      fprintf(stdout, "     Support overlay\n");
    else
      fprintf(stdout, "     Does not support overlay\n");
    if (cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)
      fprintf(stdout, "     Support capture\n");
    else
      fprintf(stdout, "     Does not support capture\n");
    if (cap.capabilities & V4L2_CAP_TUNER)
      fprintf(stdout, "     Support tuning\n");
    else
      fprintf(stdout, "     Does not support tuning\n");
    if (cap.capabilities & V4L2_CAP_STREAMING)
      fprintf(stdout, "     Support streaming capture.\n");
    else
      fprintf(stdout, "     Does not support streaming capture.\n");
    if(cap.capabilities & V4L2_CAP_ASYNCIO)
      fprintf(stdout, "     Support asynchronous I/O methods.\n");
    else
      fprintf(stdout, "     Does not support asynchronous I/O methods.\n");
  }

  getCapabilities();
}

/*!

  Get device capabilities.

  \exception vpFrameGrabberException::otherError : Can't get video parameters.
*/
void
vpV4l2Grabber::getCapabilities()
{
  for (__u32 ninputs = 0; ninputs < MAX_INPUTS; ninputs++) {
    inp[ninputs].index = ninputs;
    if (v4l2_ioctl(fd, VIDIOC_ENUMINPUT, &inp[ninputs]))
      break;
  }
  for (__u32 nstds = 0; nstds < MAX_NORM; nstds++) {
    std[nstds].index = nstds;
    if (v4l2_ioctl(fd, VIDIOC_ENUMSTD, &std[nstds]))
      break;

  }
  for (__u32 nfmts = 0; nfmts < MAX_FORMAT; nfmts++) {
    fmt[nfmts].index = nfmts;
    fmt[nfmts].type  = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (v4l2_ioctl(fd, VIDIOC_ENUM_FMT, &fmt[nfmts]))
      break;
  }

  streamparm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (v4l2_ioctl(fd, VIDIOC_G_PARM, &streamparm) == -1)
  {
    close();

    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Can't get video parameters") );
  }
}

/*!

  Set the capture format using the settings:
  - image size : specified by setWidth(), setHeight() or setScale()
  - frame format : specified by setFramerate()
  - pixel format : specified by setPixelFormat().


  \exception vpFrameGrabberException::settingError : Bad format, probably do to
  a wrong scale.

  \exception vpFrameGrabberException::otherError : Can't get video format.
*/
void
vpV4l2Grabber::setFormat()
{
  fmt_me.width  = _width;
  fmt_me.height = _height;
  //fmt_me.bytesperline = _width; // bad (normally _width * depth / 8), but works
  // because initialized later by an ioctl call to VIDIOC_S_FMT

  switch(pixelformat) {
  case V4L2_GREY_FORMAT : fmt_me.pixelformat = V4L2_PIX_FMT_GREY; 
    if (verbose) 
      fprintf(stdout,"v4l2: new capture params (V4L2_PIX_FMT_GREY)\n");
    break;
  case V4L2_RGB24_FORMAT: fmt_me.pixelformat = V4L2_PIX_FMT_RGB24; 
    if (verbose) 
      fprintf(stdout,"v4l2: new capture params (V4L2_PIX_FMT_RGB24)\n");
    break;
  case V4L2_RGB32_FORMAT: fmt_me.pixelformat = V4L2_PIX_FMT_RGB32; 
    if (verbose) 
      fprintf(stdout,"v4l2: new capture params (V4L2_PIX_FMT_RGB32)\n");
    break;
  case V4L2_BGR24_FORMAT: fmt_me.pixelformat = V4L2_PIX_FMT_BGR24; 
    if (verbose) 
      fprintf(stdout,"v4l2: new capture params (V4L2_PIX_FMT_BGR24)\n");
    break;
  case V4L2_YUYV_FORMAT: fmt_me.pixelformat = V4L2_PIX_FMT_YUYV;
    if (verbose) 
      fprintf(stdout,"v4l2: new capture params (V4L2_PIX_FMT_YUYV)\n");
    break;

  default:
    close();

    throw (vpFrameGrabberException(vpFrameGrabberException::settingError,
				   "Bad format, probably do to a wrong scale"));
  }

  /* Get Video Format */
  vpCLEAR (fmt_v4l2);

  fmt_v4l2.type                 = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (v4l2_ioctl (fd, VIDIOC_G_FMT, &fmt_v4l2) == -1 ) {
    close();

    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Can't get video format") );
  }
  fmt_v4l2.fmt.pix.pixelformat  = fmt_me.pixelformat;
  fmt_v4l2.fmt.pix.width        = fmt_me.width;
  fmt_v4l2.fmt.pix.height       = fmt_me.height;
  //printf("1 - w: %d h: %d\n", fmt_v4l2.fmt.pix.width, fmt_v4l2.fmt.pix.height);


  switch (frameformat) {
  case V4L2_FRAME_FORMAT: fmt_v4l2.fmt.pix.field = V4L2_FIELD_ALTERNATE;
    if (verbose) {
      fprintf(stdout,"v4l2: new capture params (V4L2_FIELD_ALTERNATE)\n");
    }
    break;
  case V4L2_IMAGE_FORMAT: fmt_v4l2.fmt.pix.field = V4L2_FIELD_INTERLACED;
    if (verbose) {
      fprintf(stdout,"v4l2: new capture params (V4L2_FIELD_INTERLACED)\n");
    }
    break;
  default:
    close();

    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Unrecognized frame format") );
  }

  //height and width of the captured image or frame
  width = _width;
  height = _height;
  if( frameformat == V4L2_FRAME_FORMAT && height > FRAME_SIZE )
  {
    height = FRAME_SIZE;
  }
  //printf("2 - w: %d h: %d\n", fmt_v4l2.fmt.pix.width, fmt_v4l2.fmt.pix.height);

  if (v4l2_ioctl(fd, VIDIOC_S_FMT, &fmt_v4l2) == -1) {
    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Can't set video format") );
  }

  if (fmt_v4l2.fmt.pix.pixelformat != fmt_me.pixelformat) {
    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
  				   "Bad pixel format") );
  }

  /* Buggy driver paranoia. */
  unsigned int min = fmt_v4l2.fmt.pix.width * 2;
  if (fmt_v4l2.fmt.pix.bytesperline < min)
    fmt_v4l2.fmt.pix.bytesperline = min;
  min = fmt_v4l2.fmt.pix.bytesperline * fmt_v4l2.fmt.pix.height;
  if (fmt_v4l2.fmt.pix.sizeimage < min)
    fmt_v4l2.fmt.pix.sizeimage = min;

  fmt_me.width        = fmt_v4l2.fmt.pix.width;
  fmt_me.height       = fmt_v4l2.fmt.pix.height;
  fmt_me.bytesperline = fmt_v4l2.fmt.pix.bytesperline;

  if (verbose) {
    fprintf(stdout,"v4l2: new capture params (%dx%d, %c%c%c%c, %d byte, %d bytes per line)\n",
	    fmt_me.width, fmt_me.height,
	    fmt_v4l2.fmt.pix.pixelformat & 0xff,
	    (fmt_v4l2.fmt.pix.pixelformat >>  8) & 0xff,
	    (fmt_v4l2.fmt.pix.pixelformat >> 16) & 0xff,
	    (fmt_v4l2.fmt.pix.pixelformat >> 24) & 0xff,
	    fmt_v4l2.fmt.pix.sizeimage,
	    fmt_v4l2.fmt.pix.bytesperline);
  }

}
/*!

  Launch the streaming capture mode and map device memory into application
  address space.

  \exception vpFrameGrabberException::otherError : If a problem occurs.

*/
void
vpV4l2Grabber::startStreaming()
{
  if (streaming == true) { // Acquisition in process.
    stopStreaming();
    streaming = false;
  }

  /* setup buffers */
  memset (&(reqbufs), 0, sizeof (reqbufs));
  reqbufs.count  = nbuffers;
  reqbufs.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbufs.memory = V4L2_MEMORY_MMAP;

  
  if (v4l2_ioctl(fd, VIDIOC_REQBUFS, &reqbufs) == -1)
  {
    if (EINVAL == errno) {
      fprintf (stderr, "%s does not support "
                                 "memory mapping\n", device);
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "Does not support memory mapping") );
    }
    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Can't require video buffers") );
  }

  for (unsigned i = 0; i < reqbufs.count; i++) {
    // Clear the buffer
    memset (&(buf_v4l2[i]), 0, sizeof (buf_v4l2[i]));
    buf_v4l2[i].index  = i;
    buf_v4l2[i].type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf_v4l2[i].memory = V4L2_MEMORY_MMAP;
    buf_v4l2[i].length = 0;
    if (v4l2_ioctl(fd, VIDIOC_QUERYBUF, &buf_v4l2[i]) == -1)
    {
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Can't query video buffers") );
    }
    memcpy(&buf_me[i].fmt, &fmt_me, sizeof(ng_video_fmt));
    buf_me[i].size = buf_me[i].fmt.bytesperline * buf_me[i].fmt.height;

    // if (verbose)
    //   std::cout << "1: buf_v4l2[" << i << "].length: " << buf_v4l2[i].length
    // 	   << " buf_v4l2[" << i << "].offset: " <<  buf_v4l2[i].m.offset
    // 	   << std::endl;


    buf_me[i].data = (unsigned char *) v4l2_mmap(NULL, buf_v4l2[i].length,
						 PROT_READ | PROT_WRITE, 
						 MAP_SHARED,
						 fd, (off_t)buf_v4l2[i].m.offset);

    if(buf_me[i].data == MAP_FAILED)
    {
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Can't map memory") );
    }

    buf_me[i].refcount = 0;

//     if (verbose)
//     {
//       std::cout << "2: buf_v4l2[" << i << "].length: " << buf_v4l2[i].length
// 	   << " buf_v4l2[" << i << "].offset: " <<  buf_v4l2[i].m.offset
// 	   << std::endl;
//       std::cout << "2: buf_me[" << i << "].size: " << buf_me[i].size << std::endl;
//     }

    if (verbose)
      printBufInfo(buf_v4l2[i]);
  }

  /* queue up all buffers */
  queueAll();

  /* Set video stream capture on */
  if (v4l2_ioctl(fd, VIDIOC_STREAMON, &fmt_v4l2.type)<0)
  {
    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Can't start streaming") );
  }

  streaming = true;
}

/*!

  Stops the streaming capture mode and unmap the device memory.

  \exception vpFrameGrabberException::otherError : if can't stop streaming.
*/
void
vpV4l2Grabber::stopStreaming()
{
  unsigned int i;

  //nothing to do if (fd < 0) or if  (streaming == false)
  if ((fd >= 0) && (streaming == true)) {

    //vpTRACE(" Stop the streaming...");
    /* stop capture */
    fmt_v4l2.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (v4l2_ioctl(fd, VIDIOC_STREAMOFF,&fmt_v4l2.type)) {
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "Can't stop streaming") );
    }
    /* free buffers */
    for (i = 0; i < reqbufs.count; i++) {
      if (verbose)
	printBufInfo(buf_v4l2[i]);
      //vpTRACE("v4l2_munmap()");

      if (-1 == v4l2_munmap(buf_me[i].data, buf_me[i].size)) {
	throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				       "Can't unmap memory") );
      }
    }
    queue = 0;
    waiton_cpt = 0;
    streaming = false;
  }
}

/*!
  Fill the next buffer. If all the buffers are filled return NULL.

  Update the buffer index. If all the buffers are filled index is set to -1.

  \param index : Index in the buffer where image is available.

  \param timestamp : Timeval data structure providing the unix time
  [microseconds] at which the frame was captured in the ringbuffer.

  \exception vpFrameGrabberException::otherError : If can't access to the
  frame.
*/
unsigned char *
vpV4l2Grabber::waiton(__u32 &index, struct timeval &timestamp)
{
  struct v4l2_buffer buf;
  struct timeval tv;
  fd_set rdset;

  /* wait for the next frame */
 again:

  tv.tv_sec  = 30;
  tv.tv_usec = 0;
  FD_ZERO(&rdset);
  FD_SET(static_cast<unsigned int>(fd), &rdset);
  switch (select(fd + 1, &rdset, NULL, NULL, &tv)) {
  case -1:
    if (EINTR == errno)
      goto again;
    index = 0;
    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Can't access to the frame") );
    return NULL;
  case  0:
    index = 0;
    throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				   "Can't access to the frame: timeout") );
    return NULL;
  }


  /* get it */
  memset(&buf, 0, sizeof(buf));
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP; // Fabien manquait
  if (-1 == v4l2_ioctl(fd,VIDIOC_DQBUF, &buf)) {
    index = 0;
    switch(errno)
    {
    case EAGAIN:
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "VIDIOC_DQBUF: EAGAIN") );
      break;
    case EINVAL:
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "VIDIOC_DQBUF: EINVAL") );
      break;
    case ENOMEM:
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "VIDIOC_DQBUF: ENOMEM") );
      break;
    default:
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "VIDIOC_DQBUF") );
      break;
    }
    return NULL;
  }

  waiton_cpt++;
  buf_v4l2[buf.index] = buf;

  index = buf.index;

  field = buf_v4l2[index].field;

  timestamp = buf_v4l2[index].timestamp;

  // if(verbose)
  // {
  //   vpERROR_TRACE("field: %d\n", buf_v4l2[index].field);

  //   vpERROR_TRACE("data adress : 0x%p\n", buf_me[buf.index].data);
  // }
  return buf_me[buf.index].data;
}

/*!

 Capture helpers.

*/
int
vpV4l2Grabber::queueBuffer()
{
  unsigned int frame = queue % reqbufs.count;
  int rc;


  if (0 != buf_me[frame].refcount) {
    if (0 != queue - waiton_cpt)
      return -1;
    fprintf(stderr,"v4l2: waiting for a free buffer..............\n");
    //ng_waiton_video_buf(h->buf_me+frame);
    std::cout << "Normalement call ng_waiton_video_buf(buf_me+frame); --------\n";
  }

  //    std::cout << "frame: " << frame << std::endl;
  rc = v4l2_ioctl(fd, VIDIOC_QBUF, &buf_v4l2[frame]);
  if (0 == rc)
    queue++;
  else
  {
    switch(errno)
    {
    case EAGAIN:
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "VIDIOC_QBUF: EAGAIN") );
      break;
    case EINVAL:
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "VIDIOC_QBUF: EINVAL") );
      break;
    case ENOMEM:
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "VIDIOC_QBUF: ENOMEM") );
      break;
    default:
      throw (vpFrameGrabberException(vpFrameGrabberException::otherError,
				     "VIDIOC_QBUF") );
      break;
    }
  }
  return rc;
}

/*!

  Call the queue buffer private method if needed

*/
void
vpV4l2Grabber::queueAll()
{
  for (;;) {
    if (queue - waiton_cpt >= reqbufs.count) {
      return;
    }
    if (0 != queueBuffer()) {
      return;
    }
  }
}

/*!

  Get device capabilities.

*/
void
vpV4l2Grabber::printBufInfo(struct v4l2_buffer buf)
{
  char type[40];

  switch(buf.type) {
  case V4L2_BUF_TYPE_VIDEO_CAPTURE: sprintf(type, "video-cap"); break;
  case V4L2_BUF_TYPE_VIDEO_OVERLAY: sprintf(type, "video-over"); break;
  case V4L2_BUF_TYPE_VIDEO_OUTPUT:  sprintf(type, "video-out"); break;
  case V4L2_BUF_TYPE_VBI_CAPTURE:   sprintf(type, "vbi-cap"); break;
  case V4L2_BUF_TYPE_VBI_OUTPUT:    sprintf(type, "vbi-out"); break;
  default:                          sprintf(type, "unknown"); break;
  }

  fprintf(stdout,"v4l2: buf %d: %d ad: 0x%lx offset 0x%x+%d (=0x%x),used %d\n",
	  buf.index, buf.type, buf.m.userptr, buf.m.offset,
	  buf.length, buf.length, buf.bytesused);

}
#endif
