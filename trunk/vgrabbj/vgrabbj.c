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

#include <vgrabbj.h>

/* Globals */

int terminate=0;

static struct palette_list plist[] = {
  { 0, NULL },
  { VIDEO_PALETTE_GREY,   "GREY" },
  { VIDEO_PALETTE_HI240,  "HI240" },
  { VIDEO_PALETTE_RGB565, "RGB565" },
  { VIDEO_PALETTE_RGB24,  "RGB24" },
  { VIDEO_PALETTE_RGB32,  "RGB32" },
  { VIDEO_PALETTE_RGB555, "RGB555" },
  { VIDEO_PALETTE_YUV422, "YUV422" },
  { VIDEO_PALETTE_YUYV,   "YUYV" },
  { VIDEO_PALETTE_UYVY,   "UYVY" },
  { VIDEO_PALETTE_YUV420, "YUV420" },
  { VIDEO_PALETTE_YUV411, "YUV411" },
  { VIDEO_PALETTE_RAW,    "RAW" },
  { VIDEO_PALETTE_YUV422P,"YUV422P" },
  { VIDEO_PALETTE_YUV411P,"YUV411P" },
  { VIDEO_PALETTE_YUV420P,"YUV420P" },
  { VIDEO_PALETTE_YUV410P,"YUV410P" },
  { -1, NULL }
};

/* Event handler for SIGKILL */

void sigterm();

void sigterm() {
  signal(SIGTERM,sigterm); /* reset signal */
  syslog(LOG_WARNING, "Caught sigterm, cleaning up...");
  terminate=1;
}

/* Cleanup in case of TERM signal */

void cleanup(struct vconfig *vconf, 
#ifdef HAVE_LIBTTF
	     struct ttneed *ttinit, 
#endif
	     char *buffer, char *o_buffer) {

#ifdef HAVE_LIBTTF
  if ( vconf->use_ts ) {
    Face_Done(ttinit->instance, ttinit->face);
    TT_Done_FreeType(ttinit->engine);
  }
  free(ttinit->properties);
  free(ttinit);
#endif
  free(vconf);
  syslog(LOG_CRIT, "exiting.");
  closelog();
  _exit(1);
}

void v_error(struct vconfig *vconf, int msg, char *fmt, ...)
{
  va_list arg_ptr;
  static char buf[MAX_ERRORMSG_LENGTH];

  /* Loglevels:

     0     silent (NONE)
     2     severe errors which cause vgrabbj to exit in daemon mode (LOG_CRIT)
     3     critical errors which cause vgrabbj to exit in non-daemon mode
           and retry forever in daemon-mode (with a sleep(1) inbetween) (LOG_ERR)
     4     warning conditions (LOG_WARNING)
     6     normal information (LOG_INFO)
     7     debug information (LOG_DEBUG)

     Every log-message with a loglevel equal-smaller than LOGLEVEL-Level (supplied
     via command line, otherwise default=4) will be 
         a) displayed on screen (stderr) if non-daemon
         b) written to syslog if daemon

     One exception:

     In daemon mode, every message before successful fork into background
     will be written to stderr (if any).

  */

  if ( msg <= vconf->debug ) {
   
    va_start(arg_ptr, fmt);
    
    vsprintf(buf, fmt, arg_ptr);
    
    strcat(buf, "\n");

    va_end(arg_ptr);
  
    if (vconf->loop && vconf->init_done) {
      
      syslog(msg, buf);
      if (msg == 3) {
	sleep(1);
      }
    } else {
      fprintf(stderr, buf);
      fflush(stderr);
    }
    
  }

  /* Decision about exiting is independent of log or not to log */

  if ( (vconf->loop && msg < 3) || (msg==3 && vconf->err_count++ > 3600) ) {
    syslog(LOG_ERR, "Fatal Error, exiting...\n"); // exit
    closelog();
    _exit(1);
  } else if ( !vconf->loop && msg < 4 ) {
    fprintf(stderr, "Fatal Error, exiting...\n"); // exit
    exit(1);
  }
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
	  " -b                Switch vgrabbj's brightness adjustment (default: off)\n"
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
	  " -S                Switch BGR colormap to RGB colormap (try if colors are odd)\n"
#ifdef HAVE_LIBTTF
	  " -t <font-file>    Full path to the font file\n"
	  "                   (default: %s)\n"
	  " -T <font-size>    Font-size (min. 1, max. 100, default: %d)\n"
	  " -p <format-str>   Definable timestamp format (see man strftime)\n"
	  "                   (default: \"%s\")\n"
	  "                   *MUST* be with \" and \" !\n"
	  " -a <0|1|2|3|4|5>  Alignment of timestamp: 0=upper left, 1=upper right,\n"
	  "                   2=lower left, 3=lower right, 4=upper center, 5=lower center\n"
	  "                   you still have to enable the timestamp (default: %d) \n"
	  " -m <blendvalue>   Blending of timestamp on original image (1-100, default: %d)\n"
	  "                   1 = most original, 100 = no original image \"behind\" timestamp\n"
	  " -B <pixel>        Border of timestamp to be blended around string in pixel\n"
	  "                   (1-255, default: %d)\n"
	  " -e                enable timestamp with defaults (default: disabled)\n"
	  "                   if any other timestamp option is given, it is enabled\n"
#endif
	  " -D <0|2|3|4|6|7>  Set log/debug-level (0=silent, 7=debug, default: %d)\n"
	  " -V                Display version information and exit\n"
	  " -F <value>        Force usage of specified palette (see videodev.h for values)\n"
	  "                   (Fallback to supported palette, if this one is not supported\n"
	  " -C                Show copyright information in image (lower left corner)\n"
	  "\n"
	  "Example: %s -l 5 -f /usr/local/image.jpg\n"
	  "         Would write a single jpeg-image to image.jpg approx. every five seconds\n"
	  "\n"
	  "The video stream has to one of RGB24, RGB32, YUV420, YUV420P or YUYV.\n",
	  basename(pname), VERSION, basename(pname), 
	  DEFAULT_QUALITY, DEFAULT_WIDTH, DEFAULT_HEIGHT,
	  DEFAULT_OUTPUT, DEFAULT_VIDEO_DEV, 
#ifdef HAVE_LIBTTF
	  DEFAULT_FONT, DEFAULT_FONTSIZE, DEFAULT_TIMESTAMP,
	  DEFAULT_ALIGN, DEFAULT_BLEND, DEFAULT_BORDER, 
#endif
	  LOGLEVEL, basename(pname));
  exit (1);
}

/* Get information from v4l device and show them  */

void show_capabilities(char *in, char *pname) 
{
  struct video_capability cap;
  struct video_window win;
  struct video_picture pic;
  int dev;

  if ( (dev = open(in, O_RDONLY)) < 0 ) {
    fprintf(stderr, "Can't open device %s", in);
    exit(1);
  }
  if (ioctl(dev, VIDIOCGCAP, &cap) < 0) {
    fprintf(stderr, "Can't get capabilities of device %s", in);
    exit(1);
  }
  if (ioctl(dev, VIDIOCGPICT, &pic) < 0) {
    fprintf(stderr, "Can't get picture properties of device %s", in);
    exit(1);
  }
  if (ioctl(dev, VIDIOCGWIN, &win) < 0) {
    fprintf(stderr, "Can't get overlay values of device %s", in);
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
	  "Palette   : %s (%d)\n"
	  "Width     : %d\n"
	  "Height    : %d\n"
	  "Chromakey : %d\n",
	  basename(pname), VERSION, in, cap.name, cap.type, cap.channels, cap.audios,
	  cap.maxwidth, cap.maxheight, cap.minwidth, cap.minheight,
	  pic.brightness, pic.hue, pic.colour, pic.contrast,
	  pic.whiteness, pic.depth, plist[pic.palette].name, pic.palette,
	  win.width, win.height, win.chromakey);
  dev=close(dev);
  if (dev)
    fprintf(stderr, "Error occured while closing %s\n", in);
  exit(0);
}

struct vconfig *decode_options(struct vconfig *vconf, int argc, char *argv[]) {
  int n;
  FILE *x;
  int dev;

  // Set defaults
  vconf->quality    = DEFAULT_QUALITY;
  vconf->in         = DEFAULT_VIDEO_DEV;
  vconf->out        = DEFAULT_OUTPUT;
  vconf->win.height = DEFAULT_HEIGHT;
  vconf->win.width  = DEFAULT_WIDTH;
  vconf->outformat  = DEFAULT_OUTFORMAT;
  vconf->brightness = DEFAULT_BRIGHTNESS;
  vconf->switch_bgr = FALSE;
  vconf->windowsize = TRUE;
  vconf->loop       = 0;
  vconf->use_ts     = FALSE;
  vconf->init_done  = FALSE;
  vconf->debug      = LOGLEVEL;
  vconf->err_count  = 0;
  vconf->dev        = 0;
  vconf->forcepal   = 0;
#ifdef HAVE_LIBTTF
  vconf->font       = DEFAULT_FONT;
  vconf->timestamp  = DEFAULT_TIMESTAMP;
  vconf->font_size  = DEFAULT_FONTSIZE;
  vconf->border     = DEFAULT_BORDER;
  vconf->align      = DEFAULT_ALIGN;
  vconf->blend      = DEFAULT_BLEND;
#endif
  
  while ((n = getopt (argc, argv, "L:l:f:q:hd:s:o:t:T:p:ebi:a:D:B:m:wSVMN:F:"))!=EOF) 
    {
      switch (n) 
	{
	case 'l':
	  if ( sscanf (optarg, "%ld", &vconf->loop) != 1 || ( vconf->loop < 1 ) ) 
	    v_error(vconf, LOG_CRIT, "Wrong sleeptime"); // exit
	  vconf->loop=vconf->loop*1000000;
	  break;
	case 'L':
	  if ( sscanf (optarg, "%ld", &vconf->loop) != 1 || ( vconf->loop < 1 ) ) 
	    v_error(vconf, LOG_CRIT, "Wrong sleeptime"); // exit
	  break;
	case 'f':
	  if ( !(x=fopen(vconf->out=optarg, "w+") ) ) 
	    v_error(vconf, LOG_CRIT, "Can't access output file %s",vconf->out); // exit
	  fclose(x);
	  if (vconf->debug)
	    v_error(vconf, LOG_DEBUG, "Outputfile is %s", vconf->out);
	  break;
	case 'q':
	  if ( sscanf (optarg, "%d", &vconf->quality) != 1 || (vconf->quality<0)
	       || (vconf->quality>100) ) 
	    v_error(vconf, LOG_CRIT, "Wrong picture quality \"%d\"", vconf->quality); // exit
	  if (vconf->debug)
	    v_error(vconf, LOG_DEBUG, "Image quality is %d", vconf->quality);
	  break;
	case 'o':
	  if ( !strcasecmp(optarg,"jpeg") || !strcasecmp(optarg,"jpg") )
	    vconf->outformat=1;
	  else if ( !strcasecmp(optarg,"png") )
	    vconf->outformat=2;
	  else 
	    v_error(vconf, LOG_CRIT, "Wrong output format specified"); // exit
	  break;
	case 'i':
	  if ( !strcasecmp(optarg,"sqcif") ) {
	    vconf->win.width  = 128;
	    vconf->win.height =  96;
	  } else if ( !strcasecmp(optarg,"qsif") ) {
	    vconf->win.width  = 160;
	    vconf->win.height = 120;
	  } else if ( !strcasecmp(optarg,"qcif") ) {
	    vconf->win.width  = 176;
	    vconf->win.height = 144;
	  } else if ( !strcasecmp(optarg,"sif") ) {
	    vconf->win.width  = 320;
	    vconf->win.height = 240;
	  } else if ( !strcasecmp(optarg,"cif") ) {
	    vconf->win.width  = 352;
	    vconf->win.height = 288;
	  } else if ( !strcasecmp(optarg,"vga") ) {
	    vconf->win.width  = 640;
	    vconf->win.height = 480;
	  } else
	    v_error(vconf, LOG_CRIT, "Wrong imagesize specified"); // exit
	  break;
	case 'd':
	  if ( ( dev=open( vconf->in = optarg, O_RDONLY) ) < 0 ) 
	    v_error(vconf, LOG_CRIT, "Device %s not accessible", vconf->in); // exit
	  dev=close(dev);
	  if (dev)
	    v_error(vconf, LOG_CRIT, "Device %s error occured", vconf->in); // exit
	  break;
#ifdef HAVE_LIBTTF
	case 't':
	  if ( !( x = fopen(vconf->font = optarg, "r") ) )
	    v_error(vconf, LOG_CRIT, "Font-file %s not found", vconf->font); // exit
	  vconf->use_ts=TRUE;
	  vconf->font=optarg;
	  fclose(x);
	  break;
	case 'T':
	  if ( sscanf(optarg, "%d", &vconf->font_size) != 1 || vconf->font_size < 1
	       || vconf->font_size > 100 ) 
	    v_error(vconf, LOG_CRIT, "Wrong fontsize (min. 1, max 100)"); // exit
	  vconf->use_ts=TRUE;
	  break;
	case 'p':
	  if ( !( x = fopen(vconf->font, "r") ) ) 
	    v_error(vconf, LOG_CRIT, "Fontfile %s not found", vconf->font); // exit
	  fclose(x);
	  vconf->timestamp = optarg;
	  vconf->use_ts = TRUE;
	  break;
	case 'e':
	  vconf->use_ts=TRUE;
	  break;
	case 'a':
	  if ( sscanf (optarg, "%d", &vconf->align) != 1 || vconf->align < 0
	       || vconf->align>5 ) 
	    v_error(vconf, LOG_CRIT, "Wrong timestamp alignment"); // exit
	  vconf->use_ts=TRUE;
	  break;
	case 'm':
	  if ( sscanf (optarg, "%d", &vconf->blend) != 1 || vconf->blend > 100
	       || vconf->blend < 1 ) 
	    v_error(vconf, LOG_CRIT, "Wrong blend value"); // exit
	  vconf->use_ts=TRUE;
	  break;
	case 'B':
	  if ( sscanf (optarg, "%d", &vconf->border) != 1 || vconf->border > 255
	       || vconf->border < 1 ) 
	    v_error(vconf, LOG_CRIT, "Wrong border value"); // exit
	  vconf->use_ts=TRUE;
	  break;
#endif
	case 'b':
	  vconf->brightness = !DEFAULT_BRIGHTNESS;
	  break;
	case 's':
	  if ( (dev = open(vconf->in = optarg, O_RDONLY) ) < 0 ) 
	    v_error(vconf, LOG_CRIT, "Device %s not accessible", vconf->in); // exit
	  dev=close(dev);
	  if (dev)
	    v_error(vconf, LOG_CRIT, "Device %s error occured", vconf->in); // exit
	  show_capabilities(vconf->in, argv[0]);
	  break;
	case 'D':
	  if ( sscanf (optarg, "%d", &vconf->debug) != 1 || vconf->debug >7
	       || vconf->debug < 0 )
	    v_error(vconf, LOG_CRIT, "Wrong debuglevel value");
	  break;
	case 'w':
	  vconf->windowsize=FALSE;
	  break;
	case 'S':
	  vconf->switch_bgr=TRUE;
	  break;
	case 'V':
	  fprintf(stderr, "%s %s\n", basename(argv[0]), VERSION);
	  exit(0);
	  break;
	case 'F':
	  sscanf(optarg, "%d", &vconf->forcepal);
	  break;
	default:
	  usage (argv[0]);
	  break;
	}
    }
  return vconf;
}

/* Adjustment of brightness of picture  */

int get_brightness_adj(struct vconfig *vconf, unsigned char *image, int *brightness) 
{
  long i, tot = 0;
  long size = vconf->win.width * vconf->win.height;
  for ( i = 0; i < size * 3; i++ )
    tot += image[i];
  *brightness = (128 - tot/(size*3))/3;
  return !((tot/(size*3)) >= 126 && (tot/(size*3)) <= 130);
}

/* Write image as jpeg to FILE  */

int write_jpeg(struct vconfig *vconf, char *buffer, FILE *x) 
{
  char *line;
  int n, y=0, i, line_width;
  
  struct jpeg_compress_struct cjpeg;
  struct jpeg_error_mgr jerr;
  JSAMPROW row_ptr[1];
  
  line=malloc(vconf->win.width * 3);
  if (!line) 
    v_error(vconf, LOG_CRIT, "OUT OF MEMORY, Exiting..."); // exit
  cjpeg.err = jpeg_std_error(&jerr);
  jpeg_create_compress (&cjpeg);
  cjpeg.image_width = vconf->win.width;
  cjpeg.image_height= vconf->win.height;
  cjpeg.input_components = 3;
  cjpeg.in_color_space = JCS_RGB;
  
  jpeg_set_defaults (&cjpeg);
  jpeg_set_quality (&cjpeg, vconf->quality, TRUE);
  cjpeg.dct_method = JDCT_FASTEST;
  
  jpeg_stdio_dest (&cjpeg, x);
  jpeg_start_compress (&cjpeg, TRUE);
  row_ptr[0]=line;
  line_width=vconf->win.width * 3;
  n=0;
  
  for (y = 0; y < vconf->win.height; y++) 
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
  free(line);
  return(0);
}

/* Write image as png to FILE  */

int write_png(struct vconfig *vconf, char *image, FILE *x) 
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
  png_set_IHDR (png_ptr, info_ptr, vconf->win.width, vconf->win.height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_set_bgr (png_ptr);
  png_write_info (png_ptr, info_ptr);
  p = image;
  vconf->win.width *= 3;
  for (y = 0; y < vconf->win.height; y++) 
    {
      png_write_row (png_ptr, p);
      p += vconf->win.width;
    }
  png_write_end (png_ptr, info_ptr);
  png_destroy_write_struct (&png_ptr, &info_ptr);
  return(0);
}

/* Check if palette is supported by v4l device */

int try_palette(struct vconfig *vconf, int palette, int dev)
{
  v_error(vconf, LOG_INFO, "Trying palette %s", plist[palette].name);
  vconf->vpic.palette=palette;

  if (ioctl(dev, VIDIOCSPICT, &vconf->vpic) < 0) {
    v_error(vconf, LOG_WARNING, "Unable to set palette"); // exit
    return 0;
  }
  if (ioctl(dev, VIDIOCSPICT, &vconf->vpic) < 0) {
    v_error(vconf, LOG_WARNING, "Unable to get palette info"); // exit
    return 0;
  }
  if (vconf->vpic.palette == palette)
    return palette;
  return 0;
}

/* Check and set v4l device */

struct vconfig *check_device(struct vconfig *vconf) {

  struct video_window twin;
  int dev;

  while ((dev=open(vconf->in, O_RDONLY)) < 0)
    v_error(vconf, LOG_ERR, "Problem opening input-device %s", vconf->in);

  v_error(vconf, LOG_DEBUG, "Device %s successfully opened", vconf->in);

  if (vconf->debug)
    v_error(vconf, LOG_INFO, "Checking settings of device %s", vconf->in);
  
  while (ioctl(dev, VIDIOCGCAP, &vconf->vcap) < 0)
    v_error(vconf, LOG_ERR, "Problem getting video capabilities"); // exit
  if ( (vconf->vcap.maxwidth < vconf->win.width) ||
       (vconf->vcap.minwidth > vconf->win.width) ||
       (vconf->vcap.maxheight < vconf->win.height) ||
       (vconf->vcap.minheight > vconf->win.height) )
    v_error(vconf, LOG_CRIT, "Device doesn't support width/height"); // exit
  while (ioctl(dev, VIDIOCGWIN, &twin))
    v_error(vconf, LOG_ERR, "Problem getting window information"); // exit
  vconf->win.flags=twin.flags;
  vconf->win.x=twin.x;
  vconf->win.y=twin.y;
  vconf->win.chromakey=twin.chromakey;
  if (vconf->windowsize)
    while (ioctl(dev, VIDIOCSWIN, &vconf->win) )
      v_error(vconf, LOG_ERR, "Problem setting window size"); // exit
  while (ioctl(dev, VIDIOCGWIN, &vconf->win) <0)
    v_error(vconf, LOG_ERR, "Problem getting window size"); // exit
  while (ioctl(dev, VIDIOCGPICT, &vconf->vpic) < 0)
    v_error(vconf, LOG_ERR, "Problem getting picture properties"); // exit

  // HERE we actually TRY to get a palette the device delivers.
  // PROBLEM is that V4L does NOT provide a function to query available
  // palettes for the device! Hence, this util has to rely on try-and-error
  // to find a palette suitable.
  // Currently, only palettes below are supported directly.
  // If it is a different one, it has to be one of those tested below - simply because
  // I have no other conversion routines on hand.
  // If this prog does not work with your device, please blame someone else for
  // an insufficient V4L implementation.
  // Sorry for the inconvenience!

  if (vconf->forcepal)
    if ( (vconf->vpic.palette=try_palette(vconf, vconf->forcepal, dev)) )
      v_error(vconf, LOG_INFO, "Set palette successfully to %s", plist[vconf->vpic.palette].name);
  
  switch(vconf->vpic.palette) {
  case VIDEO_PALETTE_RGB24:
  case VIDEO_PALETTE_YUV420P:
  case VIDEO_PALETTE_YUV420:
  case VIDEO_PALETTE_YUYV:
  case VIDEO_PALETTE_YUV422: // equal to YUYV with my cam
  case VIDEO_PALETTE_RGB32:
    break;
  default:
    if ( (vconf->vpic.palette=try_palette(vconf, VIDEO_PALETTE_RGB24, dev)) ||
	 ( vconf->vpic.palette=try_palette(vconf, VIDEO_PALETTE_RGB32, dev)) ||
	 ( vconf->vpic.palette=try_palette(vconf, VIDEO_PALETTE_YUYV, dev)) ||
	 ( vconf->vpic.palette=try_palette(vconf, VIDEO_PALETTE_YUV420, dev)) ||
	 ( vconf->vpic.palette=try_palette(vconf, VIDEO_PALETTE_YUV420P, dev)) )
      v_error(vconf, LOG_DEBUG, "Set palette successfully to %s", plist[vconf->vpic.palette].name);
    else
      v_error(vconf, LOG_CRIT, "Unable to set supported video-palette"); // exit
    break;
  }
    
  while ( close(dev) )
    v_error(vconf, LOG_ERR, "Error while closing %s", vconf->in); // exit
  
  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Device %s closed", vconf->in);

  return vconf;
}

/* Turns RGB into BGR (or vice versa) */

unsigned char *switch_color(struct vconfig *vconf, unsigned char *buffer) {
  char a;
  long int y;
  for (y = 0; y < (vconf->win.width * vconf->win.height); y++) {
    memcpy(&a, buffer+(y*3),1);
    memcpy(buffer+(y*3),buffer+(y*3)+2, 1);
    memcpy(buffer+(y*3)+2, &a, 1);
  }
  return buffer;
}

/* Strips last byte of RGB32 to convert to RGB24 - breaks picture if alpha is used! */

unsigned char *conv_rgb32_rgb24(unsigned char *o_buffer, unsigned char *buffer,
				int width, int height) {
  long int y;
  for (y = 0; y < (width * height); y++) 
    memcpy(o_buffer+(y*3), buffer+(y*4), 3);
  return o_buffer;
}

/* Read the image from device, adjust brightness, if wanted, for RGB24 */

unsigned char *read_image(struct vconfig *vconf, unsigned char *buffer, int size) {
  int f, newbright;
  int err_count, dev;
  struct video_mmap vmap;
  struct video_mbuf vbuf;
  //  struct video_channel vchan;
  char *map;

  if (vconf->debug)
    v_error(vconf, LOG_DEBUG, "Palette to be used: %s (%d), size: %d",
	    plist[vconf->vpic.palette].name, vconf->vpic.palette, size);

  // Opening input device

  while ((dev=open(vconf->in, O_RDONLY)) < 0)
    v_error(vconf, LOG_ERR, "Problem opening input-device %s", vconf->in);

  if (vconf->debug)
    v_error(vconf, LOG_DEBUG, "Device %s successfully opened", vconf->in);

  // Re-initialize the palette, in case someone changed it meanwhile

  while (ioctl(dev, VIDIOCSPICT, &vconf->vpic) < 0 )
    v_error(vconf, LOG_ERR, "Device %s couldn't be reset to known palette %s",
	    vconf->in, vconf->vpic.palette);
  if (vconf->windowsize)
    while (ioctl(dev, VIDIOCSWIN, &vconf->win) )
      v_error(vconf, LOG_ERR, "Problem setting window size"); // exit
  
  // Read image via read()

  if (ioctl(dev, VIDIOCGMBUF, &vbuf) < 0 || vconf->brightness) {
    if (vconf->brightness)
      v_error(vconf, LOG_INFO, "Forced to use brightness adj. - using read()");
    else
      v_error(vconf, LOG_INFO, "Could not get mmap-buffer - Falling back to read()");

    err_count=0;
    if (vconf->brightness && vconf->vpic.palette==VIDEO_PALETTE_RGB24) {
      if (vconf->debug) 
	v_error(vconf, LOG_INFO, "Doing brightness adjustment");
      do {
	while (read(dev, buffer, size) < size)
	  v_error(vconf, LOG_ERR, "Error reading from %s", vconf->in);
	f = get_brightness_adj(vconf, buffer, &newbright);
	if (f) {
	  vconf->vpic.brightness += (newbright << 8);
	  if (ioctl(dev, VIDIOCSPICT, &vconf->vpic)==-1) 
	    v_error(vconf, LOG_WARNING, "Problem setting brightness");
	  err_count++;
	  
	  if (err_count>100) {
	    v_error(vconf, LOG_WARNING, "Brightness not optimal");
	    break;
	  }
	}
      } while (f);
      if (vconf->debug)
	v_error(vconf, LOG_INFO, "Brightness adjusted");
    } else {
      v_error(vconf, LOG_DEBUG, "Using normal read for image grabbing");
      read(dev, buffer, size);
    }
  } else { // read mmaped
    v_error(vconf, LOG_DEBUG, "Using mmap for image grabbing");
    vmap.height=vconf->win.height;
    vmap.width=vconf->win.width;
    vmap.frame=0;
    vmap.format=vconf->vpic.palette;

    map = mmap(0, vbuf.size, PROT_READ, MAP_SHARED, dev, 0);
    v_error(vconf, LOG_DEBUG, "Size allocated for framebuffer: %d", vbuf.size);

    if (!map)
      v_error(vconf, LOG_CRIT, "mmap'ed area not allocated");

    while (ioctl(dev,VIDIOCMCAPTURE,&vmap) < 0)
      v_error(vconf, LOG_ERR, "Could not grab a frame");    // grab a frame

    while (ioctl(dev,VIDIOCSYNC,&vmap.frame) < 0)
      v_error(vconf, LOG_ERR, "Could not sync with frame");  // sync with frame and wait for result

    buffer=memcpy(buffer, map, size);

    munmap(map, vbuf.size);
  }
  
  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Image successfully read");

  // Closing Input Device
  
  while ( close(dev) )
    v_error(vconf, LOG_ERR, "Error while closing %s", vconf->in); // exit
  
  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Device %s closed", vconf->in);

  return buffer;
}

int daemonize(struct vconfig *vconf, char *progname) 
{
  openlog(progname, LOG_DEBUG, LOG_DAEMON);
  if (vconf->debug)
    v_error(vconf, LOG_DEBUG, "Forking for daemon mode");
    
  switch( fork() ) 
    {
    case 0: // Child 
      if (vconf->debug) 
	v_error(vconf, LOG_DEBUG, "I'm the child process and are going to read images...");
      closelog();
      break;
    case -1: /* Error  */
      v_error(vconf, LOG_CRIT, "Can't fork, exiting..."); // exit
      break;
    default: /* Parent  */
      if (vconf->debug) 
	v_error(vconf, LOG_DEBUG, "I'm the parent and exiting now"
		"(child takes care of the rest).");
   closelog();
   free(vconf);
   exit(0);
    }
  openlog(progname, LOG_PID, LOG_DAEMON);
  v_error(vconf, LOG_WARNING, "%s started, reading from %s", progname, vconf->in);
  
  return(1);
}

/* Now for the LIBTTF functions for the timestamp */
		
#ifdef HAVE_LIBTTF

/* Open Face via routine found in font.c  */

struct ttneed *OpenFace(struct ttneed *ttinit, struct vconfig *vconf) 
{
  int i, j;

  if (vconf->debug)
    v_error(vconf, LOG_INFO, "Initializing Font-Engine");

  if (vconf->debug) {
    TT_FreeType_Version( &i, &j);
    v_error(vconf, LOG_DEBUG, "FreeType, Version %d.%d", i, j);
  }

  if (Face_Open (vconf->font, ttinit->engine, &ttinit->face, ttinit->properties,
		 &ttinit->instance, vconf->font_size)) {
    v_error(vconf, LOG_WARNING, "Font not found: %s, timestamp disabled", vconf->font);
    return(0);
  } 
  if (vconf->debug)
    v_error(vconf, LOG_DEBUG, "Font-Engine initialized");

  return(ttinit);
}

/* Manipulate image to show timestamp string and return manipulated buffer */

char *inserttext(struct ttneed *ttinit, unsigned char *buffer, struct vconfig *vconf)
{
    
  time_t t;
  struct tm *tm;
  char ts_buff[TS_MAX+1];
  int ts_len;
  TT_Glyph *glyphs = NULL;
  TT_Raster_Map bit;
  TT_Raster_Map sbit;

  ttinit=OpenFace(ttinit, vconf);
  if ( !ttinit ) {
      v_error(vconf, LOG_WARNING, "Could not initialize font-engine");
      return(buffer);
  }
  
  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Getting all values for the timestamp.");

  time (&t);
  tm = localtime (&t);
  ts_buff[TS_MAX] = '\0';
  strftime (ts_buff, TS_MAX, vconf->timestamp, tm);
  ts_len = strlen (ts_buff);

  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Stamp: %s, length: %d", ts_buff, ts_len);

  glyphs = Glyphs_Load(ttinit->face, ttinit->properties, ttinit->instance,
		       ts_buff, ts_len);

  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Glyphs loaded");

  Raster_Init(ttinit->face, ttinit->properties, ttinit->instance, ts_buff,
	      ts_len, vconf->border, glyphs, &bit);

  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Returned from Raster_Init");

  Raster_Small_Init (&sbit, &ttinit->instance);

  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Returned from Raster_Small_Init");

  Render_String(glyphs, ts_buff, ts_len, &bit, &sbit, vconf->border);

  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Returned from Render_String");

  if (bit.bitmap) 
    {
      int x, y, psize, i, x_off, y_off;
      unsigned char *p;

      if (vconf->debug) 
	v_error(vconf, LOG_DEBUG, "Now performing calculation of position...");

      if (bit.rows>vconf->win.height) 
	bit.rows=vconf->win.height;
      if (bit.width>vconf->win.width) 
	bit.width=vconf->win.width;
      psize = 3;
      switch (vconf->align) 
	{
	case 1:
	  x_off = (vconf->win.width - bit.width) * psize;
	  y_off = 0;
	  break;
	case 2:
	  x_off = 0;
	  y_off = vconf->win.height - bit.rows;
	  break;
	case 3:
	  x_off = (vconf->win.width - bit.width) * psize;
	  y_off = vconf->win.height - bit.rows;
	  break;
	case 4:
	  x_off = (vconf->win.width/2 - bit.width/2) * psize;
	  y_off = 0;
	  break;
	case 5:
	  x_off = (vconf->win.width/2 - bit.width/2) * psize;
	  y_off = vconf->win.height - bit.rows;
	  break;
	default:
	  x_off = y_off = 0;
	  break;
	}

      if (vconf->debug) 
	v_error(vconf, LOG_DEBUG, "Done. Now we change the image with the string.");

      for (y = 0; y < bit.rows; y++) {
	  p = buffer + (y + y_off) * (vconf->win.width * psize) + x_off;
	  for (x = 0; x < bit.width; x++) {
	      switch (((unsigned char *)bit.bitmap)
		      [((bit.rows-y-1)*bit.cols)+x]) {
		case 0:
		  for (i = 0; i < psize; i++) {
		      *p = (255 * vconf->blend + *p * (100 - vconf->blend))/100;
		      p++;
		    }
		  break;
		case 1:
		  for (i = 0; i < psize; i++) {
		      *p = (220 * vconf->blend + *p * (100 - vconf->blend))/100;
		      p++;
		    }
		  break;
		case 2:
		  for (i = 0; i < psize; i++) {
		      *p = (162 * vconf->blend + *p * (100 - vconf->blend))/100;
		      p++;
		    }
		  break;
		case 3:
		  for (i = 0; i < psize; i++) {
		      *p = (64 * vconf->blend + *p * (100 - vconf->blend))/100;
		      p++;
		    }
		  break;
		default:
		  for (i = 0; i < psize; i++) {
		      *p = (0 * vconf->blend + *p * (100 - vconf->blend))/100;
		      p++;
		    }
		  break;
		}
	    }
	}
    }

  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Image manipulated, now closing...");

  Raster_Done (&sbit);
  Raster_Done (&bit);
  Glyphs_Done (glyphs);
  glyphs = NULL;
  Face_Done(ttinit->instance, ttinit->face);

  if (vconf->debug) 
    v_error(vconf, LOG_INFO, "Font-Engine unloaded, stamp inserted into image");

  return buffer;
}

#endif

/* Main loop  */

int main(int argc, char *argv[]) 
{
  struct vconfig *vconf;
  unsigned char *buffer, *o_buffer, *t_buffer;
  int size;
  FILE *x;

#ifdef HAVE_LIBTTF
  struct ttneed *ttinit;
#endif

  signal(SIGTERM,sigterm);

#ifdef DEBUGGING
  mtrace();
#endif

  vconf=malloc(sizeof(*vconf));
  vconf=decode_options(vconf, argc, argv);

  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Read all arguments, starting up...");

  vconf->init_done=TRUE;

  if (vconf->loop) 
    daemonize(vconf, basename(argv[0]));
  else
    v_error(vconf, LOG_WARNING, "Reading image from %s", vconf->in);

#ifdef HAVE_LIBTTF
  ttinit = malloc(sizeof(*ttinit));
  ttinit->properties = malloc(sizeof(*ttinit->properties));
 
  if ( !ttinit->properties || !ttinit )
    vconf->use_ts=FALSE;
  
  if (vconf->use_ts)
    if (TT_Init_FreeType(&ttinit->engine)) {
      v_error(vconf, LOG_WARNING, "Could not initialize Font-Engine, timestamp disabled");
      vconf->use_ts=FALSE;
  }
#endif

  vconf=check_device(vconf);

  /* initialize appropriate memory */
  
  if (vconf->debug)
    v_error(vconf, LOG_DEBUG, "Initializing memory");
  
  switch (vconf->vpic.palette) {
  case VIDEO_PALETTE_RGB24:
    size = vconf->win.width * vconf->win.height * 3;
    break;
  case VIDEO_PALETTE_RGB32:
    size = vconf->win.width * vconf->win.height * 4;
    break;
  case VIDEO_PALETTE_YUYV:
  case VIDEO_PALETTE_YUV422: // equal to YUYV
    size = vconf->win.width * vconf->win.height * 2;
    break;
  case VIDEO_PALETTE_YUV420:
    size = vconf->win.width * vconf->win.height * 3 / 2;
    break;
  case VIDEO_PALETTE_YUV420P:
    size = vconf->win.width * vconf->win.height * 3 / 2;
    break;
  default:
    size=0;
    v_error(vconf, LOG_CRIT, "No supported palette available!"); //exit	
    break;
  }
  
  buffer=malloc(size);                                       // depending on palette
  o_buffer=malloc(vconf->win.width * vconf->win.height * 3); // RGB24 (3 byte/pixel)
  if (!buffer || !o_buffer) 
    v_error(vconf, LOG_CRIT, "Out of memory! Exiting...");
  
  if (vconf->debug)
    v_error(vconf, LOG_DEBUG, "Memory initialized, size: %d (in), %d (tmp), %d (out)",
	    size, vconf->win.width * vconf->win.height * 4,
	    vconf->win.width * vconf->win.height * 3);
  
  // now start the loop (if daemon), read image and convert it, if necessary 
  do {
    
    buffer = read_image(vconf, buffer, size);
    
    switch (vconf->vpic.palette) {
    case VIDEO_PALETTE_RGB24:
      o_buffer=memcpy(o_buffer, buffer, 
		      vconf->win.width * vconf->win.height * 3);
      if (vconf->debug)
	v_error(vconf, LOG_DEBUG, "No conversion, we have RGB24");
      break;
      
    case VIDEO_PALETTE_RGB32:
      if (vconf->debug)
	v_error(vconf, LOG_INFO, "Got RGB32, converting...");
      
      o_buffer=conv_rgb32_rgb24(o_buffer, buffer, vconf->win.width, vconf->win.height);
      break;
      
    case VIDEO_PALETTE_YUV420P:
      if (vconf->debug)
	v_error(vconf, LOG_INFO, "Got YUV420p, converting...");
      
      ccvt_420p_bgr24(vconf->win.width, vconf->win.height, buffer,
		      buffer + (vconf->win.width * vconf->win.height),
		      buffer + (vconf->win.width * vconf->win.height)+
		      (vconf->win.width * vconf->win.height / 4),
		      o_buffer);
      break;
      
    case VIDEO_PALETTE_YUV420:
      if (vconf->debug)
	v_error(vconf, LOG_INFO, "Got YUV420, converting...");

      t_buffer=malloc(size);
      ccvt_420i_420p(vconf->win.width, vconf->win.height, buffer, t_buffer,
		     t_buffer + (vconf->win.width * vconf->win.height),
		     t_buffer + (vconf->win.width * vconf->win.height)+
		     (vconf->win.width * vconf->win.height / 4));
      ccvt_420p_bgr24(vconf->win.width, vconf->win.height, t_buffer,
		      t_buffer + (vconf->win.width * vconf->win.height),
		      t_buffer + (vconf->win.width * vconf->win.height)+
		      (vconf->win.width * vconf->win.height / 4),
		      o_buffer);
		      free(t_buffer);
		      // As of now the direct conversion does not work properly.
		      // Therefore, I use a very slow but working solution...
//      ccvt_420i_rgb24(vconf->win.width, vconf->win.height, buffer, o_buffer);
		    
      break;
    case VIDEO_PALETTE_YUYV:
    case VIDEO_PALETTE_YUV422:
      if (vconf->debug)
	v_error(vconf, LOG_INFO, "Got YUYV, converting...");
      
      ccvt_yuyv_bgr24(vconf->win.width, vconf->win.height, buffer, o_buffer);
      break;
    default:
      v_error(vconf, LOG_CRIT, "Should not happen - Unknown input image format");
      break;
    }
    
    if (vconf->debug && vconf->vpic.palette!=VIDEO_PALETTE_RGB24)
      v_error(vconf, LOG_DEBUG, "converted to RGB24");

    if (vconf->switch_bgr) {
      o_buffer=switch_color(vconf, o_buffer);
      if (vconf->debug)
	v_error(vconf, LOG_DEBUG, "Switching vom BGR to RGB - or vice versa");
    }

#ifdef HAVE_LIBTTF
    if (vconf->use_ts) 
      o_buffer=inserttext(ttinit, o_buffer, vconf);
#endif
    
    while (! (x = fopen(vconf->out, "w+") ) )
      v_error(vconf, LOG_ERR, "Could not open outputfile %s", vconf->out);
    if (vconf->debug)
      v_error(vconf, LOG_DEBUG, "Opened output-file %s", vconf->out);
    
    switch (vconf->outformat) 
      {
      case 1:
	while (write_jpeg(vconf, o_buffer, x))
	  v_error(vconf, LOG_ERR, "Could not write outputfile %s", vconf->out);
	break;
      case 2:
	while (write_png(vconf, o_buffer, x)) 
	  v_error(vconf, LOG_ERR, "Could not write outputfile %s", vconf->out);
	break;
      default:		// should never happen  
	v_error(vconf, LOG_CRIT, "Unknown error! Report all circumstances to author");
	break;
      }
    fclose(x);
    
    if (vconf->debug) 
      v_error(vconf, LOG_DEBUG, "Outputfile %s closed", vconf->out);
    
    vconf->err_count=0;
    usleep(vconf->loop);
    
    // We got a SIGTERM, so we have to exit now (for daemon mode)
    if (terminate == 1)
      cleanup(vconf,
#ifdef HAVE_LIBTTF
	      ttinit,
#endif
	      buffer, o_buffer);
    
  } while (vconf->loop);
  
  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG,"No daemon, exiting...");
  
  free (buffer);
  free (o_buffer);
  if (vconf->debug) 
    v_error(vconf, LOG_DEBUG, "Image-buffers freed");
  
#ifdef HAVE_LIBTTF
  if (vconf->use_ts)
    TT_Done_FreeType(ttinit->engine);
  free(ttinit->properties);
  free(ttinit);
#endif
  
  free(vconf);
  exit(0);
}
