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

int signal_terminate=0;

void cleanup(struct vconfig *vconf, boolean clean) {

  if (vconf->openonce) {
    while ( close(vconf->dev) )
      v_error(vconf, LOG_ERR, "Error while closing %s", vconf->in); // exit
    
    v_error(vconf, LOG_DEBUG, "Device %s closed", vconf->in);
  }

#ifdef LIBTTF
  if ( vconf->use_ts ) {
    TT_Done_FreeType(vconf->ttinit->engine);
  }
  free(vconf->ttinit->properties);
  free(vconf->ttinit);
  v_error(vconf, LOG_DEBUG, "TrueType engine ended");
#endif
  free (vconf->buffer);
  free (vconf->o_buffer);
  v_error(vconf, LOG_DEBUG, "Image-buffers freed");
  
  if ( vconf->tmpout ) {
    free(vconf->tmpout);
  }
  free(vconf);
  if ( !clean) {
    syslog(LOG_CRIT, "exiting.");
    closelog();
    _exit(1);
  }
  v_error(vconf, LOG_DEBUG, "No daemon, exiting...");
}

/* Event handler for SIGHUP */
/* to re-read the configuration file */



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

/* Adjustment of brightness of picture  */

int get_brightness_adj(struct vconfig *vconf, int *brightness) 
{
  long i, tot = 0;
  long size = vconf->win.width * vconf->win.height;
  for ( i = 0; i < size * 3; i++ )
    tot += vconf->buffer[i];
  *brightness = (128 - tot/(size*3))/3;
  return !((tot/(size*3)) >= 126 && (tot/(size*3)) <= 130);
}


/* Turns RGB into BGR (or vice versa) */

unsigned char *switch_color(struct vconfig *vconf) {
  char a;
  long int y;
  for (y = 0; y < (vconf->win.width * vconf->win.height); y++) {
    memcpy(&a, vconf->buffer+(y*3),1);
    memcpy(vconf->buffer+(y*3), vconf->buffer+(y*3)+2, 1);
    memcpy(vconf->buffer+(y*3)+2, &a, 1);
  }
  return vconf->buffer;
}

/* Strips last byte of RGB32 to convert to RGB24 - breaks picture if alpha is used! */

unsigned char *conv_rgb32_rgb24(unsigned char *o_buffer, unsigned char *buffer,
				int width, int height) {
  long int y;
  for (y = 0; y < (width * height); y++) 
    memcpy(o_buffer+(y*3), buffer+(y*4), 3);
  return o_buffer;
}


/* Returns the actual byte-size of the image */

int img_size(struct vconfig *vconf, int palette) {
  return (plist[palette].mul * vconf->win.width * vconf->win.height) / plist[palette].div;
}

/* Read the image from device, adjust brightness, if wanted, for RGB24 */

unsigned char *read_image(struct vconfig *vconf, int size) {
  int f, newbright;
  int err_count, dev;
  struct video_mmap vmap;
  struct video_mbuf vbuf;
  //  struct video_channel vchan;
  char *map;

  v_error(vconf, LOG_DEBUG, "Palette to be used: %s (%d), size: %d",
	  plist[vconf->vpic.palette].name, vconf->vpic.palette, img_size(vconf, vconf->vpic.palette));

  // Opening input device
  if ( !vconf->openonce) {
    while ((dev=open(vconf->in, O_RDONLY)) < 0)
      v_error(vconf, LOG_ERR, "Problem opening input-device %s", vconf->in);
    
    v_error(vconf, LOG_DEBUG, "Device %s successfully opened", vconf->in);
  } else {
    dev=vconf->dev;
  }

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
      v_error(vconf, LOG_INFO, "Doing brightness adjustment");
      do {
	while (read(dev, vconf->buffer, size) < size)
	  v_error(vconf, LOG_ERR, "Error reading from %s", vconf->in);
	f = get_brightness_adj(vconf, &newbright);
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
      v_error(vconf, LOG_INFO, "Brightness adjusted");
    } else {
      v_error(vconf, LOG_DEBUG, "Using normal read for image grabbing");
      read(dev, vconf->buffer, size);
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

    vconf->buffer=memcpy(vconf->buffer, map, size);

    munmap(map, vbuf.size);
  }
  
  v_error(vconf, LOG_DEBUG, "Image successfully read");

  // Closing Input Device
  
  if ( !vconf->openonce ) {
    while ( close(dev) )
      v_error(vconf, LOG_ERR, "Error while closing %s", vconf->in); // exit
    
    v_error(vconf, LOG_DEBUG, "Device %s closed", vconf->in);
  }

  return vconf->buffer;
}

int daemonize(struct vconfig *vconf, char *progname) 
{
  openlog(progname, LOG_DEBUG, LOG_DAEMON);
  v_error(vconf, LOG_DEBUG, "Forking for daemon mode");
    
  switch( fork() ) 
    {
    case 0: // Child 
      v_error(vconf, LOG_DEBUG, "I'm the child process and are going to read images...");
      closelog();
      break;
    case -1: /* Error  */
      v_error(vconf, LOG_CRIT, "Can't fork, exiting..."); // exit
      break;
    default: /* Parent  */
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


unsigned char *conv_image(struct vconfig *vconf) {

  //  unsigned char *t_buffer;
  switch (vconf->vpic.palette) {
  case VIDEO_PALETTE_RGB24:
    vconf->o_buffer=memcpy(vconf->o_buffer, vconf->buffer, 
		    vconf->win.width * vconf->win.height * 3);
    v_error(vconf, LOG_DEBUG, "No conversion, we have RGB24");
    break;
   
  case VIDEO_PALETTE_RGB32:
    v_error(vconf, LOG_INFO, "Got RGB32, converting...");
   
    vconf->o_buffer=conv_rgb32_rgb24(vconf->o_buffer, vconf->buffer,
				     vconf->win.width, vconf->win.height);
    break;
   
  case VIDEO_PALETTE_YUV420P:
    v_error(vconf, LOG_INFO, "Got YUV420p, converting...");
   
    ccvt_420p_bgr24(vconf->win.width, vconf->win.height, vconf->buffer,
		    vconf->buffer + (vconf->win.width * vconf->win.height),
		    vconf->buffer + (vconf->win.width * vconf->win.height)+
		    (vconf->win.width * vconf->win.height / 4),
		    vconf->o_buffer);
    break;
   
  case VIDEO_PALETTE_YUV420:
    v_error(vconf, LOG_INFO, "Got YUV420, converting...");

    ////    t_buffer=malloc(img_size(vconf, vconf->vpic.palette));
   
    //      ccvt_420i_420p(vconf->win.width, vconf->win.height, buffer, t_buffer,
    //	     t_buffer + (vconf->win.width * vconf->win.height),
    //	     t_buffer + (vconf->win.width * vconf->win.height)+
    //	     (vconf->win.width * vconf->win.height / 4));
    //      ccvt_420p_bgr24(vconf->win.width, vconf->win.height, t_buffer,
    //	      t_buffer + (vconf->win.width * vconf->win.height),
    //	      t_buffer + (vconf->win.width * vconf->win.height)+
    //	      (vconf->win.width * vconf->win.height / 4),
    //	      o_buffer);
    //	      free(t_buffer);
    // As of now the direct conversion does not work properly.
    // Therefore, I use a very slow but working solution...
   
    ccvt_420i_rgb24(vconf->win.width, vconf->win.height, vconf->buffer, vconf->o_buffer);
      
    break;
  case VIDEO_PALETTE_YUYV:
  case VIDEO_PALETTE_YUV422:
    v_error(vconf, LOG_INFO, "Got YUYV, converting...");
      
    ccvt_yuyv_bgr24(vconf->win.width, vconf->win.height, vconf->buffer, vconf->o_buffer);
    break;
  default:
    v_error(vconf, LOG_CRIT, "Should not happen - Unknown input image format");
    break;
  }

  if (vconf->vpic.palette!=VIDEO_PALETTE_RGB24)
    v_error(vconf, LOG_DEBUG, "converted to RGB24");
     
  if (vconf->switch_bgr) {
    vconf->o_buffer=switch_color(vconf);
    v_error(vconf, LOG_DEBUG, "Switching vom BGR to RGB - or vice versa");
  }
  return vconf->o_buffer;
}

/* Now for the LIBTTF functions for the timestamp */
		
#ifdef LIBTTF

/* Open Face via routine found in font.c  */

struct ttneed *OpenFace(struct ttneed *ttinit, struct vconfig *vconf) 
{
  int i, j;

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
  
  v_error(vconf, LOG_DEBUG, "Getting all values for the timestamp.");

  time (&t);
  tm = localtime (&t);
  ts_buff[TS_MAX] = '\0';
  strftime (ts_buff, TS_MAX, vconf->timestamp, tm);
  ts_len = strlen (ts_buff);

  v_error(vconf, LOG_DEBUG, "Stamp: %s, length: %d", ts_buff, ts_len);

  glyphs = Glyphs_Load(ttinit->face, ttinit->properties, ttinit->instance,
		       ts_buff, ts_len);

  v_error(vconf, LOG_DEBUG, "Glyphs loaded");

  Raster_Init(ttinit->face, ttinit->properties, ttinit->instance, ts_buff,
	      ts_len, vconf->border, glyphs, &bit);

  v_error(vconf, LOG_DEBUG, "Returned from Raster_Init");

  Raster_Small_Init (&sbit, &ttinit->instance);

  v_error(vconf, LOG_DEBUG, "Returned from Raster_Small_Init");

  Render_String(glyphs, ts_buff, ts_len, &bit, &sbit, vconf->border);

  v_error(vconf, LOG_DEBUG, "Returned from Render_String");

  if (bit.bitmap) 
    {
      int x, y, psize, i, x_off, y_off;
      unsigned char *p;

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

  v_error(vconf, LOG_DEBUG, "Image manipulated, now closing...");

  Raster_Done (&sbit);
  Raster_Done (&bit);
  Glyphs_Done (glyphs);
  glyphs = NULL;
  Face_Done(ttinit->instance, ttinit->face);

  v_error(vconf, LOG_INFO, "Font-Engine unloaded, stamp inserted into image");

  return buffer;
}

#endif

/* Main loop  */

int main(int argc, char *argv[]) 
{
  struct vconfig *vconf=NULL;

#ifdef DEBUGGING
  mtrace();
#endif

  vconf=v_init(vconf, 0, argc, argv);

  if (vconf->loop) 
    daemonize(vconf, basename(argv[0]));
  else
    v_error(vconf, LOG_WARNING, "Reading image from %s", vconf->in);

  if (vconf->openonce) {
    while ((vconf->dev=open(vconf->in, O_RDONLY)) < 0)
      v_error(vconf, LOG_ERR, "Problem opening input-device %s", vconf->in);
    
    v_error(vconf, LOG_DEBUG, "Device %s successfully opened", vconf->in);
  } 
  
  // now start the loop (if daemon), read image and convert it, if necessary 
  do {
    vconf->buffer=read_image(vconf, img_size(vconf, vconf->vpic.palette));
    vconf->o_buffer = conv_image(vconf);
    
#ifdef LIBTTF
    if (vconf->use_ts) 
      vconf->o_buffer=inserttext(vconf->ttinit, vconf->o_buffer, vconf);
#endif
    
    write_image(vconf, vconf->o_buffer);
    
    vconf->err_count=0;
    usleep(vconf->loop);
    
    // We got a signal, we do have to...
    switch (signal_terminate) {
    case 0:
      // No signal received
      break;
    case SIGTERM:
      // We got sigkill, cleanup (for daemon mode) and exit
      cleanup(vconf, FALSE);
      break;
    case SIGHUP:
      // We got sighup, re-read the config-file (do NOT parse commandline)
      vconf=v_reinit(vconf);
      break;
    default:
      v_error(vconf, LOG_DEBUG, "Unknown signal received, ignoring.");
      break;
    }
  } while (vconf->loop);
  
  cleanup(vconf, TRUE);
  exit(0);
}






