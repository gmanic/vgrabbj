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

/* Functions to parse the (optional) config file, or command */
/* line options, print out help information                  */


#include "vgrabbj.h"

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
	  " -q <quality>      Quality setting (%d-%d, default: %d), JPEG only\n"
	  " -i <sqcif|qsif|qcif|sif|cif|vga|svga|xga|sxga|uxga>\n"
	  "                   Sets the imagesize of input device to sqcif=128x96,\n"
	  "                   qsif=160x120, qcif=176x144, sif=320x240, cif=352x288,\n"
	  "                   vga=640x480, svga=800x600, xga=1024x768, sxga=1280x1024, or\n"
	  "                   uxga=1600x1200 (default: %dx%d)\n"
	  " -W                Set imagesize to individual width (needs -H)\n"
	  " -H                Set imagesize to individual height (needs -W)\n"
	  "                   Be careful! These settings supersede any other setting of\n"
	  "                   the imagesize and may damage your hardware!!\n"
	  "                   The values are NOT checked to be valid!!!!\n"
	  " -o <jpg|png|ppm>  Output format (default: jpg)\n"
	  " -f <filename>     Write to <filename> (default: %s)\n"
	  " -d <device>       Read from <device> as input (default: %s)\n"
	  " -C                Open device permanently instead of opening for each image\n"
	  " -w                Disable setting of image-size, necessary for certain cams\n"
	  "                   (e.g. IBM USB-Cam, QuickCam)\n"
	  " -s <device>       See capabilities of <device>\n"
	  " -S                Switch BGR colormap to RGB colormap (try if colors are odd)\n"
#ifdef LIBTTF
	  " -t <font-file>    Full path to the font file\n"
	  "                   (default: %s)\n"
	  " -T <font-size>    Font-size (min. %d, max. %d, default: %d)\n"
	  " -p <format-str>   Definable timestamp format (see man strftime)\n"
	  "                   (default: \"%s\")\n"
	  "                   *MUST* be with \" and \" !\n"
	  " -a <0|1|2|3|4|5>  Alignment of timestamp: 0=upper left, 1=upper right,\n"
	  "                   2=lower left, 3=lower right, 4=upper center, 5=lower center\n"
	  "                   you still have to enable the timestamp (default: %d) \n"
	  " -m <blendvalue>   Blending of timestamp on original image (%d-%d, default: %d)\n"
	  "                   %d = most original, %d = no original image \"behind\" timestamp\n"
	  " -B <pixel>        Border of timestamp to be blended around string in pixel\n"
	  "                   (%d-%d, default: %d)\n"
	  " -e                enable timestamp with defaults (default: disabled)\n"
	  "                   if any other timestamp option is given, it is enabled\n"
#endif
	  " -D <0|2|3|4|6|7>  Set log/debug-level (%d=silent, %d=debug, default: %d)\n"
	  " -n                Do write directly to the output file (if not %s)\n"
	  "                   instead of using a tmp-file and copying it to the output file\n"
	  " -V                Display version information and exit\n"
	  " -F <value>        Force usage of specified palette (see videodev.h for values)\n"
	  "                   (Fallback to supported palette, if this one is not supported\n"
	  "\n"
	  "Example: %s -l 5 -f /usr/local/image.jpg\n"
	  "         Would write a single jpeg-image to image.jpg approx. every five seconds\n"
	  "\n"
	  "The video stream has to one of RGB24, RGB32, YUV420, YUV420P or YUYV.\n",
	  basename(pname), VERSION, basename(pname), MIN_QUALITY, MAX_QUALITY, 
	  DEFAULT_QUALITY, DEFAULT_WIDTH, DEFAULT_HEIGHT,
	  DEFAULT_OUTPUT, DEFAULT_VIDEO_DEV, 
#ifdef LIBTTF
	  DEFAULT_FONT, MIN_FONTSIZE, MAX_FONTSIZE, DEFAULT_FONTSIZE, DEFAULT_TIMESTAMP,
	  DEFAULT_ALIGN, MIN_BLEND, MAX_BLEND, DEFAULT_BLEND, MIN_BLEND, MAX_BLEND,
	  MIN_BORDER, MAX_BORDER, DEFAULT_BORDER, 
#endif
	  MIN_DEBUG, MAX_DEBUG, LOGLEVEL, DEFAULT_OUTPUT, basename(pname));
  exit (1);
}

struct vconfig *init_defaults(struct vconfig *vconf) {
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
  vconf->openonce   = FALSE;
  vconf->usetmpout  = TRUE;
  vconf->tmpout     = NULL;
  vconf->cpyline    = NULL;
#ifdef LIBTTF
  vconf->font       = DEFAULT_FONT;
  vconf->timestamp  = DEFAULT_TIMESTAMP;
  vconf->font_size  = DEFAULT_FONTSIZE;
  vconf->border     = DEFAULT_BORDER;
  vconf->align      = DEFAULT_ALIGN;
  vconf->blend      = DEFAULT_BLEND;
#endif
#ifdef LIBFTP
  vconf->ftp.enable = FALSE;
  vconf->ftp.state  = 0;
#endif
  return vconf;
}  


void check_files(struct vconfig *vconf) {
/* This function is for checking to make sure that all the file
 * information in vconf is right.  It's called after all configs are
 * done. Input from Michael Janssen.
 */
  int dev;
  FILE *x;
  if ( (dev=open(vconf->in, O_RDONLY)) < 0) {
    v_error(vconf, LOG_CRIT, "Can't open %s as VideoDevice!", vconf->in);
  } else {
    close(dev);
  }
  
  if ( !(x=fopen(vconf->out, "w+"))) {
    v_error(vconf, LOG_CRIT, "Can't open %s as OutputFile", vconf->out);
  } else {
    fclose(x);
  }
  
  if (!(x=fopen(vconf->font, "r"))) {
    v_error(vconf, LOG_CRIT, "Can't open %s as FontFile!", vconf->font);
  } else { 
    fclose(x);
  }
}


struct vconfig *parse_commandline(struct vconfig *vconf, int argc, char *argv[]) {
  int n;
  int dev=0;
  int is_width=0;
  int is_height=0;

  while ((n = getopt (argc, argv, "c:L:l:f:q:hd:s:o:t:T:p:ebi:a:D:B:m:gSVMN:F:Cw:H:n"))!=EOF) 
    {
      switch (n) 
	{
	case 'c':
	  vconf=parse_config(vconf, optarg);
	  break;
	case 'C':
	  vconf->openonce=TRUE;
	  v_error(vconf, LOG_DEBUG, "Videodevice set to be permanently open");
	  break;
	case 'l':
	  if ( sscanf (optarg, "%ld", &vconf->loop) != 1 || ( vconf->loop < MIN_LOOP ) ) 
	    v_error(vconf, LOG_CRIT, "Wrong sleeptime"); // exit
	  vconf->loop=vconf->loop*1000000;
	  break;
	case 'L':
	  if ( sscanf (optarg, "%ld", &vconf->loop) != 1 || ( vconf->loop < MIN_LOOP ) ) 
	    v_error(vconf, LOG_CRIT, "Wrong sleeptime"); // exit
	  break;
	case 'w':
	  if ( sscanf (optarg, "%d", &is_width) != 1 ) 
	    v_error(vconf, LOG_CRIT, "Wrong individual image width"); // exit
	  break;
	case 'H':
	  if ( sscanf (optarg, "%d", &is_height) != 1 ) 
	    v_error(vconf, LOG_CRIT, "Wrong individual image height"); // exit
	  break;
	case 'f':
	  vconf->out=optarg;
	  v_error(vconf, LOG_DEBUG, "Outputfile is %s", vconf->out);
	  break;
	case 'q':
	  if ( sscanf (optarg, "%d", &vconf->quality) != 1 || (vconf->quality<MIN_QUALITY)
	       || (vconf->quality>MAX_QUALITY) ) 
	    v_error(vconf, LOG_CRIT, "Wrong picture quality \"%d\"", vconf->quality); // exit
	  v_error(vconf, LOG_DEBUG, "Image quality is %d", vconf->quality);
	  break;
	case 'o':
	  if ( (!strcasecmp(optarg,"jpeg")) || (!strcasecmp(optarg,"jpg")) )
	    vconf->outformat=1;
	  else if ( !strcasecmp(optarg,"png") )
	    vconf->outformat=2;
	  else if ( !strcasecmp(optarg,"ppm") )
	    vconf->outformat=3;
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
	  } else if ( !strcasecmp(optarg,"svga") ) {
	    vconf->win.width  = 800;
	    vconf->win.height = 600;
	  } else if ( !strcasecmp(optarg,"xga") ) {
	    vconf->win.width  = 1024;
	    vconf->win.height = 768;
	  } else if ( !strcasecmp(optarg,"sxga") ) {
	    vconf->win.width  = 1280;
	    vconf->win.height = 1024;
	  } else if ( !strcasecmp(optarg,"uxga") ) {
	    vconf->win.width  = 1600;
	    vconf->win.height = 1200;
	  } else
	    v_error(vconf, LOG_CRIT, "Wrong imagesize specified"); // exit
	  break;
	case 'd':
	  vconf->in = optarg;
	  v_error(vconf, LOG_DEBUG, "Input device set to %s", vconf->in);
	  break;
#ifdef LIBTTF
	case 't':
	  vconf->font = optarg;
	  vconf->use_ts=TRUE;
	  break;
	case 'T':
	  if ( sscanf(optarg, "%d", &vconf->font_size) != 1 || vconf->font_size < MIN_FONTSIZE
	       || vconf->font_size > MAX_FONTSIZE ) 
	    v_error(vconf, LOG_CRIT, "Wrong fontsize (min. %d, max %d)", MIN_FONTSIZE, MAX_FONTSIZE); // exit
	  vconf->use_ts=TRUE;
	  break;
	case 'p':
	  vconf->timestamp = optarg;
	  vconf->use_ts = TRUE;
	  break;
	case 'e':
	  vconf->use_ts=TRUE;
	  break;
	case 'a':
	  if ( sscanf (optarg, "%d", &vconf->align) != 1 || vconf->align < MIN_ALIGN
	       || vconf->align>MAX_ALIGN ) 
	    v_error(vconf, LOG_CRIT, "Wrong timestamp alignment"); // exit
	  vconf->use_ts=TRUE;
	  break;
	case 'm':
	  if ( sscanf (optarg, "%d", &vconf->blend) != 1 || vconf->blend > MAX_BLEND
	       || vconf->blend < MIN_BLEND ) 
	    v_error(vconf, LOG_CRIT, "Wrong blend value"); // exit
	  vconf->use_ts=TRUE;
	  break;
	case 'B':
	  if ( sscanf (optarg, "%d", &vconf->border) != 1 || vconf->border > MAX_BORDER
	       || vconf->border < MIN_BORDER ) 
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
	  if ( sscanf (optarg, "%d", &vconf->debug) != 1 || vconf->debug > MAX_DEBUG
	       || vconf->debug < MIN_DEBUG )
	    v_error(vconf, LOG_CRIT, "Wrong debuglevel value");
	  break;
	case 'g':
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
	case 'n':
	  vconf->usetmpout=FALSE;
	  break;
	default:
	  usage (argv[0]);
	  break;
	}
    }

  if ( (is_width != 0) && (is_height != 0) ) {
    vconf->win.width=is_width;
    vconf->win.height=is_height;
    v_error(vconf, LOG_WARNING, "Imagesize set to unchecked individual size!");
  }
  
  if ( (strcasecmp(vconf->out, DEFAULT_OUTPUT)) && vconf->usetmpout ) {
    vconf->tmpout = malloc(strlen(vconf->out)+6);
    sprintf(vconf->tmpout, "%s.tmp", vconf->out);
  }
  
  check_files(vconf);

  v_error(vconf, LOG_DEBUG, "Read all arguments, starting up...");

  vconf->init_done=TRUE;
  return vconf;
}


int get_int(char *value) {
  int tmp;
  if ( sscanf(value, "%d", &tmp) != 1 )
    return -1;
  return tmp;
}


char *get_str(char *value) {
  char *tmp;
  if ( strlen(value)<1 )
    return NULL;
  tmp=malloc(strlen(value)+1);
  tmp=strcpy(tmp, value);
  return tmp;
}


int get_bool(char *value) {
  if ( !(strcasecmp(value, "oN")) || !(strcasecmp(value, "Yes")))
    return 1;
  else if ( !(strcasecmp(value, "Off")) || !(strcasecmp(value, "No")) )
    return 0;
  return -1;
}


int get_format(char *value) {
  int tmp;
  if ( !(strcasecmp(value, "JPEG")) || !(strcasecmp(value,"JPG")) )
    tmp=1;
  else if ( !(strcasecmp(value, "PNG")) )
    tmp=2;
  else if ( !(strcasecmp(value, "PPM")) )
    tmp=3;
  else
    tmp=-1;
  return tmp;
}


int get_position(char *value) {
  int tmp;
  if ( (!(strcasecmp(value, "UL"))) || (!(strcasecmp(value, "UpperLeft"))) )
    tmp=0;
  else if ( (!(strcasecmp(value, "UR"))) || (!(strcasecmp(value, "UpperRight"))) )
    tmp=1;
  else if ( (!(strcasecmp(value, "LL"))) || (!(strcasecmp(value, "LowerLeft"))) )
    tmp=2;
  else if ( (!(strcasecmp(value, "LR"))) || (!(strcasecmp(value, "LowerRight"))) )
    tmp=3;
  else if ( (!(strcasecmp(value, "UC"))) || (!(strcasecmp(value, "UpperCenter"))) )
    tmp=4;
  else if ( (!(strcasecmp(value, "LC"))) || (!(strcasecmp(value, "LowerCenter"))) )
    tmp=5;
  else
    tmp=-1;
  return tmp;
}


int decode_size(char *value) {
  int tmp;
  if ( !(strcasecmp(value, "sqcif")) )
    tmp=8;
  else if ( !(strcasecmp(value, "qsif")) )
    tmp=10;
  else if ( !(strcasecmp(value, "qcif")) )
    tmp=11;
  else if ( !(strcasecmp(value, "sif")) )
    tmp=20;
  else if ( !(strcasecmp(value, "cif")) )
    tmp=22;
  else if ( !(strcasecmp(value, "vga")) )
    tmp=40;
  else if ( !(strcasecmp(value, "svga")) )
    tmp=50;
  else if ( !(strcasecmp(value, "xga")) )
    tmp=64;
  else if ( !(strcasecmp(value, "sxga")) )
    tmp=80;
  else if ( !(strcasecmp(value, "uxga")) )
    tmp=100;
 else
    tmp=0;
  return tmp;
}


int get_width(char *value) {
  return (16 * decode_size(value));
}


int get_height(char *value) {
  if ( decode_size(value) == 11 )
    return 144;
  else if ( decode_size(value) == 22 )
    return 288;
  else if ( decode_size(value) == 80 )
    return 1024;
  return (12 * decode_size(value));
}


struct vconfig *parse_config(struct vconfig *vconf, char *path){

  int           n=0, tmp, is_width=0, is_height=0;
  char          line[MAX_LINE];
  char          *option=NULL, *value=NULL, *p=NULL;
  FILE          *fd;

  /* Check for config file */

  if (! (fd = fopen(path, "r") ) ) {
    v_error(vconf, LOG_WARNING, "Could not open configfile %s, ignoring", path);
    return vconf;
  }

  /* read every line */
  v_error(vconf, LOG_DEBUG, "Starting to read arguments from file %s", path);

  while (fgets(line, sizeof(line), fd)) {
    n++;
    p=line;
    /* hide comments 
    if ((p=strchr(line, '#')))
      *p='\0';
    if ((p=strchr(line, ';')))
      *p='\0';
    p=line;

     Check options */

    if ( (option=strtok(line," \t\n")) && (value=p+strlen(option)) ) {
      if ( (p=strchr(option, ';')) )
	continue;
      if ( !strcasecmp(option, "ImageQuality")) {
	if ( (MIN_QUALITY > (vconf->quality=get_int((value=strtok(NULL, " \t\n"))))) ||
	      (vconf->quality > MAX_QUALITY) ) 
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)", 
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "VideoDevice") ) {
	vconf->in=get_str((value=strtok(NULL, " \t\n")));
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "OutputFile") ) {
	vconf->out=get_str((value=strtok(NULL, " \t\n")));
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "ImageSize") ) {
	if ( (vconf->win.width=get_width((value=strtok(NULL, " \t\n")))) == 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %d", option, vconf->win.width);
	if ( (vconf->win.height=get_height(value)) == 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %d", option, vconf->win.height);
      }
      else if ( !strcasecmp(option, "OutputFormat") ) {
	if ( (vconf->outformat=get_format((value=strtok(NULL, " \t\n")))) < 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %d", option, vconf->outformat);
      }
      else if ( !strcasecmp(option, "OpenOnce") ) {
	if ( (tmp=get_bool((value=strtok(NULL, " \t\n")))) < 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->openonce=TRUE;
	else
	  vconf->openonce=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "Brightness") ) {
	if ( (tmp=get_bool((value=strtok(NULL, " \t\n")))) < 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->brightness=TRUE;
	else
	  vconf->brightness=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "SwitchColor") ) {
	if ((tmp=get_bool((value=strtok(NULL, " \t\n")))) < 0)
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->switch_bgr=TRUE;
	else
	  vconf->switch_bgr=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "SetImageSize") ) {
	if ((tmp=get_bool((value=strtok(NULL, " \t\n")))) < 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->windowsize=TRUE;
	else
	  vconf->windowsize=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "Daemon") ) {
	if ( ((vconf->loop=get_int((value=strtok(NULL, " \t\n")))) < MIN_LOOP) && (vconf->loop != 0 ) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "UseTimestamp") ) {
	if ( (tmp=get_bool((value=strtok(NULL, " \t\n")))) < 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->use_ts=TRUE;
	else
	  vconf->use_ts=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "ImageWidth") ) {
	if ( (is_width=get_int((value=strtok(NULL, " \t\n")))) ) {
	  v_error(vconf, LOG_CRIT, "Wrong value '%s' for %s (line %d, %s)",
		  value, option, n, path);
	  is_width=0;
	}
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "ImageHeight") ) {
	if ( (is_height=get_int((value=strtok(NULL, " \t\n")))) ) {
	  v_error(vconf, LOG_CRIT, "Wrong value '%s' for %s (line %d, %s)",
		  value, option, n, path);
	  is_height=0;
	}
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "DebugLevel") ) {
	if ( (MIN_DEBUG > (vconf->debug=get_int((value=strtok(NULL, " \t\n"))))) ||
	     (vconf->debug > MAX_DEBUG))
	  v_error(vconf, LOG_CRIT, "Wrong value '%s' for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "ForcePalette") ) {
	if ( (MIN_PALETTE > (vconf->forcepal=get_int((value=strtok(NULL, " \t\n"))))) ||
	     (vconf->forcepal > MAX_PALETTE) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "UseTmpOut") ) {
	if ((tmp=get_bool((value=strtok(NULL, " \t\n")))) < 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->usetmpout=TRUE;
	else
	  vconf->usetmpout=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }

#ifdef LIBTTF
      else if ( !strcasecmp(option, "FontFile") ) {
	vconf->font=get_str((value=strtok(NULL, " \t\n")));
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "TimeStamp") ) {
	if ( !(vconf->timestamp=get_str((value=strtok(NULL, "\"\t\n")))) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "FontSize") ) {
	if ( (MIN_FONTSIZE > (vconf->font_size=get_int((value=strtok(NULL, " \t\n"))))) ||
	     (vconf->font_size > MAX_FONTSIZE) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "BorderSize") ) {
	if ( (MIN_BORDER > (vconf->border=get_int((value=strtok(NULL, " \t\n"))))) ||
	     (vconf->border > MAX_BORDER) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "Position") ) {
	if ( (vconf->align=get_position((value=strtok(NULL, " \t\n")))) < 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "Blend") ) {
	if ( (MIN_BLEND > (vconf->blend=get_int((value=strtok(NULL, " \t\n"))))) ||
	     (vconf->blend > MAX_BLEND) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
#endif
#ifdef LIBFTP
/* ftp */      
      else if ( !strcasecmp(option, "EnableFtp") ) {
	if ( (tmp=get_bool((value=strtok(NULL, " \t\n")))) < 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->ftp.enable=TRUE;
	else
	  vconf->ftp.enable=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "KeepAlive") ) {
	if ( (tmp=get_bool((value=strtok(NULL, " \t\n")))) < 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->ftp.keepalive=TRUE;
	else
	  vconf->ftp.keepalive=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "RemoteHost") ) {
	if ( !(vconf->ftp.remoteHost=get_str((value=strtok(NULL, "\"\t\n")))) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "Username") ) {
	if ( !(vconf->ftp.username=get_str((value=strtok(NULL, "\"\t\n")))) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "Password") ) {
	if ( !(vconf->ftp.password=get_str((value=strtok(NULL, "\"\t\n")))) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else 
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }      
      else if ( !strcasecmp(option, "RemoteDir") ) {
	if ( !(vconf->ftp.remoteDir=get_str((value=strtok(NULL, "\"\t\n")))) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }      
      else if ( !strcasecmp(option, "RemoteImageName") ) {
	if ( !(vconf->ftp.remoteImageName=get_str((value=strtok(NULL, "\"\t\n")))) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }      
/* end ftp */
#endif
    }
    else
      v_error(vconf, LOG_CRIT, "Unknown Option %s (value %s, line %d, %s)", option, value, n, path);
  }
  fclose(fd);

  if ( (is_width!=0) && (is_height!=0) ) {
    vconf->win.width = is_width;
    vconf->win.height = is_height;
    v_error(vconf, LOG_WARNING, "Imagesize set to unchecked individual size!\n");
  }
  return(vconf);
}
