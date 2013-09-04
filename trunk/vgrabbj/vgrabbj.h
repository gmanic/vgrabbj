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

#ifdef HAVE_CONFIG_H
#include <config.h>
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
#include <jpeglib.h>
#include <png.h>
#include <ccvt.h>
#include <signal.h>
#include <mcheck.h>
#include <sys/mman.h>

#include <libv4l1.h>
#include <libv4l1-videodev.h>

#if defined(HAVE_LIBTTF)
#if defined(HAVE_FREETYPE_FREETYPE_H)
#define LIBTTF 1
#define TTF_H_LOC <freetype/freetype.h>
#else
#if defined(HAVE_FREETYPE1_FREETYPE_H)
#define LIBTTF 1
#define TTF_H_LOC <freetype1/freetype.h>
#else
#undef LIBTTF
#endif
#endif
#else
#undef LIBTTF
#endif

#ifdef LIBTTF
#include TTF_H_LOC
#endif

#if defined(HAVE_LIBFTP) && defined(HAVE_FTPLIB_H)
#define LIBFTP 1
#define STATE_UNINITIALIZED 0
#define STATE_CONNECT 1
#define STATE_LOGIN 2
#define STATE_CHDIR 3
#define STATE_PUT 4
#define STATE_RENAME 5
#define STATE_FINISH 6
#include <ftplib.h>
#else
#undef LIBFTP
#endif

#ifndef DEBUGGING
#define DEBUGGING 0
#endif

/* Defines, defaults */

//#define boolean int
//#define TRUE 1
//#define FALSE 0

#define __u32 int

#define DEFAULT_QUALITY 75
#define MIN_QUALITY 0
#define MAX_QUALITY 100
#define DEFAULT_WIDTH 352
#define DEFAULT_HEIGHT 288
#define MIN_SIZE 0
#define MAX_SIZE 65535
#define RGB_DEFAULT 24
#define MIN_PALETTE 1
#define MAX_PALETTE 16
#define MIN_DISCARD 0
#define MAX_DISCARD 255
#define MIN_BOOL 0
#define MAX_BOOL 1
#define MIN_LOOP 0
#define MIN_ARCHIVE 0
#define MAX_ARCHIVE 65535

#define DEFAULT_VIDEO_DEV "/dev/video"
#define DEFAULT_OUTPUT "/dev/stdout"
#define DEFAULT_OUTFORMAT 1		// 1=jpeg, 2=png, 3=ppm
#define MIN_FORMAT 1
#define MAX_FORMAT 3
#define DEFAULT_BRIGHTNESS FALSE
#define MAX_ERRORMSG_LENGTH 1024
#define DEFAULT_CONFIG SYSCONF_DIR
#define LOGLEVEL 4
#define MIN_DEBUG 0
#define MAX_DEBUG 7

#ifdef LIBTTF
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

#ifdef LIBTTF
struct ttneed {
  TT_Engine engine;
  TT_Face face;
  TT_Face_Properties *properties;
  TT_Instance instance;
  boolean use;
};
#endif

struct s_arch {
  char *filename;
  struct s_arch *next;
};

struct vconfig {
  long int loop;
  int debug;
  int err_count;
  int quality;
  int outformat;
  int dev;
  int discard;
  int archiveeach;
  int archivemax;
  int archivecount;
  char *in;
  char *out;
  char *tmpout;
  char *conf_file;
  char *buffer;
  char *o_buffer;
  char *archive;
  struct s_arch *arch;
  boolean usemmap;
  boolean usetmpout;
  boolean windowsize;
  boolean switch_bgr;
  boolean use_ts;
  boolean autobrightness;
  boolean init_done;
  boolean swaprl;
  boolean swaptb;
  boolean nousemmap;
  int inputnorm;
  int channel;
  int forcepal;
#ifdef LIBTTF
  char *font;
  char *timestamp;
  int font_size;
  int align;
  int border;
  int blend;
  struct ttneed *ttinit;
#endif
  boolean openonce;
  struct video_window win;
  struct video_picture vpic;
  struct video_capability vcap;
  struct video_mmap vmap;
  struct video_mbuf vbuf;
  int hue;
  int brightness;
  int contrast;
  int colour;
  int whiteness;
  char *map;
  boolean nofork;
#ifdef LIBFTP
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
    boolean passive;
  }ftp;
#endif
};

struct palette_list {
  int num;
  char *name;
  int mul;
  int div;
};

struct v_options {
  const char *name;
  const char *short_name;
  int has_arg;
  void *var;
  int var_type;
  int min_value;
  int max_value;
  int max_length;
  const char *standard;
  struct v_options *next;
};

struct v_out_type {
  int type;
  const char *name;
};

struct v_pos_type {
  int type;
  const char *name;
};

struct v_size_type {
  int type;
  const char *name;
};

enum { opt_void, opt_int, opt_longint, opt_char, opt_bool, opt_format, opt_size, opt_charptr,
       opt_position, opt_int_s, opt_conf, opt_setting, opt_help, opt_version };
enum { none, req, opt };

/* External functions */

extern char           *basename (const char *);
extern struct vconfig *v_init(struct vconfig *vconf, int argc, char *argv[]);
extern struct vconfig *v_reinit(struct vconfig *vconf);
extern void            show_capabilities(char *in, char *pname);
extern void            ftp_upload(struct vconfig *vconf);
extern void            write_image(struct vconfig *vconf);
extern void            v_error(struct vconfig *vconf, int msg, const char *fmt, ...);
extern int             img_size(struct vconfig *vconf, int palette);
extern void           *free_ptr(void *buf);
extern int             get_height(char *value);
extern int             get_width(char *value);
extern int             decode_size(char *value);
extern int             get_position(char *value);
extern int             get_format(char *value);
extern int             get_bool(char *value);
extern char           *get_str(char *value, char *var);
extern long int        get_int(char *value);
extern int             daemonize(struct vconfig *vconf, char *progname);
extern void            sighup();
extern void            sigterm();
extern int             brightness_adj(struct vconfig *vconf, int *brightness);
extern unsigned char  *conv_rgb32_rgb24(struct vconfig *vconf);
extern unsigned char  *switch_color(struct vconfig *vconf);
extern void            init_mmap(struct vconfig *vconf);
extern void            free_mmap(struct vconfig *vconf);
extern void            open_device(struct vconfig *vconf);
extern int             set_picture_parms(struct vconfig *vconf);
extern void            close_device(struct vconfig *vconf);
extern void            cleanup(struct vconfig *vconf);
extern int             signal_terminate;
extern struct v_options l_opt[];
extern char           *strip_white(char *value);
extern long int        check_minmax(struct vconfig *vconf, char *value, long int tmp, int n,
				    struct v_options l_opt);
extern void            v_update_ptr(struct vconfig *vconf);
extern char           *check_maxlen(struct vconfig *vconf, char *value, struct v_options l_opt, int n);
extern FILE           *open_outfile(char *filename);
extern char           *timestring(char *format);
extern struct vconfig *parse_config(struct vconfig *vconf);
extern unsigned char  *swap_left_right(char *buffer, int width, int height);
extern unsigned char  *swap_top_bottom(char *buffer, int width, int height);

#ifdef LIBTTF
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
extern char      *inserttext (struct ttneed *ttinit, unsigned char *buffer,
			      struct vconfig *vconf);
extern struct ttneed *OpenFace(struct vconfig *vconf);
#endif
