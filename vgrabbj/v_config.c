/* Simple Video4Linux image grabber. Made for my Philips Vesta Pro
 * 
 * Copyright (C) 2001, 2002 Jens Gecius, Hannover, Germany
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


#include "v_config.h"

/* Usage information  */

void usage (char *pname) 
{
  fprintf(stderr,
	  "%s, Version %s\n"
	  "Usage: %s [options]\n"
	  " -h                This screen\n"
	  " -c <filename>     parse <filename> as config file\n"
	  " -l <seconds>      Daemonize & sleep <seconds> (min. 1!) between images\n"
	  " -L <microseconds> Daemonize & sleep <microseconds> between images\n"
	  " -a                Switch vgrabbj's auto brightness adjustment (default: off)\n"
	  "                   You might need to set -F 4, too, if it doesn't work\n"
	  " -q <quality>      Quality setting (%d-%d, default: %d), JPEG only\n"
	  " -i <sqcif|qsif|qcif|sif|cif|vga|svga|xga|sxga|uxga>\n"
	  "                   Sets the imagesize of input device to sqcif=128x96,\n"
	  "                   qsif=160x120, qcif=176x144, sif=320x240, cif=352x288,\n"
	  "                   vga=640x480, svga=800x600, xga=1024x768, sxga=1280x1024, or\n"
	  "                   uxga=1600x1200 (default: %dx%d)\n"
	  " -w                Set imagesize to individual width (needs -H)\n"
	  " -H                Set imagesize to individual height (needs -W)\n"
	  "                   Be careful! These settings supersede any other setting of\n"
	  "                   the imagesize and may damage your hardware!!\n"
	  "                   The values are NOT checked to be valid!!!!\n"
	  " -o <jpg|png|ppm>  Output format (default: jpg)\n"
	  " -f <filename>     Write to <filename> (default: %s)\n"
	  " -d <device>       Read from <device> as input (default: %s)\n"
	  " -C                Open device permanently instead of opening for each image\n"
	  " -g                Disable setting of image-size, necessary for certain cams\n"
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
	  "                   Value 4 refers to RGB24, you need this for the brightness\n"
	  "                   adjustment.\n"
	  " -z <value>        Discards <value> frames before the actual picture is taken\n"
	  "                   and written to the output. Only works in mmap'ed mode.\n"
	  " -A <path+file>    Path and filename to write archive images. See man strftime\n"
	  "                   for possible tokens.\n"
	  " -M <value>        Maximum number of images to keep in archive.\n"
	  " -E <value>        Take a snapshot for the archive each <value> image.\n"
	  " -R                Swap left/right like a mirror.\n"
	  " -U                Swap top/bottom like a mirror.\n"
	  " -G                Do not use mmap'ed memory - needed only for certain cams\n"
	  "\n"
	  "Example: %s -l 5 -f /usr/local/image.jpg\n"
	  "         Would write a single jpeg-image to image.jpg approx. every five seconds\n"
	  "\n"
	  "The video stream has to be one of RGB24, RGB32, YUV420, YUV420P or YUYV.\n",
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
#if DEBUGGING
void debug_vconf(struct vconfig *vconf) {
  fprintf(stderr, "Values of char* in vconf:\n"
	  "vconf->in                 :%s*\nvconf->out                :%s*\n"
	  "vconf->timestamp          :%s*\nvconf->font               :%s*\n"
	  "vconf->ftp.remoteHost     :%s*\nvconf->ftp.remoteDir      :%s*\n"
	  "vconf->ftp.remoteImageName:%s*\nvconf->ftp.username       :%s*\n"
	  "vconf->ftp.password       :%s*\n", vconf->in, vconf->out, vconf->timestamp,
	  vconf->font, vconf->ftp.remoteHost, vconf->ftp.remoteDir,
	  vconf->ftp.remoteImageName, vconf->ftp.username, vconf->ftp.password);
}  
#endif

/*
struct v_options *init_conf_list(struct v_options *conf)
{
  struct v_options *list;
  struct v_options *list_start;
  int i;

  list=malloc(sizeof(list));
  list_start=list;
  for (i=0; l_opt[i].name||l_opt[i].shortname; i++) {
    list.name=l_opt[i].name;
    list.short_name=l_opt[i].short_name;
    list.has_arg=l_opt[i].has_arg;
    list.var=decode_options(struct vconfig *vconf, char *option, char *value, int o, int n)
    //decode default options!!!    list.var=l_opt[i].default;
    list.var_type=l_opt[i].var_type;
    list.min_value=l_opt[i].min_value;
    list.max_value=l_opt[i].max_value;
    list.max_length=l_opt[i].max_length;
    list.default=l_opt[i].default;
    list.next=malloc(sizeof(list));
    list=list.next;
  }
  free(list.next);
  list.next=list_start;

  return (list_start);
}

    Some development, not release-ready...
*/

struct vconfig *init_defaults(struct vconfig *vconf) {
  int idx = 0;
  /* Set defaults */
  vconf->debug      = LOGLEVEL;
  vconf->quality    = DEFAULT_QUALITY;
  vconf->in         = strcpy(malloc(strlen(DEFAULT_VIDEO_DEV)+1),DEFAULT_VIDEO_DEV);
  vconf->out        = strcpy(malloc(strlen(DEFAULT_OUTPUT)+1),DEFAULT_OUTPUT);
  vconf->conf_file  = strcpy(malloc(strlen(DEFAULT_CONFIG)+1),DEFAULT_CONFIG);
  vconf->win.height = DEFAULT_HEIGHT;
  vconf->win.width  = DEFAULT_WIDTH;
  vconf->outformat  = DEFAULT_OUTFORMAT;
  vconf->autobrightness = DEFAULT_BRIGHTNESS;
  vconf->switch_bgr = FALSE;
  vconf->windowsize = TRUE;
  vconf->loop       = 0;
  vconf->use_ts     = FALSE;
  vconf->init_done  = FALSE;
  vconf->err_count  = 0;
  vconf->dev        = 0;
  vconf->forcepal   = 0;
  vconf->discard    = 0;
  vconf->usemmap    = TRUE;
  vconf->openonce   = TRUE;
  vconf->usetmpout  = TRUE;
  vconf->swaprl     = FALSE;
  vconf->swaptb     = FALSE;
  vconf->nousemmap  = FALSE;
  vconf->tmpout     = NULL;
  vconf->buffer     = NULL;
  vconf->o_buffer   = NULL;
  l_opt[idx++].var  = &(int)vconf->debug;
  l_opt[idx++].var  = &(long int)vconf->loop;
  l_opt[idx++].var  = &(long int)vconf->loop;
  l_opt[idx++].var  = &vconf->autobrightness;
  l_opt[idx++].var  = &(int)vconf->quality;
  idx              += 1; /* Image Size */
  l_opt[idx++].var  = &vconf->win.width;
  l_opt[idx++].var  = &vconf->win.height;
  l_opt[idx++].var  = &(int)vconf->outformat;
  l_opt[idx++].var  = (char *)vconf->out;
  l_opt[idx++].var  = (char *)vconf->in;
  l_opt[idx++].var  = &vconf->openonce;
  l_opt[idx++].var  = &vconf->switch_bgr;
  l_opt[idx++].var  = &vconf->windowsize;
  l_opt[idx++].var  = &(int)vconf->discard;
  l_opt[idx++].var  = &(int)vconf->forcepal;
  l_opt[idx++].var  = &vconf->usetmpout;
  l_opt[idx++].var  = &vconf->use_ts;
#ifdef LIBTTF
  vconf->ttinit     = NULL;
  vconf->font       = strcpy(malloc(strlen(DEFAULT_FONT)+1),DEFAULT_FONT);
  vconf->timestamp  = strcpy(malloc(strlen(DEFAULT_TIMESTAMP)+1),DEFAULT_TIMESTAMP);
  vconf->font_size  = DEFAULT_FONTSIZE;
  vconf->border     = DEFAULT_BORDER;
  vconf->align      = DEFAULT_ALIGN;
  vconf->blend      = DEFAULT_BLEND;
  l_opt[idx++].var  = (char *)vconf->font;
  l_opt[idx++].var  = (char *)vconf->timestamp;
  l_opt[idx++].var  = &(int)vconf->font_size;
  l_opt[idx++].var  = &(int)vconf->align;
  l_opt[idx++].var  = &(int)vconf->blend;
  l_opt[idx++].var  = &(int)vconf->border;
#endif
#ifdef LIBFTP
  vconf->ftp.enable          = FALSE;
  vconf->ftp.state           = 0;
  vconf->ftp.keepalive       = FALSE;
  vconf->ftp.remoteHost      = NULL;
  vconf->ftp.remoteDir       = NULL;
  vconf->ftp.remoteImageName = NULL;
  vconf->ftp.username        = NULL;
  vconf->ftp.password        = NULL;
  vconf->ftp.tryharder       = 0;
  l_opt[idx++].var  = &vconf->ftp.enable;
  l_opt[idx++].var  = (char *)vconf->ftp.remoteHost;
  l_opt[idx++].var  = (char *)vconf->ftp.remoteImageName;
  l_opt[idx++].var  = (char *)vconf->ftp.username;
  l_opt[idx++].var  = (char *)vconf->ftp.password;
  l_opt[idx++].var  = &vconf->ftp.keepalive;
  l_opt[idx++].var  = &(unsigned int)vconf->ftp.tryharder;
  l_opt[idx++].var  = (char *)vconf->ftp.remoteDir;
#endif
  idx              += 4;
  vconf->hue = -1;
  l_opt[idx++].var  = &(int)vconf->hue;
  vconf->brightness = -1;
  l_opt[idx++].var  = &(int)vconf->brightness;
  vconf->contrast = -1;
  l_opt[idx++].var  = &(int)vconf->contrast;
  vconf->colour = -1;
  l_opt[idx++].var  = &(int)vconf->colour;
  vconf->whiteness = -1;
  l_opt[idx++].var  = &(int)vconf->whiteness;
  vconf->archive    = NULL;
  l_opt[idx++].var  = (char *)vconf->archive;
  vconf->arch       = malloc(sizeof(struct s_arch));
  vconf->arch->filename = NULL;
  vconf->arch->next = NULL;
  vconf->archiveeach = 0;
  vconf->archivemax = 0;
  l_opt[idx++].var  = &vconf->archiveeach;
  l_opt[idx++].var  = &vconf->archivemax;
  l_opt[idx++].var  = &vconf->swaprl;
  l_opt[idx++].var  = &vconf->nousemmap;
  l_opt[idx++].var  = &vconf->swaptb;
  if ( idx != sizeof(l_opt)/sizeof(l_opt[0])-1 ) {
    v_error(vconf, LOG_CRIT, "Bug in l_opt - contact developer with full debug details.");
  }
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
    v_error(vconf, LOG_CRIT, "Can't open \"%s\" as VideoDevice!", vconf->in);
  } else {
    close(dev);
  }
  
  if ( !(x=fopen(vconf->out, "w+"))) {
    v_error(vconf, LOG_CRIT, "Can't open \"%s\" as OutputFile", vconf->out);
  } else {
    fclose(x);
  }
#ifdef LIBTTF
  if (vconf->use_ts) {
    if (!(x=fopen(vconf->font, "r"))) {
      v_error(vconf, LOG_CRIT, "Can't open \"%s\" as FontFile!", vconf->font);
    } else {
      fclose(x);
    }
  }
#endif
  
}

#ifdef LIBFTP
void check_ftpconf(struct vconfig *vconf)
{
  /* This function checks for proper settings if ftp-enable is set to on
     In case any essential setting is not set to a proper value, ftp-enable
     will be disabled automagically                                          */
  if (vconf->ftp.enable) {
    if ( (!vconf->ftp.remoteHost) | (!vconf->ftp.remoteImageName) |
	 (!vconf->ftp.username) )
      {
	v_error(vconf, LOG_ERR, "ftp settings do not meet minimum requirements");
	v_error(vconf, LOG_ERR, "check remoteHost, remoteImageName, and Username in config");
	v_error(vconf, LOG_ERR, "ftp-upload disabled");
	vconf->ftp.enable=FALSE;
      }
  }
}
#endif

/* Initialize/Reinitialize pointers for archive-filenames */

struct s_arch *init_archive(struct vconfig *vconf, struct s_arch *archive, int count)
{
  archive=realloc(archive, sizeof(struct s_arch));
  archive->filename=NULL;
  archive->next=NULL;
  if ( count-->1 )
    archive->next=init_archive(vconf, archive->next, count);
  else
    archive->next=vconf->arch;
  return archive;
}

/* Check if palette is supported by v4l device */

int try_palette(struct vconfig *vconf, int palette, int dev)
{
  v_error(vconf, LOG_INFO, "Trying palette %s", plist[palette].name);
  vconf->vpic.palette=palette;

  if (ioctl(dev, VIDIOCSPICT, &vconf->vpic) < 0) {
    v_error(vconf, LOG_WARNING, "Unable to set palette");
    return 0;
  }
  if (ioctl(dev, VIDIOCGPICT, &vconf->vpic) < 0) {
    v_error(vconf, LOG_WARNING, "Unable to get palette info");
    return 0;
  }
  if (vconf->vpic.palette == palette)
    return palette;
  return 0;
}

/* Check and set v4l device */

struct vconfig *check_device(struct vconfig *vconf) {

  struct video_window twin;

  open_device(vconf);

  v_error(vconf, LOG_INFO, "Checking settings of device %s", vconf->in);
  
  while (ioctl(vconf->dev, VIDIOCGCAP, &vconf->vcap) < 0)
    v_error(vconf, LOG_ERR, "Problem getting video capabilities");
  if ( (vconf->vcap.maxwidth < vconf->win.width) ||
       (vconf->vcap.minwidth > vconf->win.width) ||
       (vconf->vcap.maxheight < vconf->win.height) ||
       (vconf->vcap.minheight > vconf->win.height) )
    v_error(vconf, LOG_CRIT, "Device doesn't support width/height");
  while (ioctl(vconf->dev, VIDIOCGWIN, &twin))
    v_error(vconf, LOG_ERR, "Problem getting window information");
  vconf->win.flags=twin.flags;
  vconf->win.x=twin.x;
  vconf->win.y=twin.y;
  vconf->win.chromakey=twin.chromakey;
  if (vconf->windowsize)
    while (ioctl(vconf->dev, VIDIOCSWIN, &vconf->win) )
      v_error(vconf, LOG_ERR, "Problem setting window size");
  while (ioctl(vconf->dev, VIDIOCGWIN, &vconf->win) <0)
    v_error(vconf, LOG_ERR, "Problem getting window size");
  while (ioctl(vconf->dev, VIDIOCGPICT, &vconf->vpic) < 0)
    v_error(vconf, LOG_ERR, "Problem getting picture properties");

  /* HERE we actually TRY to get a palette the device delivers.
   * PROBLEM is that V4L does NOT provide a function to query available
   * palettes for the device! Hence, this util has to rely on try-and-error
   * to find a palette suitable.
   * Currently, only palettes below are supported directly.
   * If it is a different one, it has to be one of those tested below - simply because
   * I have no other conversion routines on hand.
   * If this prog does not work with your device, please blame someone else for
   * an insufficient V4L implementation.
   * Sorry for the inconvenience! */

  if (vconf->forcepal)
    if ( (vconf->vpic.palette=try_palette(vconf, vconf->forcepal, vconf->dev)) )
      v_error(vconf, LOG_INFO, "Set palette successfully to %s", plist[vconf->vpic.palette].name);
  
  switch(vconf->vpic.palette) {
  case VIDEO_PALETTE_RGB24:
  case VIDEO_PALETTE_YUV420P:
  case VIDEO_PALETTE_YUV420:
  case VIDEO_PALETTE_YUYV:
  case VIDEO_PALETTE_YUV422: /* equal to YUYV with my cam */
  case VIDEO_PALETTE_RGB32:
    break;
  default:
    if ( (vconf->vpic.palette=try_palette(vconf, VIDEO_PALETTE_RGB24, vconf->dev))  ||
	 (vconf->vpic.palette=try_palette(vconf, VIDEO_PALETTE_RGB32, vconf->dev))  ||
	 (vconf->vpic.palette=try_palette(vconf, VIDEO_PALETTE_YUYV, vconf->dev))   ||
	 (vconf->vpic.palette=try_palette(vconf, VIDEO_PALETTE_YUV420, vconf->dev)) ||
	 (vconf->vpic.palette=try_palette(vconf, VIDEO_PALETTE_YUV420P, vconf->dev)) )
      ;
    else
      v_error(vconf, LOG_CRIT, "Unable to set supported video-palette");
    break;
  }
    
  v_error(vconf, LOG_DEBUG, "Set palette successfully to %s", plist[vconf->vpic.palette].name);

  if ( (ioctl(vconf->dev, VIDIOCGMBUF, &vconf->vbuf) < 0) || 
       ((vconf->autobrightness) && 
	(vconf->vpic.palette==VIDEO_PALETTE_RGB24)) ||
       (vconf->nousemmap) )
    vconf->usemmap=FALSE;

  set_picture_parms(vconf);

  if (!vconf->openonce)
    close_device(vconf);

  return vconf;
}


void v_update_ptr(struct vconfig *vconf) {
  vconf->in=(char *)l_opt[10].var;
  vconf->out=(char *)l_opt[9].var;
#ifdef LIBTTF
  vconf->timestamp=(char *)l_opt[19].var;
  vconf->font=(char *)l_opt[18].var;
#endif
#ifdef LIBFTP
  vconf->ftp.remoteHost=(char *)l_opt[25].var;
  vconf->ftp.remoteImageName=(char *)l_opt[26].var;
  vconf->ftp.username=(char *)l_opt[27].var;
  vconf->ftp.password=(char *)l_opt[28].var;
  vconf->ftp.remoteDir=(char *)l_opt[31].var;
#endif
  vconf->archive=(char *)l_opt[36].var;
  v_error(vconf, LOG_DEBUG, "Updated pointers to new allocated memory.");
}

/* Decode options */

void decode_options(struct vconfig *vconf, char *option, char *value, int o, int n) {
  int i;
  
  for (i=0;l_opt[i].name || l_opt[i].short_name; i++) {
    if ( (l_opt[i].name && !strcasecmp(option, l_opt[i].name) && !o) ||  
	 (l_opt[i].short_name && o==*l_opt[i].short_name) ) {
      switch (l_opt[i].var_type) {
      case opt_int:
	*(int *)l_opt[i].var=(int)check_minmax(vconf, value, get_int(value), n, l_opt[i]);
	break;
      case opt_longint:
	*(long *)l_opt[i].var=check_minmax(vconf, value, get_int(value), n, l_opt[i]);
	break;
      case opt_int_s:
	*(long *)l_opt[i].var=check_minmax(vconf, value, get_int(value), n, l_opt[i])*1000000;
	break;
      case opt_bool:
	*(boolean *)l_opt[i].var=check_minmax(vconf, value, 
					      n?get_bool(value):!*(boolean *)l_opt[i].var,
					      n, l_opt[i]) ? TRUE : FALSE;
	break;
      case opt_charptr:
	(int *)l_opt[i].var=(int)check_maxlen(vconf, get_str(value, (char *)l_opt[i].var),
					      l_opt[i], n);
	break;
      case opt_format:
	*(int *)l_opt[i].var=(int)check_minmax(vconf, value, get_format(value), n, l_opt[i]);
	break;
      case opt_size:
	vconf->win.width= (int)check_minmax(vconf, value, get_width(value), n, l_opt[i]);
	vconf->win.height=(int)check_minmax(vconf, value, get_height(value),n, l_opt[i]);
	break;
      case opt_position:
	*(int *)l_opt[i].var=(int)check_minmax(vconf, value, get_position(value), n, l_opt[i]);
	break;
      case opt_conf:
	if (vconf->conf_file)
	  v_error(vconf, LOG_DEBUG, "Overwriting old conf (%s) settings with new conf (%s)",
		  vconf->conf_file, optarg);
	vconf->conf_file=get_str(optarg, vconf->conf_file);
	parse_config(vconf);
	break;
      case opt_version:
	fprintf(stderr, "%s %s\n", option, VERSION);
	exit(0);
	break;
      case opt_help:
	usage (option);
	break;
      case opt_setting:
	vconf->in=get_str(optarg, vconf->in);
	open_device(vconf);
	close_device(vconf);
	show_capabilities(vconf->in, option);
	break;
      default:
	v_error(vconf, LOG_WARNING, "Unknown option %s (line %d), shouldn't happen", value, n);
	break;
      }
    }      
  }
}


/* Parse a config-file for options */

struct vconfig *parse_config(struct vconfig *vconf){

  int           n=0;//, i=0;
  char          line[MAX_LINE];
  char          *option=NULL, *value=NULL, *p=NULL;
  FILE          *fd;

  /* Check for config file */

  if (! (fd = fopen(vconf->conf_file, "r") ) ) {
    v_error(vconf, LOG_WARNING, "Could not open configfile %s, ignoring", vconf->conf_file);
    return vconf;
  }

  /* read every line */
  v_error(vconf, LOG_INFO, "Starting to read arguments from file %s", vconf->conf_file);

  while (fgets(line, sizeof(line), fd)) {
    n++;
    p=line;
    /* hide comments */
    line[strcspn(line, "#")]='\0';
    line[strcspn(line, ";")]='\0';
    line[strcspn(line, "\n")]='\0';

    option=strcpy(malloc(strlen(line)+1), line);

    if ( (strlen(option)>1) && (p=strpbrk(option," \t")) ) {
      *p='\0';
      p++;
      /* Strip whitespace */
      value=strip_white(strcpy(malloc(strlen(p)+1),p));

      decode_options(vconf, option, value, 0, n);

    }
    free_ptr(option);
  }
  v_update_ptr(vconf);
  fclose(fd);
#if DEBUGGING
  debug_vconf(vconf);
#endif
  v_error(vconf, LOG_INFO, "Done parsing config file %s", vconf->conf_file);

  return(vconf);
}

char *build_opt(struct v_options l_opt[]) {
  int i, n=0;
  char string[255];
  for (i=0; l_opt[i].name || l_opt[i].short_name; i++ ) {
    if (l_opt[i].short_name) {
      string[n++]=*l_opt[i].short_name;
      if (l_opt[i].has_arg==req)
	string[n++]=':';
    }
  }
  string[n]='\0';
  return (strcpy(malloc(strlen(string)+1), string));
}

/* Parse the commandline */

struct vconfig *parse_commandline(struct vconfig *vconf, int argc, char *argv[]) {
  int n;
  //, i;
  char *opt_str;
  
  opt_str=build_opt(l_opt);

  while ( (n = getopt (argc, argv, opt_str) ) !=EOF ) {
    decode_options(vconf, argv[0], optarg, n, 0);
  }
  free_ptr(opt_str);
  v_update_ptr(vconf);
  
  v_error(vconf, LOG_INFO, "Done parsing commandline");

  return vconf;
}

struct vconfig *v_init(struct vconfig *vconf, int argc, char *argv[]) {

  if (!vconf) {
    vconf=malloc(sizeof(*vconf));
    vconf=init_defaults(vconf);
  }
  else if (vconf->openonce) {
    free_mmap(vconf);
    close_device(vconf);
  }

  if (!vconf->init_done) {
    signal(SIGTERM,sigterm);
    signal(SIGHUP,sighup);
  }

  vconf=parse_config(vconf);

  if ( !vconf->init_done ) {
    vconf=parse_commandline(vconf, argc, argv);
  }

  if ( (strcasecmp(vconf->out, DEFAULT_OUTPUT)) && vconf->usetmpout ) {
    vconf->tmpout=free_ptr(vconf->tmpout);
    vconf->tmpout=malloc(strlen(vconf->out)+6);
    sprintf(vconf->tmpout, "%s.tmp", vconf->out);
    v_error(vconf, LOG_DEBUG, "Setting tmpout file to %s", vconf->tmpout);
  }
  else {
    vconf->usetmpout=FALSE;
  }
  
  check_files(vconf);

#ifdef LIBFTP
  check_ftpconf(vconf);
#endif

  vconf=check_device(vconf);

  /* re/initialize appropriate memory */
  
  if (vconf->init_done)
    v_error(vconf, LOG_DEBUG, "Reinitializing memory");
  else
    v_error(vconf, LOG_DEBUG, "Initializing memory");

  vconf->buffer=free_ptr(vconf->buffer);
  vconf->o_buffer=free_ptr(vconf->o_buffer);

  vconf->buffer=malloc(img_size(vconf, vconf->vpic.palette));  /* depending on palette */
  vconf->o_buffer=malloc(img_size(vconf, VIDEO_PALETTE_RGB24)); /* RGB24 (3 byte/pixel) */
  if (!vconf->buffer || !vconf->o_buffer) 
    v_error(vconf, LOG_CRIT, "Out of memory! Exiting...");

  if (vconf->archive && vconf->archivemax) {
    vconf->arch->next=init_archive(vconf, vconf->arch->next, vconf->archivemax-1);
    vconf->archivecount=vconf->archiveeach;
  }
  
  v_error(vconf, LOG_DEBUG, "Memory initialized, size: %d (in), %d (out)",
	  img_size(vconf, vconf->vpic.palette), img_size(vconf, VIDEO_PALETTE_RGB24));
  
  vconf->vmap.height=vconf->win.height;
  vconf->vmap.width=vconf->win.width;
  vconf->vmap.frame=0;
  vconf->vmap.format=vconf->vpic.palette;
  if (vconf->openonce) {
    init_mmap(vconf);
  }
#ifdef LIBTTF
  if ( vconf->init_done ) {
    if ( vconf->use_ts ) {
      Face_Done(vconf->ttinit->instance, vconf->ttinit->face);
    }
  } else {
    vconf->ttinit = malloc(sizeof(*vconf->ttinit));
    vconf->ttinit->properties = malloc(sizeof(*vconf->ttinit->properties));
    if ( !vconf->ttinit->properties || !vconf->ttinit ) {
      v_error(vconf, LOG_WARNING, "No memory for timestamp, disabled!");
      vconf->use_ts=FALSE;
    }
    else
      v_error(vconf, LOG_DEBUG, "Allocated memory for TT-engine");
  }
  if (vconf->use_ts && !vconf->init_done)
    if (TT_Init_FreeType(&vconf->ttinit->engine)) {
      v_error(vconf, LOG_WARNING, "Could not initialize Font-Engine, timestamp disabled");
      vconf->use_ts=FALSE;
    }
  if (vconf->use_ts) {
#if DEBUGGING
    debug_vconf(vconf);
#endif
    OpenFace(vconf);
  }
#endif

  return vconf;
}

