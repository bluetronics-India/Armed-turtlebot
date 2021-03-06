/****************************************************************************
 *
 * $Id: calibrateTsai.cpp 3619 2012-03-09 17:28:57Z fspindle $
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
 * Tsai calibration example to estimate hand to eye transformation.
 *
 * Authors:
 * Fabien Spindler
 *
 *****************************************************************************/


/*!
  \example calibrateTsai.cpp
  \brief Example of Tsai calibration to estimate extrinsic camera parameters, ie hand-eye homogeneous transformation.

*/

#include <visp/vpDebug.h>
#include <visp/vpParseArgv.h>
#include <visp/vpIoTools.h>
#include <visp/vpCalibration.h>
#include <visp/vpExponentialMap.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>

int main()
{
  // We want to calibrate the hand to eye extrinsic camera parameters from 6 couple of poses: cMo and wMe
  const int N = 6;
  // Input: six couple of poses used as input in the calibration proces
  vpHomogeneousMatrix cMo[N] ; // eye (camera) to object transformation. The object frame is attached to the calibrartion grid
  vpHomogeneousMatrix wMe[N] ; // world to hand (end-effector) transformation
  // Output: Result of the calibration
  vpHomogeneousMatrix eMc; // hand (end-effector) to eye (camera) transformation 

  // Initialize an eMc transformation used to produce the simulated input transformations cMo and wMe
  vpTranslationVector etc(0.1, 0.2, 0.3); 
  vpThetaUVector erc;
  erc[0] = vpMath::rad(10); // 10 deg
  erc[1] = vpMath::rad(-10); // -10 deg
  erc[2] = vpMath::rad(25); // 25 deg

  eMc.buildFrom(etc, erc);
  std::cout << "Simulated hand to eye transformation: eMc " << std::endl ;
  std::cout << eMc << std::endl ;
  std::cout << "Theta U rotation: " << vpMath::deg(erc[0]) << " " << vpMath::deg(erc[1]) << " " << vpMath::deg(erc[2]) << std::endl;

  vpColVector v_c(6) ; // camera velocity used to produce 6 simulated poses
  for (int i=0 ; i < N ; i++)
  {
    v_c = 0 ;
    if (i==0) {
      // Initialize first poses 
      cMo[0].buildFrom(0, 0, 0.5, 0, 0, 0); // z=0.5 m
      wMe[0].buildFrom(0, 0, 0, 0, 0, 0); // Id
    }
    else if (i==1)
      v_c[3] = M_PI/8 ;
    else if (i==2)
      v_c[4] = M_PI/8 ;
    else if (i==3)
      v_c[5] = M_PI/10 ;
    else if (i==4)
      v_c[0] = 0.5 ;
    else if (i==5)
      v_c[1] = 0.8 ;

    vpHomogeneousMatrix cMc; // camera displacement
    cMc = vpExponentialMap::direct(v_c) ; // Compute the camera displacement due to the velocity applied to the camera
    if (i > 0) {
      // From the camera displacement cMc, compute the wMe and cMo matrixes
      cMo[i] = cMc.inverse() * cMo[i-1];
      wMe[i] = wMe[i-1] * eMc * cMc * eMc.inverse();
    }
  }

  if (0) {
    for (int i=0 ; i < N ; i++) {
      vpHomogeneousMatrix wMo;
      wMo = wMe[i] * eMc * cMo[i];
      std::cout << std::endl << "wMo[" << i << "] " << std::endl ;
      std::cout << wMo << std::endl ;
      std::cout << "cMo[" << i << "] " << std::endl ;
      std::cout << cMo[i] << std::endl ;
      std::cout << "wMe[" << i << "] " << std::endl ;
      std::cout << wMe[i] << std::endl ;
    }
  }

  // Reset the eMc matrix to eye
  eMc.eye();

  // Compute the eMc hand to eye transformation from six poses
  // - cMo[6]: camera to object poses as six homogeneous transformations
  // - wMe[6]: world to hand (end-effector) poses as six homogeneous transformations
  vpCalibration::calibrationTsai(N, cMo, wMe, eMc) ;

  std::cout << std::endl << "Output: hand to eye calibration result: eMc estimated " << std::endl ;
  std::cout << eMc << std::endl ;
  eMc.extract(erc);
  std::cout << "Theta U rotation: " << vpMath::deg(erc[0]) << " " << vpMath::deg(erc[1]) << " " << vpMath::deg(erc[2]) << std::endl;
  return 0 ;
}

/*
 * Local variables:
 * c-basic-offset: 2
 * End:
 */
