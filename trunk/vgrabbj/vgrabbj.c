/* Simple Video4Linux image grabber. Made for my Philips Vesta Pro
 * 
 * Copyright (C) 2000, 2001 geci, Larchmont, USA
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
#include <linux/types.h>
#include <linux/videodev.h>
#include <jpeglib.h>
#include <png.h>
#include <freetype/freetype.h>
#include "font.h"

#define QUAL_DEFAULT 75
#define WIDTH_DEFAULT 352
#define HEIGHT_DEFAULT 288
#define RGB_DEFAULT 24
#define VIDEO_DEV "/dev/video"
#define OUT_DEFAULT "/dev/stdout"
#define OUTFORMAT_DEFAULT 1		// 1=jpeg, 2=png
#define DEFAULT_FONT "/usr/share/fonts/truetype/Arialn.ttf"
#define DEFAULT_TIMESTAMP "%a, %e. %B %Y - %T"
#define DEFAULT_FONTSIZE 12
#define DEFAULT_BORDER 2
#define DEFAULT_BLEND 60
#define DEFAULT_ALIGN 1
#define TS_MAX 128
#define DEBUG 0

/* Global Variables  */

int debug = DEBUG;

/* Adjustment of brightness of picture  */

int get_brightness_adj(unsigned char *image, long size, int *brightness) 
{
  long i, tot = 0;
  for (i=0;i<size*3;i++)
    tot += image[i];
  *brightness = (128 - tot/(size*3))/3;
  if (debug)
    fprintf(stderr,"Brightness adjusted, runs: %d\n",i);
  return !((tot/(size*3)) >= 126 && (tot/(size*3)) <= 130);
}

/* Usage information  */

void usage (char *pname) 
{
  fprintf(stderr,
	  "%s, Version %s\n"
	  "Usage: %s [options]\n"
	  " -h                This screen\n"
	  " -l <seconds>      Daemonize & sleep <seconds> (min. 1!) between images\n"
	  " -L <microseconds> Daemonize & sleep <microseconds> between images\n"
	  " -b                Disable vgrabbj's brightness adjustment\n"
	  " -q <quality>      Quality setting (1-100, default: %d), JPEG only\n"
	  " -i <sqcif|qsif|qcif|sif|cif|vga>\n"
	  "                   Sets the imagesize of input device to sqcif=128x96,\n"
	  "                   qsif=160x120, qcif=176x144, sif=320x240, cif=352x288, or\n"
	  "                   vga=640x480 (default: %dx%d)\n"
	  " -o <jpeg|png>     Output format (default: jpeg)\n"
	  " -f <filename>     Write to <filename> (default: %s)\n"
	  " -d <device>       Read from <device> as input (default: %s)\n"
	  " -w                Disable setting of image-size, necessary for certain cams\n"
	  "                   (e.g. IBM USB-Cam, QuickCam)\n"
	  " -s <device>       See capabilities of <device>\n"
	  " -t <font-file>    Enable timestamp of image, needs full path to the font file\n"
	  "                   (default: %s)\n"
	  " -T <font-size>    Font-size (min. 1, max. 100, default: %d)\n"
	  " -p <format-str>   Definable timestamp format (see man strftime)\n"
	  "                   (default: \"%s\")\n"
	  "                   *MUST* be within \" and \" !\n"
	  " -e                enable timestamp with defaults (default: disabled)\n"
	  "                   if -t or -p is given, timestamp is already enabled\n"
	  " -a <0|1|2|3|4|5>  Alignment of timestamp: 0=upper left, 1=upper right,\n"
	  "                   2=lower left, 3=lower right, 4=upper center, 5=lower center\n"
	  "                   you still have to enable the timestamp (default: %d) \n"
	  " -m <blendvalue>   Blending of timestamp on original image (1-100, default: %d)\n"
	  "                   1 = most original, 100 = no original image \"behind\" timestamp\n"
	  " -B <pixel>        Border of timestamp to be blended around string in pixel\n"
	  "                   (1-255, default: %d)\n"
	  " -D                Toggle debug output (default: %s)\n"
	  "\n"
	  "Example: %s -l 5 -f /usr/local/image.jpg\n"
	  "         Would write a single jpeg-image to image.jpg about every five seconds\n"
	  "\n"
	  "Currently, the video stream has to be rgb24. Sorry folks.\n",
	  basename(pname), VERSION, basename(pname), 
	  QUAL_DEFAULT, WIDTH_DEFAULT, HEIGHT_DEFAULT,
	  OUT_DEFAULT, VIDEO_DEV, DEFAULT_FONT, 
	  DEFAULT_FONTSIZE, DEFAULT_TIMESTAMP, DEFAULT_ALIGN,
	  DEFAULT_BLEND, DEFAULT_BORDER, DEBUG, basename(pname));
  exit (1);
}

/* Get information from v4l device and show them  */

void show_capabilities(char *in, char *pname) 
{
  struct video_capability cap;
  struct video_window win;
  struct video_picture pic;
  int dev;
  struct jpeg_compress_struct cjpeg;
  struct jpeg_error_mgr jerr;
  
  dev = open(in, O_RDONLY);
  if (dev < 0) 
    {
      fprintf(stderr, "Can't open device %s\n", in);
      exit(1);
    }
  if (ioctl(dev, VIDIOCGCAP, &cap) < 0) 
    {
      perror("Get capabilities");
      close(dev);
      exit(1);
    }
  if (ioctl(dev, VIDIOCGPICT, &pic) < 0) 
    {
      perror("Get picture properties");
      close(dev);
      exit(1);
    }
  if (ioctl(dev, VIDIOCGWIN, &win) < 0) 
    {
      perror("Get overlay values");
      close(dev);
      exit(1);
    }
  fprintf(stderr,"%s, Version %s\n"
	  "Videodevice name: %s (%s)\n"
	  "Capabilities\n"
	  "Type     : %d\tValues can be looked up at videodev.h\n"
	  "Channels : %d\n"
	  "Audio    : %d\n"
	  "MaxWidth : %d\n"
	  "MaxHeight: %d\n"
	  "MinWidth : %d\n"
	  "MinHeigth: %d\n"
	  "\nCurrent Settings:\n"
	  "Brightness: %d\n"
	  "Hue       : %d\n"
	  "Color     : %d\n"
	  "Contrast  : %d\n"
	  "Whiteness : %d\n"
	  "Depth     : %d\n"
	  "Palette   : %d\tValues can be looked up at videodev.h\n"
	  "Width     : %d\n"
	  "Height    : %d\n"
	  "Chromakey : %d\n",
	  basename(pname), VERSION, in, cap.name, cap.type, cap.channels, cap.audios,
	  cap.maxwidth, cap.maxheight, cap.minwidth, cap.minheight,
	  pic.brightness, pic.hue, pic.colour, pic.contrast,
	  pic.whiteness, pic.depth, pic.palette, win.width, win.height,
	  win.chromakey);
  close(dev);
  exit(0);
}

/* Write image as jpeg to FILE  */

int write_jpeg(char *buffer, FILE *x, int quality, int width, int height) 
{
  char *line;
  int n, y=0, i, line_width;
  
  struct jpeg_compress_struct cjpeg;
  struct jpeg_error_mgr jerr;
  JSAMPROW row_ptr[1];
  
  line=malloc(width * 3);
  if (!line) 
    {
      perror("OUT OF MEMORY, Exiting...");
      syslog(LOG_ALERT, "OUT OF MEMORY, Exiting...");
      exit(1);
    }
  cjpeg.err = jpeg_std_error(&jerr);
  jpeg_create_compress (&cjpeg);
  cjpeg.image_width = width;
  cjpeg.image_height= height;
  cjpeg.input_components = 3;
  cjpeg.in_color_space = JCS_RGB;
  
  jpeg_set_defaults (&cjpeg);
  jpeg_set_quality (&cjpeg, quality, TRUE);
  cjpeg.dct_method = JDCT_FASTEST;
  
  jpeg_stdio_dest (&cjpeg, x);
  jpeg_start_compress (&cjpeg, TRUE);
  row_ptr[0]=line;
  line_width=width * 3;
  n=0;
  
  for (y = 0; y < height; y++) 
    {
      for (i = 0; i< line_width; i+=3) 
	{
	  line[i]   = buffer[n+2];
	  line[i+1] = buffer[n+1];
	  line[i+2] = buffer[n];
	  n+=3;
	}
      jpeg_write_scanlines (&cjpeg, row_ptr, 1);
    }
  jpeg_finish_compress (&cjpeg);
  jpeg_destroy_compress (&cjpeg);
  free (line);
  if (debug) fprintf(stderr,"Wrote jpg.\n");
  return(0);
}

/* Write image as png to FILE  */

int write_png(char *image, FILE *x, int width, int height) 
{
  register int y;
  register char *p;
  png_infop info_ptr;
  png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
						 NULL, NULL, NULL);
  if (!png_ptr)
    return(1);
  info_ptr = png_create_info_struct (png_ptr);
  if (!info_ptr)
    return(1);
  
  png_init_io (png_ptr, x);
  png_set_IHDR (png_ptr, info_ptr, width, height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_bgr (png_ptr);
  png_write_info (png_ptr, info_ptr);
  p = image;
  width *= 3;
  for (y = 0; y < height; y++) 
    {
      png_write_row (png_ptr, p);
      p += width;
    }
  png_write_end (png_ptr, info_ptr);
  png_destroy_write_struct (&png_ptr, &info_ptr);
  if (debug) fprintf(stderr,"Wrote png.");
  return(0);
}

/* Open Face via routine found in font.h  */

int OpenFace (char *font, TT_Engine engine, TT_Face *face, 
	      TT_Face_Properties *properties, 
	      TT_Instance *instance, int font_size, int loop) 
{
  int rc,i,j;
  rc=TT_FreeType_Version(&i, &j);
  if (debug) fprintf(stderr,"FreeType, Version %d.%d.\n",i,j);
  rc=TT_Init_FreeType (&engine);
  if (rc) 
    {
      font = NULL;
    }
  rc=Face_Open (font, engine, face, properties, instance, font_size);
  if (rc) 
    {
      if (loop) 
	{
	  syslog(LOG_WARNING, "Font not found: %s, timestamp disabled\n", font);
	}
      else 
	{
	  fprintf(stderr,"Font not found: %s, timestamp disabled\n",font);
	}
      TT_Done_FreeType (engine);
      font = NULL;
      if (debug) fprintf(stderr,"Font-file error.\n");
      return(FALSE);
    }
  else 
    {
      if (debug) fprintf(stderr,"Font-Engine initialized.\n");
      return(TRUE);
    }
}

/* Manipulate image to show timestamp string  */

int inserttext(unsigned char *buffer, char *font, TT_Engine engine, TT_Face face, 
	       TT_Face_Properties *properties, TT_Instance instance, int font_size, 
	       char *timestamp, int align, int width, int height, int border, int blend)
{
  time_t t;
  struct tm *tm;
  char ts_buff[TS_MAX+1];
  int ts_len;
  TT_Glyph *glyphs = NULL;
  TT_Raster_Map bit;
  TT_Raster_Map sbit;
  
  if (debug) 
    fprintf(stderr,"Getting all values for the timestamp.\n");
  time (&t);
  tm = localtime (&t);
  ts_buff[TS_MAX] = '\0';
  strftime (ts_buff, TS_MAX, timestamp, tm);
  ts_len = strlen (ts_buff);
  if (debug) 
    fprintf(stderr,"Timestring: %s, length: %d\n",ts_buff,ts_len);
  glyphs = Glyphs_Load (face, properties, instance, ts_buff, ts_len);
  if (debug) 
    fprintf(stderr,"Glyphs loaded\n");
  Raster_Init(face, properties, instance, ts_buff, ts_len, border, glyphs, &bit);
  if (debug) 
    fprintf(stderr,"Returned from Raster_Init\n");
  Raster_Small_Init (&sbit, &instance);
  if (debug) 
    fprintf(stderr,"Returned from Raster_Small_Init\n");
  Render_String (glyphs, ts_buff, ts_len, &bit, &sbit, border);
  if (debug) 
    fprintf(stderr,"Returned from Render_String\n");
  if (bit.bitmap) 
    {
      int x, y, psize, i, x_off, y_off;
      unsigned char *p;
      if (debug) 
	fprintf(stderr,"Now performing calculation of position...\n");
      if (bit.rows>height) 
	bit.rows=height;
      if (bit.width>width) 
	bit.width=width;
      psize = 3;
      switch (align) 
	{
	case 1:
	  x_off = (width - bit.width) * psize;
	  y_off = 0;
	  break;
	case 2:
	  x_off = 0;
	  y_off = height - bit.rows;
	  break;
	case 3:
	  x_off = (width - bit.width) * psize;
	  y_off = height - bit.rows;
	  break;
	case 4:
	  x_off = (width/2 - bit.width/2) * psize;
	  y_off = 0;
	  break;
	case 5:
	  x_off = (width/2 - bit.width/2) * psize;
	  y_off = height - bit.rows;
	  break;
	default:
	  x_off = y_off = 0;
	  break;
	}
      if (debug) 
	fprintf(stderr,"Wow, we did it... Now we change the image with the string.\n");
      for (y = 0; y < bit.rows; y++) 
	{
	  p = buffer + (y + y_off) * (width * psize) + x_off;
	  for (x = 0; x < bit.width; x++) 
	    {
	      switch (((unsigned char *)bit.bitmap)
		      [((bit.rows-y-1)*bit.cols)+x]) 
		{
		case 0:
		  for (i = 0; i < psize; i++) 
		    {
		      *p = (255 * blend + *p * (100 - blend))/100;
		      p++;
		    }
		  break;
		case 1:
		  for (i = 0; i < psize; i++) 
		    {
		      *p = (220 * blend + *p * (100 - blend))/100;
		      p++;
		    }
		  break;
		case 2:
		  for (i = 0; i < psize; i++) 
		    {
		      *p = (162 * blend + *p * (100 - blend))/100;
		      p++;
		    }
		  break;
		case 3:
		  for (i = 0; i < psize; i++) 
		    {
		      *p = (64 * blend + *p * (100 - blend))/100;
		      p++;
		    }
		  break;
		default:
		  for (i = 0; i < psize; i++) 
		    {
		      *p = (0 * blend + *p * (100 - blend))/100;
		      p++;
		    }
		  break;
		}
	    }
	}
    }
  if (debug) 
    fprintf(stderr,"Image manipulated, now closing...\n");
  Raster_Done (&sbit);
  if (debug) 
    fprintf(stderr,"Returned from Raster_Done(sbit)\n");
  Raster_Done (&bit);
  if (debug) 
    fprintf(stderr,"Returned from Raster_Done(bit)\n");
  Glyphs_Done (glyphs);
  if (debug) 
    fprintf(stderr,"Returned from Glyphs_Done\n");
  glyphs = NULL;
}

/* Main loop  */


int main(int argc, char *argv[]) 
{
  int y, dev, f, n, rc;
  int bpp = RGB_DEFAULT;
  boolean in_loop=FALSE;
  int quality= QUAL_DEFAULT;
  int err_count2;
  int loop=0;
  char tbuff[MAXPATHLEN];
  char *in = VIDEO_DEV;
  int outformat = OUTFORMAT_DEFAULT;
  char *out = OUT_DEFAULT;
  char *line;
  unsigned char *buffer;
  unsigned int i, src_depth;
  
  FILE *x;
  
  struct video_capability cap;
  struct video_window win, twin;
  struct video_picture vpic;
  boolean enable_timestamp = FALSE, use_ts=FALSE;
  boolean brightness=TRUE;
  boolean windowsize=TRUE;
  char *font = DEFAULT_FONT;
  char *timestamp = DEFAULT_TIMESTAMP;
  int font_size = DEFAULT_FONTSIZE;
  int align = DEFAULT_ALIGN;
  int border = DEFAULT_BORDER;
  int blend = DEFAULT_BLEND;
  TT_Engine engine;
  TT_Face face;
  TT_Face_Properties properties;
  TT_Instance instance;
  win.width=WIDTH_DEFAULT;
  win.height=HEIGHT_DEFAULT;
  
  if (debug) fprintf(stderr,"Starting up...\n");
  while ((n = getopt (argc, argv, "L:l:f:q:hd:s:o:t:T:p:ebi:a:DB:m:w"))!=EOF) 
    {
      switch (n) 
	{
	case 'l':
	  rc=sscanf (optarg, "%d", &loop);
	  if ( (rc!=1) || (loop<1) ) 
	    {
	      perror("Wrong sleeptime");
	      usage(argv[0]);
	    }
	  loop=loop*1000000;
	  break;
	case 'L':
	  rc=sscanf (optarg, "%d", &loop);
	  if ( (rc!=1) || (loop<1) ) 
	    {
	      perror("Wrong sleeptime");
	      usage(argv[0]);
	    }
	  break;
	case 'f':
	  if (!(x=fopen(out=optarg,"w+"))) 
	    {
	      perror("Couldn't acces output file");
	      usage(argv[0]);
	    }
	  fclose(x);
	  break;
	case 'q':
	  rc=sscanf (optarg, "%d", &quality);
	  if ( (rc!=1) || (quality<0) || (quality>100) ) 
	    {
	      perror("Quality wrong");
	      usage(argv[0]);
	    }
	  break;
	case 'o':
	  if ( !strcasecmp(optarg,"jpeg") || !strcasecmp(optarg,"jpg") )
	    outformat=1;
	  else
	    if ( !strcasecmp(optarg,"png") ) outformat=2;
	    else {
	      perror("Wrong format specified");
	      usage(argv[0]);
	    }
	  break;
	case 'i':
	  if ( !strcasecmp(optarg,"sqcif") ) 
	    {
	      win.width=128;
	      win.height=96;
	    }
	  else 
	    {
	      if ( !strcasecmp(optarg,"qsif") ) 
		{
		  win.width=160;
		  win.height=120;
		}
	      else 
		{
		  if ( !strcasecmp(optarg,"qcif") ) 
		    {
		      win.width=176;
		      win.height=144;
		    }
		  else 
		    {
		      if ( !strcasecmp(optarg,"sif") ) 
			{
			  win.width=320;
			  win.height=240;
			}
		      else 
			{
			  if ( !strcasecmp(optarg,"cif") ) 
			    {
			      win.width=352;
			      win.height=288;
			    }
			  else 
			    {
			      if ( !strcasecmp(optarg,"vga") ) 
				{
				  win.width=640;
				  win.height=480;
				}
			      else 
				{
				  perror("Wrong imagesize specified");
				  usage(argv[0]);
				}
			    }
			}
		    }
		}
	    }
	  break;
	case 'd':
	  if ((dev=open(in=optarg,O_RDONLY))<0) 
	    {
	      perror("Device not accessible");
	      usage(argv[0]);
	    }
	  close(dev);
	  break;
	case 't':
	  if (!(x=fopen(font=optarg, "r"))) 
	    {
	      perror("Font-file not found");
	      usage(argv[0]);
	    }
	  fclose(x);
	  use_ts=TRUE;
	  font=optarg;
	  break;
	case 'T':
	  rc=sscanf(optarg, "%d", &font_size);
	  if ((rc!=1)||(font_size<1)||font_size>100) 
	    {
	      perror("Wrong font-size (min. 1, max 100)");
	      usage(argv[0]);
	    }
	  break;
	case 'p':
	  if (!(x=fopen(font, "r"))) 
	    {
	      perror("Font-file not found");
	      usage(argv[0]);
	    }
	  fclose(x);
	  timestamp=optarg;
	  use_ts=TRUE;
	  break;
	case 'e':
	  if (!(x=fopen(font, "r"))) 
	    {
	      perror("Font-file not found");
	      usage(argv[0]);
	    }
	  fclose(x);
	  use_ts=TRUE;
	  break;
	case 'a':
	  rc=sscanf (optarg, "%d", &align);
	  if ( (rc!=1) || (align<0) || (align>5) ) 
	    {
	      perror("Wrong Timestamp alignment");
	      usage(argv[0]);
	    }
	  break;
	case 'm':
	  rc=sscanf (optarg, "%d", &blend);
	  if ( (rc!=1) || (blend > 100) || (blend <1) ) 
	    {
	      perror("Wrong blend value");
	      usage(argv[0]);
	    }
	  break;
	case 'B':
	  rc=sscanf (optarg, "%d", &border);
	  if ( (rc!=1) || (border > 255) || (border <1) ) 
	    {
	      perror("Wrong border value");
	      usage(argv[0]);
	    }
	  break;
	case 'b':
	  brightness=FALSE;
	  break;
	case 's':
	  if ((dev=open(in=optarg,O_RDONLY))<0) 
	    {
	      perror("Device not accessible");
	      usage(argv[0]);
	    }
	  close(dev);
	  show_capabilities(in, argv[0]);
	  break;
	case 'D':
	  debug=!debug;
	  break;
	case 'w':
	  windowsize=FALSE;
	  break;
	default:
	  usage (argv[0]);
	  break;
	}
    }
  if (debug) 
    fprintf(stderr,"Read all arguments, now reading image...\n");
  if (loop) 
    {
      if (debug) fprintf(stderr,"Forking for daemon mode...\n");
      sleep(5);
      openlog("vgrabbj", LOG_PID, LOG_DAEMON);
      switch(fork()) 
	{
	case 0: /* Child  */
	  closelog();
	  setsid;
	  if (debug) 
	    perror("I'm the child process and are going to read images...");
	  break;
	case -1: /* Error  */
	  perror("Can't fork, exiting...");
	  closelog();
	  exit(1);
	default: /* Parent  */
	  closelog();
	  if (debug) 
	    perror("I'm the parent and exiting now (child takes care of the rest).");
	  exit(0);
	}
      openlog("vgrabbj", LOG_PID, LOG_DAEMON);
      syslog(LOG_INFO, "%s started, reading from %s\n", basename(argv[0]), in);
    }
  else
    fprintf(stderr, "Reading image from %s\n", in);
  
  if (use_ts) 
    enable_timestamp=OpenFace(font, engine, &face, &properties, &instance, font_size, loop);

  do 
    {
      dev = open(in, O_RDONLY);
      if (dev < 0) 
	{
	  if (loop) 
	    {
	      syslog(LOG_ERR, "Problem opening %s\n", in);
	      usleep (250000);
	    }
	  else 
	    {
	      fprintf(stderr, "Can't open device %s\n", in);
	      if (font && timestamp && enable_timestamp) 
		{
		  Face_Done (instance, face);
		}
	      exit(1);
	    }
	}
      else 
	{
	  in_loop=TRUE;
	  if (ioctl(dev, VIDIOCGCAP, &cap) < 0) 
	    {
	      if (loop) {
		syslog(LOG_WARNING, "Problem getting video capabilities\n");
		usleep(250000);
	      }
	      else {
		perror("Problem getting video capabilities");
		close(dev);
		if (font && timestamp && enable_timestamp) 
		  {
		    Face_Done (instance, face);
		  }
		exit(1);
	      }
	    }
	  else {
	    if ( (cap.maxwidth < win.width) || (cap.minwidth > win.width) 
		 || (cap.maxheight < win.height) 
		 || (cap.minheight > win.height) ) 
	      {
		if (loop) 
		  syslog(LOG_ALERT, "Device doesn't support width/height. Exiting...\n");
		else 
		  perror("Device doesn't support width/height");
		close(dev);
		if (font && timestamp && enable_timestamp) 
		  Face_Done (instance, face);
		if (debug) 
		  fprintf(stderr,"Device-error - doesn't support image size...\n");
		exit(1);
	      }
	    if (ioctl(dev, VIDIOCGWIN, &twin))
	      {
		if (loop)
		  {
		    syslog(LOG_WARNING, "Problem getting window information\n");
		    usleep (250000);
		  }
		else
		  {
		    perror("Problem getting window information");
		    close(dev);
		    if (font && timestamp && enable_timestamp)
		      Face_Done(instance, face);
		    exit(1);
		  }
	      }
	    else
	      {
		win.flags=twin.flags;
		win.x=twin.x;
		win.y=twin.y;
		win.chromakey=twin.chromakey;
		if (ioctl(dev, VIDIOCSWIN, &win) && windowsize) 
		  {
		    if (loop) 
		      {
			syslog(LOG_WARNING, "Problem setting window size\n");
			usleep (250000);
		      }
		    else 
		      {
			perror("Problem setting window size");
			close(dev);
			if (font && timestamp && enable_timestamp)
			  Face_Done (instance, face);
			exit(1);
		      }
		  }
		else 
		  {
		    if (ioctl(dev, VIDIOCGWIN, &win) <0) 
		      {
			if (loop) 
			  {
			    syslog(LOG_WARNING, "Problem getting window size\n");
			    usleep (250000);
			  }
			else 
			  {
			    perror("Problem getting window size");
			    close(dev);
			    if (font && timestamp && enable_timestamp)
			      Face_Done (instance, face);
			    exit(1);
			  }
		      }
		    else 
		      {
			//			vpic.depth=24;
			//			vpic.palette=VIDEO_PALETTE_RGB24;
	    
			if (ioctl(dev, VIDIOCGPICT, &vpic) < 0) 
			  {
			    if (loop) 
			      {
				syslog(LOG_WARNING, "Problem getting picture properties %m\n");
				usleep(250000);
			      }
			    else 
			      {
				perror("Problem getting picture properties");
				close(dev);
				if (font && timestamp && enable_timestamp)
				  Face_Done (instance, face);
				exit(1);
			      }
			  }
			else 
			  {
			    // HERE we got to actually set the parameters at the device to RGB24
			    // and depth 24, otherwise the device would still be set to whatever it
			    // was before the above call of properties and never obeyed the need of
			    // vgrabbj.

			    vpic.depth=24;
			    vpic.palette=VIDEO_PALETTE_RGB24;
			    if (ioctl(dev,VIDIOCSPICT, &vpic) < 0)
			      {
				if (loop) 
				  {
				    syslog(LOG_WARNING, "Problem setting picture properties (does not support RGB24) %m\n");
				    usleep(250000);
				  }
				else 
				  {
				    perror("Problem setting picture properties (does not support RGB24)");
				    close(dev);
				    if (font && timestamp && enable_timestamp)
				      Face_Done (instance, face);
				    exit(1);
				  }
			      }
			    else
			      {
				if (debug) fprintf(stderr,"Allocating memory for image.\n");
				buffer = malloc(win.width * win.height * bpp);
				if (!buffer) 
				  {
				    if (loop)
				      syslog(LOG_ALERT, "Out of Memory! Exiting...\n");
				    else
				      perror("Out of Memory!");
				    close(dev);
				    if (font && timestamp && enable_timestamp) 
				      Face_Done (instance, face);
				    exit(9);
				  }
				err_count2=0;
				if (debug) 
				  fprintf(stderr,"See, if I shall adjust the brightness.\n");
				if (brightness) 
				  {
				    do 
				      {
					int newbright;
					read(dev, buffer, win.width * win.height * bpp);
					f = get_brightness_adj(buffer, win.width * win.height, &newbright);
					if (f) 
					  {
					    vpic.brightness += (newbright << 8);
					    if(ioctl(dev, VIDIOCSPICT, &vpic)==-1) 
					      {
						if (loop)
						  syslog(LOG_DEBUG, "Problem setting brightness %m\n");
						else
						  perror("Problem setting brightness");
						break;
					      }
					    err_count2++;
					    if (err_count2>100) 
					      {
						if (loop)
						  syslog(LOG_WARNING,"Brightness not optimal\n");
						else
						  perror("Brightness not optimal");
						break;
					      }
					  }
				      } while (f);
				    if (debug) 
				      fprintf(stderr,"Finally, we have an image...\n");
				  }
				else 
				  {
				    read(dev, buffer, win.width * win.height * bpp);
				    if (debug) 
				      fprintf(stderr,"Image read without brightness-adjustment.\n");
				  }
				close(dev);
				if (debug) 
				  fprintf(stderr,"Video-Device closed.\n");
				if (font && timestamp && enable_timestamp) 
				  {
				    inserttext(buffer, font, engine, face, &properties, 
					       instance, font_size, timestamp, align, 
					       win.width, win.height, border, blend);
				  }
				do 
				  {
				    if (debug) 
				      fprintf(stderr,"Opening output-device.\n");
				    x = fopen(out, "w+");
				    if (!x) 
				      {
					if (loop) 
					  {
					    syslog(LOG_WARNING, "Could not open outputfile: %s\n", out);
					    usleep(250000);
					  }
					else 
					  {
					    perror("Could not open Outputfile");
					    close(dev);
					    if (font && timestamp && enable_timestamp)
					      Face_Done (instance, face);
					    exit(1);
					  }
				      }
				  } while (!x);
				switch (outformat) 
				  {
				  case 1:
				    while (write_jpeg(buffer, x, quality, win.width, win.height)) 
				      {
					syslog(LOG_WARNING, "Could not write to outputfile: %s\n", out);
					usleep(25000);
				      }
				    break;
				  case 2:
				    write_png(buffer, x, win.width, win.height);
				    break;
				  default:		/* should never happen  */
				    usage(argv[0]);
				    break;
				  }
				fclose(x);
				if (debug) 
				  fprintf(stderr,"Output-device closed.\n");
				free (buffer);
				if (debug) 
				  fprintf(stderr,"Image-buffer freed. Now sleeping if daemon.\n");
				usleep(loop);
			      }
			  }
		      }
		  }
	      }
	  }
	}
    } while (loop);
  if (font && timestamp && enable_timestamp) 
    {
      Face_Done (instance, face);
    }
  if (debug) 
    fprintf(stderr,"Returned from Face_Done.\n");
  //  TT_Done_FreeType (engine);
  if (debug) 
    fprintf(stderr,"No daemon, therefore: exiting...\n");
}
