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

#include "vgrabbj.h"
#include "v_plist.h"

static struct v_out_type output_list[] = {
  {1, "jpeg"},
  {1, "jpg"},
  {2, "png"},
  {3, "ppm"},
  {0, NULL}
};

static struct v_pos_type position_list[] = {
  {0, "UL"},
  {0, "upperleft"},
  {1, "UR"},
  {1, "upperright"},
  {2, "LL"},
  {2, "lowerleft"},
  {3, "LR"},
  {3, "lowerright"},
  {4, "UC"},
  {4, "uppercenter"},
  {5, "LC"},
  {5, "lowercenter"},
  {-1, NULL}
};

static struct v_size_type size_list[] = {
 {8, "sqcif"},
 {10, "qsif"},
 {11, "qcif"},
 {20, "sif"},
 {22, "cif"},
 {40, "vga"},
 {50, "svga"},
 {64, "xga"},
 {80, "sxga"},
 {100, "uxga"},
 {0, NULL}
};

