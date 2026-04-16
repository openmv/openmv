
  @verbatim
  ******************************************************************************
  *
  *  Copyright (C) 2015-2022, Verisilicon Inc. - All Rights Reserved
  *
  *  Copyright (C) 2011-2014, Google Inc. - All Rights Reserved
  *
  * @file    st_readme.txt
  * @author  GPM Application Team
  * @brief   This file lists the main modification done by STMicroelectronics on
  *          VERISILICON Video Encoder software package.
  ******************************************************************************
  *
  *  Original licensing conditions by Verisilicon Inc. 
  *
  *  "This software is dual licensed, either GPL-2.0 or BSD-3-Clause, at your
  *   option."
  *
  ******************************************************************************
  @endverbatim


### 17-December-2024 ###
=========================
  + Prevent infinite loop in case of encoding timeout
     in \source\h264\H264CodeFrame.c


### 22-October-2024 ###
=========================
  + Add fuse error handling 
     in \inc\h264encapi.h
     in \source\common\encasiccontroller.c
     in \source\h264\H264CodeFrame.c
     in \source\h264\H264CodeFrame.h
     in \source\h264\H264EncApi.c
  + Change include files order to avoid typedef conflicts
     in \source\h264\H264EncApi.c 
     in \source\h264\H264Init.c   


### 25-June-2024 ###
=========================
  + Fix output overflow bug
     lower output buffer size in \source\common\encasiccontroller.c


### 08-December-2023 ###
=========================
  + Correct input RGB32 endianness
     Set ENCH1_INPUT_SWAP_32_RGB32 to 1 in \source\common\enccfg.h


### 29-September-2023 ###
=========================
  + Deliver VERISILICON VC8000NanoE SW package
   Version: 9.22.3.7
   Release Date: Nov. 07.2022

   Delivery of software header (.h) and source (.c) files.
   License set to BSD 3-Clause license.




