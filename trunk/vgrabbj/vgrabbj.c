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

#include <vgrabbj.h>
#include <v_plist.h>

int signal_terminate=0;

void cleanup(struct vconfig *vconf) {
  int tmp=vconf->loop;

  v_update_ptr(vconf);
  if (vconf->openonce) {
    free_mmap(vconf);
    close_device(vconf);
    }
#ifdef LIBTTF
  if ( vconf->use_ts && vconf->ttinit ) {
    Face_Done(vconf->ttinit->instance, vconf->ttinit->face);
    TT_Done_FreeType(vconf->ttinit->engine);
  }
  if ( vconf->ttinit ) {
    vconf->ttinit->properties=free_ptr(vconf->ttinit->properties);
    vconf->ttinit=free_ptr(vconf->ttinit);
  }
  vconf->timestamp=free_ptr(vconf->timestamp);
  vconf->font=free_ptr(vconf->font);
  v_error(vconf, LOG_DEBUG, "TrueType engine ended, vars freed");
#endif
  vconf->buffer=free_ptr(vconf->buffer);
  vconf->o_buffer=free_ptr(vconf->o_buffer);
  v_error(vconf, LOG_DEBUG, "Buffers freed");
  vconf->tmpout=free_ptr(vconf->tmpout);
  vconf->in=free_ptr(vconf->in);
  vconf->out=free_ptr(vconf->out);
  vconf->conf_file=free_ptr(vconf->conf_file);
  v_error(vconf, LOG_DEBUG, "vars freed");

  vconf=free_ptr(vconf);
			 
  if ( tmp ) {
     closelog();
    _exit(1);
  }
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
    syslog(LOG_ERR, "Fatal Error (daemon), exiting...\n"); // exit
    cleanup(vconf);
    closelog();
    _exit(1);
  } else if ( !vconf->loop && msg < 4 ) {
    fprintf(stderr, "Fatal Error (non-daemon), exiting...\n"); // exit
    cleanup(vconf);
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

  if ( (dev = v4l1_open(in, O_RDONLY)) < 0 ) {
    fprintf(stderr, "Can't open device %s\n", in);
    exit(1);
  }
  if (v4l1_ioctl(dev, VIDIOCGCAP, &cap) < 0) {
    fprintf(stderr, "Can't get capabilities of device %s\n", in);
    exit(1);
  }
  if (v4l1_ioctl(dev, VIDIOCGPICT, &pic) < 0) {
    fprintf(stderr, "Can't get picture properties of device %s\n", in);
    exit(1);
  }
  if (v4l1_ioctl(dev, VIDIOCGWIN, &win) < 0) {
    fprintf(stderr, "Can't get overlay values of device %s\n", in);
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
  dev=v4l1_close(dev);
  if (dev)
    fprintf(stderr, "Error occured while closing %s\n", in);
  exit(0);
}


/* Read the image from device, adjust brightness, if wanted, for RGB24 */

unsigned char *read_image(struct vconfig *vconf, int size) {
  int f, newbright;
  int err_count;
  int discard=vconf->discard;

  v_error(vconf, LOG_DEBUG, "Palette to be used: %s (%d), size: %d",
	  plist[vconf->vpic.palette].name, vconf->vpic.palette, img_size(vconf, vconf->vpic.palette));

  /* Opening input device */
  if ( !vconf->openonce ) {
    open_device(vconf);

    /* and Re-initialize the palette, in case someone changed it meanwhile */

    while (v4l1_ioctl(vconf->dev, VIDIOCSPICT, &vconf->vpic) < 0 )
      v_error(vconf, LOG_ERR, "Device %s couldn't be reset to known palette %s",
	      vconf->in, vconf->vpic.palette);
    if (vconf->windowsize)
      while (v4l1_ioctl(vconf->dev, VIDIOCSWIN, &vconf->win) )
	v_error(vconf, LOG_ERR, "Problem setting window size"); // exit

    set_picture_parms(vconf);
  }


  /* Read image via read() */

  if (!vconf->usemmap) {
    if (vconf->autobrightness)
      v_error(vconf, LOG_INFO, "Forced to use brightness adj. - using read()");
    else
      v_error(vconf, LOG_INFO, "Could not get mmap-buffer - Falling back to read()");
    do {
      err_count=0;
      if (vconf->autobrightness && vconf->vpic.palette==VIDEO_PALETTE_RGB24) {
	v_error(vconf, LOG_INFO, "Doing brightness adjustment");
	do {
	  while (v4l1_read(vconf->dev, vconf->buffer, size) < size)
	    v_error(vconf, LOG_ERR, "Error reading from %s", vconf->in);
	  f = brightness_adj(vconf, &newbright);
	  if (f) {
	    vconf->vpic.brightness += (newbright << 8);
	    if (v4l1_ioctl(vconf->dev, VIDIOCSPICT, &vconf->vpic)==-1) 
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
	v4l1_read(vconf->dev, vconf->buffer, size);
      }
    } while (discard--);

    /* We're reading the image via a mmap'd area of the driver */

  } else { 
    v_error(vconf, LOG_DEBUG, "Using mmap for image grabbing");
    if (!vconf->openonce)
      init_mmap(vconf);
    do {
      err_count=0;
      do {
	if  (err_count++>100) {
	  v_error(vconf, LOG_ERR, "Could not grab frame (100 tries)");
	  break;
	}
      } while (v4l1_ioctl(vconf->dev, VIDIOCMCAPTURE, &vconf->vmap) < 0);

      err_count=0;
      do {
	if (err_count++>100) {
	  v_error(vconf, LOG_ERR, "Could not sync with frame (100 tries)");
	  break;
	}
      } while (v4l1_ioctl(vconf->dev, VIDIOCSYNC, &vconf->vmap.frame) < 0);

      vconf->buffer=memcpy(vconf->buffer, vconf->map+vconf->vbuf.offsets[vconf->vmap.frame], size);

      if (discard)
	v_error(vconf, LOG_DEBUG, "%d frames to discard", discard);
      
    } while (discard--);
    if (!vconf->openonce)
      free_mmap(vconf);
  }
  
  v_error(vconf, LOG_DEBUG, "Image successfully read");

  if (!vconf->openonce)
    close_device(vconf);
  
  return vconf->buffer;
}


unsigned char *conv_image(struct vconfig *vconf) {

long i,j;

  switch (vconf->vpic.palette) {
  case VIDEO_PALETTE_RGB24:
    vconf->o_buffer=memcpy(vconf->o_buffer, vconf->buffer, 
		    vconf->win.width * vconf->win.height * 3);
    v_error(vconf, LOG_DEBUG, "No conversion, we have RGB24");
    break;
  case VIDEO_PALETTE_GREY:
    for(i=0 ;i<(vconf->win.width * vconf->win.height * 1) ;i++)
    {
	for(j=0 ;j<3 ;j++)
	{
	    vconf->o_buffer[(3*i)+j] = vconf->buffer[i];
	}
    }
    v_error(vconf, LOG_DEBUG, "Got GREY, converting...");
    break;
  case VIDEO_PALETTE_RGB32:
    v_error(vconf, LOG_INFO, "Got RGB32, converting...");
   
    vconf->o_buffer=conv_rgb32_rgb24(vconf);
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

    ccvt_420i_rgb24(vconf->win.width, vconf->win.height, vconf->buffer, vconf->o_buffer);
    break;
  case VIDEO_PALETTE_YUYV:
  case VIDEO_PALETTE_YUV422:
    v_error(vconf, LOG_INFO, "Got YUYV, converting...");
      
    ccvt_yuyv_bgr24(vconf->win.width, vconf->win.height, vconf->buffer, vconf->o_buffer);
    break;
  case VIDEO_PALETTE_UYVY:
    v_error(vconf, LOG_INFO, "Got UYVY, converting...");

    ccvt_uyvy_bgr24(vconf->win.width, vconf->win.height, vconf->buffer, vconf->o_buffer);
    break;
  default:
    v_error(vconf, LOG_CRIT, "Should not happen - Unknown input image format");
    break;
  }

  if (vconf->vpic.palette!=VIDEO_PALETTE_RGB24)
    v_error(vconf, LOG_DEBUG, "converted to RGB24");
     
  if (vconf->switch_bgr) {
    vconf->o_buffer=switch_color(vconf);
    v_error(vconf, LOG_DEBUG, "Switched from BGR to RGB - or vice versa");
  }
  return vconf->o_buffer;
}
		

/* Main loop  */

int main(int argc, char *argv[]) 
{
  struct vconfig *vconf=NULL;

#ifdef DEBUGGING
  mtrace();
#endif

  vconf=v_init(vconf, argc, argv);

  if (vconf->loop && (!vconf->nofork) ) 
    daemonize(vconf, basename(argv[0]));
  else
    v_error(vconf, LOG_WARNING, "Reading image from %s", vconf->in);

  // now start the loop (if daemon), read image and convert it, if necessary 
  do {
    vconf->buffer=read_image(vconf, img_size(vconf, vconf->vpic.palette));
    vconf->o_buffer = conv_image(vconf);
    
    if (vconf->swaprl) 
      vconf->o_buffer=swap_left_right(vconf->o_buffer, vconf->win.width, vconf->win.height);

    if (vconf->swaptb)
      vconf->o_buffer=swap_top_bottom(vconf->o_buffer, vconf->win.width, vconf->win.height);

#ifdef LIBTTF
    if (vconf->use_ts) 
      vconf->o_buffer=inserttext(vconf->ttinit, vconf->o_buffer, vconf);
#endif
    
    write_image(vconf);
    
    vconf->err_count=0;
    usleep(vconf->loop);
    
    // We got a signal, we do have to...
    switch (signal_terminate) {
    case 0:
      // No signal received
      break;
    case SIGTERM:
      // We got sigkill, cleanup (for daemon mode) and exit
      signal_terminate=0;
      cleanup(vconf);
      break;
    case SIGHUP:
      // We got sighup, re-read the config-file (do NOT parse commandline)
      signal_terminate=0;
      vconf=v_reinit(vconf);
      break;
    default:
      v_error(vconf, LOG_DEBUG, "Unknown signal received, ignoring.");
      break;
    }
  } while (vconf->loop);
  
  cleanup(vconf);
  exit(0);
}






