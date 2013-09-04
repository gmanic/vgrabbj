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

#include "vgrabbj.h"

#ifdef LIBTTF

/* Open Face via routine found in font.c  */

struct ttneed *OpenFace(struct vconfig *vconf)
{
  int i, j;

  v_error(vconf, LOG_INFO, "Initializing Font-Engine");

  if (vconf->debug) {
    TT_FreeType_Version( &i, &j);
    v_error(vconf, LOG_DEBUG, "FreeType, Version %d.%d", i, j);
  }

  if (Face_Open (vconf->font, vconf->ttinit->engine, &vconf->ttinit->face,
		 vconf->ttinit->properties, &vconf->ttinit->instance, vconf->font_size)) {
    v_error(vconf, LOG_WARNING, "Font not found: %s, timestamp disabled", vconf->font);
    return(NULL);
  }
  v_error(vconf, LOG_DEBUG, "Font-Engine initialized");

  return(vconf->ttinit);
}

/* Manipulate image to show timestamp string and return manipulated buffer */

char *inserttext(struct ttneed *ttinit, unsigned char *buffer, struct vconfig *vconf)
{
  char *ts_buff;
  int ts_len;
  TT_Glyph *glyphs = NULL;
  TT_Raster_Map bit;
  TT_Raster_Map sbit;

  v_error(vconf, LOG_DEBUG, "Getting all values for the timestamp. ts=%s.", vconf->timestamp);

  ts_len = strlen(ts_buff=timestring(vconf->timestamp));

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

  free_ptr(ts_buff);

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

  v_error(vconf, LOG_INFO, "Stamp inserted into image");

  return buffer;
}

#endif

