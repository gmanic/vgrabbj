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

  int           n=0, tmp, dev, is_width=0, is_height=0;
  char          line[MAX_LINE];
  char          *option=NULL, *value=NULL, *p=NULL;
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
	if ( (dev=open( (vconf->in=get_str((value=strtok(NULL, " \t\n")))), O_RDONLY)) < 0 ) {
	  close(dev);
	  v_error(vconf, LOG_CRIT, "Can't open %s as %s (line %d, %s)", 
		  value, option, n, path);
	}
	else {
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	  close(dev);
	}
      }
      else if ( !strcasecmp(option, "OutputFile") ) {
	if ( !(x=fopen((vconf->out=get_str((value=strtok(NULL, " \t\n")))), "w+")) )
	  v_error(vconf, LOG_CRIT, "Can't open %s as %s (line %d, %s)",
		  value, option, n, path);
	else {
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	  fclose(x);
	}
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
#ifdef HAVE_LIBTTF
      else if ( !strcasecmp(option, "FontFile") ) {
	if (!fopen((vconf->font=get_str((value=strtok(NULL, " \t\n")))), "r") )
	  v_error(vconf, LOG_CRIT, "Can't not open %s as %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "TimeStamp") ) {
	if ( !(vconf->timestamp=get_str((value=strtok(NULL, "\"\t\n")))) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "FontSize") ) {
	if ( (MIN_FONTSIZE > (vconf->font_size=get_int((value=strtok(NULL, " \t\n"))))) ||
	     (vconf->font_size > MAX_FONTSIZE) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "BorderSize") ) {
	if ( (MIN_BORDER > (vconf->border=get_int((value=strtok(NULL, " \t\n"))))) ||
	     (vconf->border > MAX_BORDER) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "Position") ) {
	if ( (vconf->align=get_position((value=strtok(NULL, " \t\n")))) < 0 )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
      else if ( !strcasecmp(option, "Blend") ) {
	if ( (MIN_BLEND > (vconf->blend=get_int((value=strtok(NULL, " \t\n"))))) ||
	     (vconf->blend > MAX_BLEND) )
	  v_error(vconf, LOG_CRIT, "Wrong value \"%s\" for %s (line %d, %s)",
		  value, option, n, path);
	else {
	  vconf->use_ts=TRUE;
	  v_error(vconf, LOG_DEBUG, "Setting option %s to value %s", option, value);
	}
      }
#endif
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








