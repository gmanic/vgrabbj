/* Simple Video4Linux image grabber. Made for my Philips Vesta Pro
 * 
 * Copyright (C) 2000, 2001 Jens Gecius, Larchmont, USA
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef DEBUGGING
#define DEBUGGING 0
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <getopt.h>
#include <syslog.h>
#include <limits.h>
#include <time.h>
#include <pwd.h>
#include <linux/types.h>
#include <linux/videodev.h>
#include <jpeglib.h>
#include <png.h>
#include <ccvt.h>
#include <signal.h>
#include <mcheck.h>
#include <sys/mman.h>

#ifdef HAVE_LIBTTF
#include <freetype/freetype.h>
#endif

/* Defines, defaults */

#define DEFAULT_QUALITY 75
#define DEFAULT_WIDTH 352
#define DEFAULT_HEIGHT 288
#define RGB_DEFAULT 24
#define DEFAULT_VIDEO_DEV "/dev/video"
#define DEFAULT_OUTPUT "/dev/stdout"
#define DEFAULT_OUTFORMAT 1		// 1=jpeg, 2=png
#define DEFAULT_BRIGHTNESS FALSE
#define MAX_ERRORMSG_LENGTH 1024
#define LOGLEVEL 4


#ifdef HAVE_LIBTTF
#define DEFAULT_FONT "/usr/share/fonts/truetype/Arialn.ttf"
#define DEFAULT_TIMESTAMP "%a, %e. %B %Y - %T"
#define DEFAULT_FONTSIZE 12
#define DEFAULT_BORDER 2
#define DEFAULT_BLEND 60
#define DEFAULT_ALIGN 1
#endif

#define TS_MAX 128

/* Structure definitions */
  
struct vconfig {
  unsigned long int loop;
  int debug;
  int err_count;
  unsigned int quality;
  int outformat;
  int dev;
  char *in;
  char *out;
  boolean windowsize;
  boolean switch_bgr;
  boolean use_ts;
  boolean brightness;
  boolean init_done;
  int inputnorm;
  int channel;
  int forcepal;
#ifdef HAVE_LIBTTF
  char *font;
  char *timestamp;
  int font_size;
  int align;
  int border;
  int blend;
  boolean copy;
#endif
  struct video_window win;
  struct video_picture vpic;
  struct video_capability vcap;
};

#ifdef HAVE_LIBTTF
struct ttneed {
  TT_Engine engine;
  TT_Face face;
  TT_Face_Properties *properties;
  TT_Instance instance;
  boolean use;
};
#endif

struct palette_list {
  int num;
  char *name;
};

/* External functions */

extern char *basename (const char *);

#ifdef HAVE_LIBTTF
extern void      Face_Done   (TT_Instance inst, TT_Face face);
extern int       Face_Open   (char *file, TT_Engine engine, TT_Face *face,
			      TT_Face_Properties *prop, TT_Instance *inst,
			      int ptsize);
extern TT_Glyph *Glyphs_Load (TT_Face face, TT_Face_Properties *prop,
			      TT_Instance inst, char *str, int len);
extern void      Glyphs_Done (TT_Glyph *gl);
extern void      Raster_Init (TT_Face face, TT_Face_Properties *prop,
			      TT_Instance inst, char *str, int len, 
			      int border, TT_Glyph *gl, TT_Raster_Map *bit);
extern void      Raster_Done (TT_Raster_Map *bit);
extern void      Raster_Small_Init  (TT_Raster_Map *map, TT_Instance *inst);
extern unsigned char *Render_String (TT_Glyph *gl, char *str, int len,
				     TT_Raster_Map *bit, TT_Raster_Map *sbit,
				     int border);

#endif
