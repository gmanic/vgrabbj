/* Simple Video4Linux image grabber. Made for my Philips Vesta Pro
 * 
 * Definition of options
 *
 * Copyright (C) 2002 Jens Gecius, Larchmont, USA
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

#include "vgrabbj.h"
#include "v_plist.h"

/* Initializes the mapping of driver memory to vgrabbj */

void init_mmap(struct vconfig *vconf) {
  if (vconf->usemmap) {
    if ( (vconf->map = mmap(0, vconf->vbuf.size, PROT_READ, MAP_SHARED, vconf->dev, 0)) < 0 )
      v_error(vconf, LOG_CRIT, "Could not get mmap-area of size %d", vconf->vbuf.size);
    if ( ioctl(vconf->dev, VIDIOCGMBUF, &vconf->vbuf) < 0 )
      v_error(vconf, LOG_CRIT, "Could not initialize mmap-vars");
    
    v_error(vconf, LOG_DEBUG, "Size allocated for framebuffer: %d", vconf->vbuf.size);
    
    if (!vconf->map)
      v_error(vconf, LOG_CRIT, "mmap'ed area not allocated");
  }
}


/* Free's allocated mmap (if it exists) */

void free_mmap(struct vconfig *vconf) {
  if (vconf->map)
    munmap(vconf->map, vconf->vbuf.size);
}


/* Open Input Device */

void open_device(struct vconfig *vconf) {
  while ((vconf->dev=open(vconf->in, O_RDONLY)) < 0)
    v_error(vconf, LOG_ERR, "Problem opening input-device %s", vconf->in);
  v_error(vconf, LOG_DEBUG, "Device %s successfully opened", vconf->in);
}


/* Closing Input Device */

void close_device(struct vconfig *vconf) {
    while ( close(vconf->dev) )
      v_error(vconf, LOG_ERR, "Error while closing %s", vconf->in); // exit
    
    v_error(vconf, LOG_DEBUG, "Device %s closed", vconf->in);
}


/* Returns the actual byte-size of the image */

int img_size(struct vconfig *vconf, int palette) {
  return (plist[palette].mul * vconf->win.width * vconf->win.height) / plist[palette].div;
}


/* Turns RGB into BGR (or vice versa) */

unsigned char *switch_color(struct vconfig *vconf) {
  char a;
  long int y;
  v_error(vconf, LOG_DEBUG, "width: %d, heigth: %d", vconf->win.width, vconf->win.height);
  for (y = 0; y < (vconf->win.width * vconf->win.height); y++) {
    memcpy(&a, vconf->o_buffer+(y*3),1);
    memcpy(vconf->o_buffer+(y*3), vconf->o_buffer+(y*3)+2, 1);
    memcpy(vconf->o_buffer+(y*3)+2, &a, 1);
  }
  return vconf->o_buffer;
}


/* Strips last byte of RGB32 to convert to RGB24 - breaks picture if alpha is used! */

unsigned char *conv_rgb32_rgb24(struct vconfig *vconf) {
  long int y;
  for (y = 0; y < (vconf->win.width * vconf->win.height); y++) 
    memcpy(vconf->o_buffer+(y*3), vconf->buffer+(y*4), 3);
  return vconf->o_buffer;
}


/* Adjustment of brightness of picture  */

int brightness_adj(struct vconfig *vconf, int *brightness) 
{
  long i, tot = 0;
  long size = vconf->win.width * vconf->win.height;
  for ( i = 0; i < size * 3; i++ )
    tot += vconf->buffer[i];
  *brightness = (128 - tot/(size*3))/3;
  return !((tot/(size*3)) >= 126 && (tot/(size*3)) <= 130);
}


/* Event handler for SIGKILL */
/* to properly clean up on externally requested termination */

void sigterm();

void sigterm() {
  signal(SIGTERM,sigterm); /* reset signal */
  syslog(LOG_WARNING, "Caught sigterm, cleaning up...");
  signal_terminate=SIGTERM;
}


/* Event handler for SIGHUP */
/* to re-read the configuration file */

void sighup();

void sighup() {
  signal(SIGHUP,sighup); /* reset signal */
  syslog(LOG_WARNING, "Caught sighup, re-reading config-file...");
  signal_terminate=SIGHUP;
}


/* Checks whether a ptr is allocated, if so, free it */

void *free_ptr(void *buf) {
  if (buf) free(buf);
  return NULL;
}


/* Daemonize vgrabbj only if in daemon-mode */

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


/* These are more or less self-explanatory */

int get_int(char *value) {
  long tmp;
  if ( sscanf(value, "%ld", &tmp) != 1 )
    return -1;
  return tmp;
}


char *get_str(char *value, char *var) {
  if (var) free(var);
  if ( strlen(value)<1 )
    return NULL;
  var=strcpy(malloc(strlen(value)+1),value);
  return var;
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


/* If a SIGHUP was caught, do only re-initialisation */

struct vconfig *v_reinit(struct vconfig *vconf) {
  return (vconf=v_init(vconf, 1, 0, '\0'));
}
