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

/* Functions to parse the (optional) config file */

#include "vgrabbj.h"

extern void v_error(struct vconfig *vconf, int msg, char *fmt, ...);

static int get_int(char *value) {

  return 1;
}

static char *get_str(char *value) {

  return "t";
}

static int get_bool(char *value) {

  return TRUE;
}

static int get_format(char *value) {

  return 1;
}

static int get_position(char *value) {

  return 1;
}

static int get_width(char *value) {

  return 128;
}

static int get_height(char *value) {

  return 96;
}

struct vconfig *parse_config(struct vconfig *vconf, char *path){

  int           n=0, tmp, dev;
  char          line[MAX_LINE];
  char          *option, *value, *p;
  FILE          *fd, *x;

  /* Check for config file */

  if (! (fd = fopen(path, "r") ) ) {
    v_error(vconf, LOG_WARNING, "Could not open configfile %s, ignoring", path);
    return vconf;
  }

  /* read every line */
  v_error(vconf, LOG_DEBUG, "Starting to read arguments from file %s", path);

  while (fgets(line, sizeof(line), fd)) {
    n++;
    /* hide comments */
    if ((p=strchr(line, '#')))
      *p='\0';
    if ((p=strchr(line, ';')))
      *p='\0';

    /* Check options */

    if ( ( (option=strtok(line,"= \t\n")) != NULL) && ((value=strtok(NULL, "= \t\n")) != NULL) ) {
      v_error(vconf, LOG_DEBUG, "Option: %s, Value: %s", option, value);
      if ( !strcasecmp(option, "ImageQuality")) {
	if ( (MIN_QUALITY > (vconf->quality=get_int(value))) || (vconf->quality > MAX_QUALITY) ) 
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)", 
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "VideoDevice") ) {
	if ( (dev=open( (vconf->in=get_str(value)), O_RDONLY)) < 0 )
	  v_error(vconf, LOG_CRIT, "Can't open %s as %s (line %d, %s)", 
		  value, option, n, path);
	else {
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	  fclose(x);
	}
      }
      else if ( !strcasecmp(option, "OutputFile") ) {
	if ( !(x=fopen((vconf->out=get_str(value)), "w+")) )
	  v_error(vconf, LOG_CRIT, "Can't open %s as %s (line %d, %s)",
		  value, option, n, path);
	else {
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	  fclose(x);
	}
      }
      else if ( !strcasecmp(option, "ImageSize") ) {
	if ( (vconf->win.width=get_width(value)) == 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	if ( (vconf->win.height=get_height(value)) == 0)
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "OutputFormat") ) {
	if ( (vconf->outformat=get_format(value)) == 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "Brightness") ) {
	if ( (tmp=get_bool(value)) == -1 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->brightness=TRUE;
	else
	  vconf->brightness=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "SwitchColor") ) {
	if ((tmp=get_bool(value)) == -1)
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->switch_bgr=TRUE;
	else
	  vconf->switch_bgr=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
     }
      else if ( !strcasecmp(option, "SetWindowSize") ) {
	if ((tmp=get_bool(value)) == -1)
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->windowsize=TRUE;
	else
	  vconf->windowsize=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "Daemon") ) {
	if ( (vconf->loop=get_int(value)) < MIN_LOOP)
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "UseTimestamp") ) {
	if ( (tmp=get_bool(value) == -1) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else if (tmp==1)
	  vconf->use_ts=TRUE;
	else
	  vconf->use_ts=FALSE;
	v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "DebugLevel") ) {
	if ( (MIN_DEBUG > (vconf->debug=get_int(value))) || (vconf->debug > MAX_DEBUG))
	  v_error(vconf, LOG_CRIT, "Wrong value '%s' for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
      else if ( !strcasecmp(option, "ForcePalette") ) {
	if ( (MIN_PALETTE > (vconf->forcepal=get_int(value))) || (vconf->forcepal > MAX_PALETTE) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
      }
#ifdef HAVE_LIBTTF
      else if ( !strcasecmp(option, "FontFile") ) {
	if (!fopen((vconf->font=get_str(value)), "r") )
	  v_error(vconf, LOG_CRIT, "Can't not open %s as %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "TimeStamp") ) {
	if ( !(vconf->timestamp=get_str(value)) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);

	}
      }
      else if ( !strcasecmp(option, "FontSize") ) {
	if ( (MIN_FONTSIZE > (vconf->font_size=get_int(value))) || (vconf->font_size > MAX_FONTSIZE) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "BorderSize") ) {
	if ( (MIN_BORDER > (vconf->border=get_int(value))) || (vconf->border > MAX_BORDER) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "Position") ) {
	if ( (MIN_ALIGN > (vconf->align=get_position(value))) || (vconf->align > MAX_ALIGN) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "Blend") ) {
	if ( (MIN_BLEND > (vconf->blend=get_int(value))) || (vconf->blend > MAX_BLEND) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
#endif
      else
	v_error(vconf, LOG_CRIT, "Unknown Option %s (line %d, %s)", option, n, path);
    }
  }
  if (x)
    fclose(x);
  fclose(fd);
  return vconf;
}
