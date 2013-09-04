/* Simple Video4Linux image grabber. Made for my Philips Vesta Pro
 * 
 * Definition of options
 *
 * Copyright (C) 2002 Jens Gecius, Hannover, Germany
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

#include "v_utils.h"

/* Initializes the mapping of driver memory to vgrabbj */

void init_mmap(struct vconfig *vconf) {
  if (vconf->usemmap) {
    if ( (vconf->map = v4l1_mmap(0, vconf->vbuf.size, PROT_READ, MAP_SHARED, vconf->dev, 0)) < 0 )
      v_error(vconf, LOG_CRIT, "Could not get mmap-area of size %d", vconf->vbuf.size);
    if ( v4l1_ioctl(vconf->dev, VIDIOCGMBUF, &vconf->vbuf) < 0 )
      v_error(vconf, LOG_CRIT, "Could not initialize mmap-vars");
    
    v_error(vconf, LOG_DEBUG, "Size allocated for framebuffer: %d", vconf->vbuf.size);
    
    if (!vconf->map)
      v_error(vconf, LOG_CRIT, "mmap'ed area not allocated");
  }
}


/* Free's allocated mmap (if it exists) */

void free_mmap(struct vconfig *vconf) {
  if (vconf->map) {
    if (!v4l1_munmap(vconf->map, vconf->vbuf.size)) {
      v_error(vconf, LOG_DEBUG, "mmap'ed area 'freed'");
      vconf->map=NULL;
    }
    else
      v_error(vconf, LOG_ERR, "Error %d occured while unmapping map", errno);
  }
  else
    v_error(vconf, LOG_WARNING, "There was no map allocated to be freed...");
}


/* Open Input Device */

void open_device(struct vconfig *vconf) {
  int err_count=0;
  while ( ((vconf->dev=v4l1_open(vconf->in, O_RDONLY)) < 0) && (!(err_count++>200)) )
    usleep(25000);
  if (err_count>200)
    v_error(vconf, LOG_ERR, "Problem opening input-device %s", vconf->in);
  else
    v_error(vconf, LOG_DEBUG, "Device %s successfully opened", vconf->in);
}


/* Closing Input Device */

void close_device(struct vconfig *vconf) {
  if(vconf->dev) {
    if ( (vconf->dev=v4l1_close(vconf->dev)) )
      v_error(vconf, LOG_ERR, "Error while closing %s", vconf->in);
    else
      v_error(vconf, LOG_DEBUG, "Device %s closed", vconf->in);
  }
  else
    v_error(vconf, LOG_WARNING, "Device %s was already closed...", vconf->in);
}

/* following code thanks to David Austin */ 

int set_picture_parms(struct vconfig *vconf) {
  if ((vconf->hue < 0) && (vconf->brightness < 0) &&
      (vconf->colour < 0) && (vconf->contrast < 0) &&
      (vconf->whiteness < 0)) {
    return 0;
  }

  fprintf(stderr, "Setting picture params %i %i %i %i %i\n", 
	  vconf->hue, vconf->brightness, vconf->colour,
	  vconf->contrast, vconf->whiteness);
  
  if (v4l1_ioctl(vconf->dev, VIDIOCGPICT, &(vconf->vpic)) == -1) {
    perror ("PICTURE");
    return (-1);
  }
  
  if (vconf->hue > -1) 
    vconf->vpic.hue = vconf->hue;
  if (vconf->contrast > -1) 
    vconf->vpic.contrast = vconf->contrast;
  if (vconf->brightness > -1) 
    vconf->vpic.brightness = vconf->brightness;
  if (vconf->colour > -1) 
    vconf->vpic.colour = vconf->colour;
  if (vconf->whiteness > -1) 
    vconf->vpic.whiteness = vconf->whiteness;
  
  if (v4l1_ioctl(vconf->dev, VIDIOCSPICT, &(vconf->vpic)) == -1) {
    perror ("PICTURE");
    return (-1);
  }
  return 0;
}

/* Returns the actual byte-size of the image */

int img_size(struct vconfig *vconf, int palette) {
  return ( (plist[palette].mul * vconf->win.width * vconf->win.height) >> (plist[palette].div==2?1:0) );
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


/* Swap Left to Right (like a mirror) */

unsigned char *swap_left_right(char *buffer, int width, int height) 
{
  char a, b, c;
  int i, j;
  for (j=0;j < height;j++) {
    for (i=0;i < (width>>1);i++) {
      a = buffer[(j*width*3)+(i*3)];
      b = buffer[(j*width*3)+(i*3)+1];
      c = buffer[(j*width*3)+(i*3)+2];
      
      buffer[(j*width*3)+(i*3)]   = buffer[(j*width*3)+(width-i)*3];
      buffer[(j*width*3)+(i*3)+1] = buffer[(j*width*3)+(width-i)*3+1];
      buffer[(j*width*3)+(i*3)+2] = buffer[(j*width*3)+(width-i)*3+2];
      
      buffer[(j*width*3)+(width-i)*3]   = a;
      buffer[(j*width*3)+(width-i)*3+1] = b;
      buffer[(j*width*3)+(width-i)*3+2] = c;
    }
  }
  return buffer;
}

/* Swap Top to Bottom (like a mirror) */
/* Thanks to Koos van den Hout and Arthur van Leeuwen */

unsigned char *swap_top_bottom(char *buffer, int width, int height) 
{
  char a, b, c;
  int i, j;
  for (j=0;j < (height>>1);j++) {
    for (i=0;i < width;i++) {
      a = buffer[(j*width*3)+(i*3)];
      b = buffer[(j*width*3)+(i*3)+1];
      c = buffer[(j*width*3)+(i*3)+2];
      
      buffer[(j*width*3)+(i*3)]   = buffer[((height-j)*width*3)+(i*3)];
      buffer[(j*width*3)+(i*3)+1] = buffer[((height-j)*width*3)+(i*3)+1];
      buffer[(j*width*3)+(i*3)+2] = buffer[((height-j)*width*3)+(i*3)+2];
      
      buffer[((height-j)*width*3)+(i*3)]   = a;
      buffer[((height-j)*width*3)+(i*3)+1] = b;
      buffer[((height-j)*width*3)+(i*3)+2] = c;

    }
  }
  return buffer;
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
  v_error(vconf, LOG_WARNING, "Forking for daemon mode");
    
  switch( fork() ) 
    {
    case 0: // Child 
      v_error(vconf, LOG_DEBUG, "I'm the child process and are going to read images...");
      closelog();
      vconf->init_done=TRUE;
      break;
    case -1: /* Error  */
      v_error(vconf, LOG_CRIT, "Can't fork, exiting..."); // exit
      break;
    default: /* Parent  */
      v_error(vconf, LOG_DEBUG, "I'm the parent and exiting now"
		"(child takes care of the rest).");
   closelog();
   cleanup(vconf);
   exit(0);
    }
  openlog(progname, LOG_PID, LOG_DAEMON);
  v_error(vconf, LOG_WARNING, "%s started, reading from %s", progname, vconf->in);
  
  return(1);
}


/* These are more or less self-explanatory */

char *timestring(char *format) {
  time_t td;
  struct tm *tm;
  int i=1;
  char archive[512];
  /* for stamp and archive strftime format strings are used, make the final path
   * to archive_path */
  time(&td);
  tm = localtime(&td);
  while (i<512) {
    archive[i++]='0';
  }
  archive[sizeof(archive)-1]='\0';
  i=strftime(archive, sizeof(archive)-1, format, tm);
  //  fprintf(stdout, "ts=%s.%d\n", archive,i);
  return (strcpy(malloc(++i),archive));
}


char *strip_white(char *value) {
  long int tmp=0;
  /* eliminate whitespaces in value */
  while ((value[0]==' ') || (value[0]=='\t') || (value[0]=='"')) value++;
  tmp=strlen(value);
  while ((value[--tmp]==' ') || (value[tmp]=='\t') || (value[tmp]=='"')) value[tmp]='\0';
  return value;
}


long int check_minmax(struct vconfig *vconf, char *value, long int tmp, int n, struct v_options l_opt) {
  if (l_opt.max_value || tmp < 0)
    if ( (l_opt.min_value > tmp) || (tmp > l_opt.max_value) )
      v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s, min %d, max %d)",
	      value, l_opt.name, n, n?vconf->conf_file:"command-line", l_opt.min_value,
	      l_opt.max_value);
  v_error(vconf, LOG_DEBUG, "Set option %s to value %d (value: %s)", l_opt.name, tmp, value);
  return tmp;
}


char *check_maxlen(struct vconfig *vconf, char *value, struct v_options l_opt, int n) {
  //  value=strip_white(value);
  if ( (l_opt.max_length) && (strlen((value)) > l_opt.max_length) )
    v_error(vconf, LOG_CRIT, "Value \"%s\" too long (max. %d, line %d, %s)", value,
	    l_opt.max_length, n, n?vconf->conf_file:"command-line");
  v_error(vconf, LOG_DEBUG, "Set option %s to value \"%s\"", l_opt.name, value);
  return value;
}


char *get_str(char *value, char *var) {
  int t;
  if (var) free (var);
  if ( (t=strlen(value))<1 )
      return NULL;
  value=strip_white(value);
  var=strcpy(malloc(t+1),value);
  return var;
}


long int get_int(char *value) {
  long tmp;
  if ( sscanf(value, "%ld", &tmp) != 1 )
    return -1;
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
  int i;
  for (i=0; output_list[i].name; i++) {
  if ( !(strcasecmp(value, output_list[i].name)) )
    return output_list[i].type;
  }
  return -1;
}


int get_position(char *value) {
  int i;
  if ((i=get_int(value))!=-1)
    return i;
  for (i=0; position_list[i].name; i++) {
    if ( !(strcasecmp(value, position_list[i].name)) )
      return position_list[i].type;
  }
  return -1;
}


int decode_size(char *value) {
  int i;
  for (i=0; size_list[i].name; i++) {
    if ( !(strcasecmp(value, size_list[i].name)) )
      return size_list[i].type;
  }
  return 0;
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
  return (vconf=v_init(vconf, 0, '\0'));
}
