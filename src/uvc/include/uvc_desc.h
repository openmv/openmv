/*
 * uvc_desc.h
 *
 *  Created on: Nov 23, 2017
 *      Author: kurt
 */

#ifndef ST_STM32_USB_DEVICE_LIBRARY_CLASS_VIDEO_INC_UVC_DESC_H_
#define ST_STM32_USB_DEVICE_LIBRARY_CLASS_VIDEO_INC_UVC_DESC_H_

#include "usbd_desc.h"
#include "usbd_types.h"
#include "uvc.h"

// consistent macro expansion of VS_NUM_FORMATS
#define _UVC_INPUT_HEADER_DESCRIPTOR(n, p) UVC_INPUT_HEADER_DESCRIPTOR(n, p)

DECLARE_UVC_HEADER_DESCRIPTOR(1);
DECLARE_UVC_FRAME_UNCOMPRESSED(1);
DECLARE_UVC_EXTENSION_UNIT_DESCRIPTOR(1, 4);
DECLARE_UVC_EXTENSION_UNIT_DESCRIPTOR(1, 8);
DECLARE_UVC_INPUT_HEADER_DESCRIPTOR(1, VS_NUM_FORMATS);
DECLARE_UVC_FRAMES_FORMAT_UNCOMPRESSED(3);
DECLARE_UVC_FRAMES_FORMAT_UNCOMPRESSED(4);

struct uvc_vs_frames_formats_descriptor {
  struct UVC_FRAMES_FORMAT_UNCOMPRESSED(4) uvc_vs_frames_format_1;
  struct UVC_FRAMES_FORMAT_UNCOMPRESSED(3) uvc_vs_frames_format_2;
  struct UVC_FRAMES_FORMAT_UNCOMPRESSED(3) uvc_vs_frames_format_3;
};

struct usbd_uvc_cfg {
  struct usb_config_descriptor usb_configuration;
  struct usb_interface_assoc_descriptor usb_uvc_association;

  struct usb_interface_descriptor uvc_vc_if_desc;
  struct UVC_HEADER_DESCRIPTOR(1) ucv_vc_header;
  struct uvc_camera_terminal_descriptor uvc_vc_input_terminal;
  struct uvc_processing_unit_descriptor uvc_vc_processing_unit;
  struct UVC_EXTENSION_UNIT_DESCRIPTOR(1, 4) uvc_vc_xu_lep_agc;
  struct UVC_EXTENSION_UNIT_DESCRIPTOR(1, 4) uvc_vc_xu_lep_oem;
  struct UVC_EXTENSION_UNIT_DESCRIPTOR(1, 8) uvc_vc_xu_lep_rad;
  struct UVC_EXTENSION_UNIT_DESCRIPTOR(1, 4) uvc_vc_xu_lep_sys;
  struct UVC_EXTENSION_UNIT_DESCRIPTOR(1, 4) uvc_vc_xu_lep_vid;
  struct UVC_EXTENSION_UNIT_DESCRIPTOR(1, 4) uvc_vc_xu_lep_rad_2;
  struct UVC_EXTENSION_UNIT_DESCRIPTOR(1, 4) uvc_vc_xu_lep_cust;
  struct uvc_output_terminal_descriptor uvc_vc_output_terminal;
  struct usb_endpoint_descriptor uvc_vc_ep;
  struct uvc_control_endpoint_descriptor uvc_vc_cs_ep;

  struct usb_interface_descriptor uvc_vs_if_alt0_desc;
  struct _UVC_INPUT_HEADER_DESCRIPTOR(1, VS_NUM_FORMATS) uvc_vs_input_header_desc;
  struct uvc_vs_frames_formats_descriptor uvc_vs_frames_formats_desc;

  struct usb_interface_descriptor uvc_vs_if_alt1_desc;
  struct usb_endpoint_descriptor uvc_vs_if_alt1_ep;
} __attribute__ ((packed));

extern struct usbd_uvc_cfg USBD_UVC_CfgFSDesc;

#endif /* ST_STM32_USB_DEVICE_LIBRARY_CLASS_VIDEO_INC_UVC_DESC_H_ */
