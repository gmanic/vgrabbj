/* Simple Video4Linux image grabber. Made for my Philips Vesta Pro
 * 
 * Copyright (C) 2000, 2001, 2002 Jens Gecius, Hannover, Germany
 * eMail: devel@gecius.de
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at you option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston MA 02111-1307,
 * USA  
 */  

/* Includes */

static struct palette_list plist[] = {
  { 0, NULL, 0, 0 },
  { VIDEO_PALETTE_GREY,   "GREY",    1, 1 },
  { VIDEO_PALETTE_HI240,  "HI240",   0, 1 },
  { VIDEO_PALETTE_RGB565, "RGB565",  0, 1 },
  { VIDEO_PALETTE_RGB24,  "RGB24",   3, 1 },
  { VIDEO_PALETTE_RGB32,  "RGB32",   4, 1 },
  { VIDEO_PALETTE_RGB555, "RGB555",  0, 1 },
  { VIDEO_PALETTE_YUV422, "YUV422",  2, 1 },
  { VIDEO_PALETTE_YUYV,   "YUYV",    2, 1 },
  { VIDEO_PALETTE_UYVY,   "UYVY",    2, 1 },
  { VIDEO_PALETTE_YUV420, "YUV420",  3, 2 },
  { VIDEO_PALETTE_YUV411, "YUV411",  0, 1 },
  { VIDEO_PALETTE_RAW,    "RAW",     0, 1 },
  { VIDEO_PALETTE_YUV422P,"YUV422P", 0, 1 },
  { VIDEO_PALETTE_YUV411P,"YUV411P", 0, 1 },
  { VIDEO_PALETTE_YUV420P,"YUV420P", 3, 2 },
  { VIDEO_PALETTE_YUV410P,"YUV410P", 0, 1 },
  { -1, NULL, 0, 0 }
};


