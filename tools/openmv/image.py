# SPDX-License-Identifier: MIT
#
# Copyright (c) 2025 OpenMV, LLC.
#
# OpenMV Image Utilities
#
# This module provides image format conversion utilities for OpenMV camera data.
# Handles conversion from various pixel formats to RGB888 for display purposes.

import logging
try:
    import numpy as np
except ImportError:
    np = None
try:
    from PIL import Image
except ImportError:
    Image = None

# Pixel format constants
PIXFORMAT_GRAYSCALE = 0x08020001  # 1 byte per pixel
PIXFORMAT_RGB565 = 0x0C030002     # 2 bytes per pixel
PIXFORMAT_JPEG = 0x06060000       # Variable size JPEG

def convert_to_rgb888(raw_data, width, height, pixformat):
    """
    Convert various pixel formats to RGB888.
    
    Args:
        raw_data (bytes): Raw image data
        width (int): Image width
        height (int): Image height  
        pixformat (int): Pixel format identifier
        
    Returns:
        tuple: (rgb_data, format_string) where rgb_data is bytes or None on error
    """
    
    if pixformat == PIXFORMAT_GRAYSCALE:
        return _convert_grayscale(raw_data, width, height)
    elif pixformat == PIXFORMAT_RGB565:
        return _convert_rgb565(raw_data, width, height)
    elif pixformat == PIXFORMAT_JPEG:
        return _convert_jpeg(raw_data, width, height)
    else:
        # Unknown format - return raw data and let caller handle it
        fmt_str = f"0x{pixformat:08X}"
        logging.warning(f"Unknown pixel format: {fmt_str}")
        return raw_data, fmt_str

def _convert_grayscale(raw_data, width, height):
    """Convert grayscale to RGB888"""
    fmt_str = "GRAY"
    
    if np is None:
        logging.error("numpy required for grayscale conversion")
        return None, fmt_str
        
    # Convert grayscale to RGB by duplicating the gray value
    gray_array = np.frombuffer(raw_data, dtype=np.uint8)
    if len(gray_array) != width * height:
        logging.error(f"Grayscale data size mismatch: expected {width * height}, got {len(gray_array)}")
        return None, fmt_str
    
    rgb_array = np.column_stack((gray_array, gray_array, gray_array))
    return rgb_array.tobytes(), fmt_str

def _convert_rgb565(raw_data, width, height):
    """Convert RGB565 to RGB888"""
    fmt_str = "RGB565"
    
    if np is None:
        logging.error("numpy required for RGB565 conversion")
        return None, fmt_str
        
    # Convert RGB565 to RGB888
    rgb565_array = np.frombuffer(raw_data, dtype=np.uint16)
    if len(rgb565_array) != width * height:
        logging.error(f"RGB565 data size mismatch: expected {width * height}, got {len(rgb565_array)}")
        return None, fmt_str
        
    # Extract RGB components from 16-bit RGB565
    r = (((rgb565_array & 0xF800) >> 11) * 255.0 / 31.0).astype(np.uint8)
    g = (((rgb565_array & 0x07E0) >> 5) * 255.0 / 63.0).astype(np.uint8)
    b = (((rgb565_array & 0x001F) >> 0) * 255.0 / 31.0).astype(np.uint8)
    
    rgb_array = np.column_stack((r, g, b))
    return rgb_array.tobytes(), fmt_str

def _convert_jpeg(raw_data, width, height):
    """Convert JPEG to RGB888"""
    fmt_str = "JPEG"
    
    if Image is None:
        logging.error("PIL/Pillow required for JPEG conversion")
        return None, fmt_str
        
    try:
        # Decode JPEG to RGB
        image = Image.frombuffer("RGB", (width, height), raw_data, "jpeg", "RGB", "")
        rgb_array = np.asarray(image) if np else None
        
        if rgb_array is not None:
            if rgb_array.size != (width * height * 3):
                logging.error(f"JPEG decode size mismatch: expected {width * height * 3}, got {rgb_array.size}")
                return None, fmt_str
            return rgb_array.tobytes(), fmt_str
        else:
            # Fallback without numpy
            return image.tobytes(), fmt_str
            
    except Exception as e:
        logging.error(f"JPEG decode error: {e}")
        return None, fmt_str

def get_format_string(pixformat):
    """Get a human-readable format string from pixel format code"""
    format_map = {
        PIXFORMAT_GRAYSCALE: "GRAY",
        PIXFORMAT_RGB565: "RGB565", 
        PIXFORMAT_JPEG: "JPEG"
    }
    return format_map.get(pixformat, f"0x{pixformat:08X}")