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

#if defined(HAVE_LIBTTF) && defined(HAVE_FREETYPE_FREETYPE_H)
#include <freetype/freetype.h>
#endif

#if defined(HAVE_LIBFTP) && defined(HAVE_FTPLIB_H)
#include <ftplib.h>
#endif

/* Defines, defaults */

#define DEFAULT_QUALITY 75
#define MIN_QUALITY 0
#define MAX_QUALITY 100
#define DEFAULT_WIDTH 352
#define DEFAULT_HEIGHT 288
#define RGB_DEFAULT 24
#define MIN_PALETTE 1
#define MAX_PALETTE 17
#define MIN_LOOP 0

#define DEFAULT_VIDEO_DEV "/dev/video"
#define DEFAULT_OUTPUT "/dev/stdout"
#define DEFAULT_OUTFORMAT 1		// 1=jpeg, 2=png
#define DEFAULT_BRIGHTNESS FALSE
#define MAX_ERRORMSG_LENGTH 1024
#define DEFAULT_CONFIG SYSCONF_DIR
#define LOGLEVEL 4
#define MIN_DEBUG 0
#define MAX_DEBUG 7

#if defined(HAVE_LIBTTF) && defined(HAVE_FREETYPE_FREETYPE_H)
#define DEFAULT_FONT "/usr/share/fonts/truetype/Arialn.ttf"
#define DEFAULT_TIMESTAMP "%a, %e. %B %Y - %T"
#define DEFAULT_FONTSIZE 12
#define MIN_FONTSIZE 3
#define MAX_FONTSIZE 100
#define DEFAULT_BORDER 2
#define MIN_BORDER 1
#define MAX_BORDER 255
#define DEFAULT_BLEND 60
#define MIN_BLEND 1
#define MAX_BLEND 100
#define DEFAULT_ALIGN 1
#define MIN_ALIGN 0
#define MAX_ALIGN 5
#endif

#define TS_MAX 128
#define MAX_LINE 1024

/* Structure definitions */
  
struct vconfig {
  long int loop;
  int debug;
  int err_count;
  int quality;
  int outformat;
  int dev;
  char *in;
  char *out;
  char *tmpout;
  boolean windowsize;
  boolean switch_bgr;
  boolean use_ts;
  boolean brightness;
  boolean init_done;
  int inputnorm;
  int channel;
  int forcepal;
#if defined(HAVE_LIBTTF) && defined(HAVE_FREETYPE_FREETYPE_H)
  char *font;
  char *timestamp;
  int font_size;
  int align;
  int border;
  int blend;
  boolean openonce;
#endif
  struct video_window win;
  struct video_picture vpic;
  struct video_capability vcap;
#if defined(HAVE_LIBFTP) && defined(HAVE_FTPLIB_H)
  struct FTP {
    boolean enable;
    boolean keepalive;
    char *remoteHost;
    char *remoteDir;
    char *remoteImageName;
    char *username;
    char *password;
    unsigned int state;
    unsigned int tryharder;
  }ftp;
#endif
};

#if defined(HAVE_LIBTTF) && defined(HAVE_FREETYPE_FREETYPE_H)
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
  int mul;
  int div;
};

/* External functions */

extern char *basename (const char *);

extern struct vconfig *parse_config(struct vconfig *vconf, char *path); 

#if defined(HAVE_LIBFTP) && defined(HAVE_FTPLIB_H)
extern void ftp_upload(struct vconfig *vconf);
#endif

extern void write_image(struct vconfig *vconf, unsigned char *o_buffer);

#if defined(HAVE_LIBTTF) && defined(HAVE_FREETYPE_FREETYPE_H)
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



