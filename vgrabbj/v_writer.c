/* Simple Video4Linux image grabber. Made for my Philips Vesta Pro
 * 
 * Copyright (C) 2001 Jens Gecius, Larchmont, USA
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

/* Functions to write the output to something */

#include "vgrabbj.h"

/* Function to write output data to a temporary file and then copy 
 * it to the final location via system-call to avoid problems with 
 * "same-time" access to the output file                           
*/

/* Write image as jpeg to FILE
*/

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
  png_bytep rowpointers[vconf->win.height];
  png_infop info_ptr;
  png_structp png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING,
						 NULL, NULL, NULL);

  if (!png_ptr)
    return(1);
  info_ptr = png_create_info_struct (png_ptr);
  if (!info_ptr) {
    png_destroy_write_struct(&png_ptr, (png_infopp)NULL);
    return(1);
  }
  if (setjmp(png_jmpbuf(png_ptr))) {
    /* If we get here, we had a problem reading the file */
    png_destroy_write_struct(&png_ptr, &info_ptr);
    return (1);
  }

  png_init_io (png_ptr, x);
  png_set_IHDR (png_ptr, info_ptr, vconf->win.width, vconf->win.height,
		8, PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
		PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info (png_ptr, info_ptr);
  for (y = 0; y < vconf->win.height; y++) 
    {
      rowpointers[y] = image + y*vconf->win.width*3;
    }
  png_write_image(png_ptr, rowpointers);
  png_write_end (png_ptr, info_ptr);
  png_destroy_write_struct (&png_ptr, &info_ptr);
  return(0);
}

/* Write image as ppm to FILE  */

int write_ppm(struct vconfig *vconf, char *image, FILE *x) 
{

  fprintf(x,"P6\n%d %d\n255\n",vconf->win.width,vconf->win.height);
  fwrite(image,vconf->win.height,3*vconf->win.width,x);

  return(0);
}

/* Function to write an image, called by main */

void write_image(struct vconfig *vconf, unsigned char *o_buffer) {
  FILE *x;

  if ( vconf->tmpout ) {
    while (! (x = fopen(vconf->tmpout, "w+") ) )
      v_error(vconf, LOG_ERR, "Could not open temporary outputfile %s", vconf->tmpout);
    v_error(vconf, LOG_DEBUG, "Opened temporary output-file %s", vconf->tmpout);
  } else {
    while (! (x = fopen(vconf->out, "w+") ) )
      v_error(vconf, LOG_ERR, "Could not open outputfile %s", vconf->out);
    v_error(vconf, LOG_DEBUG, "Opened output-file %s", vconf->out);
  }
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
    case 3:
      while (write_ppm(vconf, o_buffer, x))
	v_error(vconf, LOG_ERR, "Could not write outputfile %s", vconf->out);
      break;
    default:		// should never happen  
      v_error(vconf, LOG_CRIT, "Unknown outformat %d (should not happen!!)", vconf->outformat);
      break;
    }
  fclose(x);
  if ( vconf->tmpout ) {
    v_error(vconf, LOG_DEBUG, "Temporary outputfile %s closed", vconf->tmpout);
    system(vconf->cpyline);
    v_error(vconf, LOG_DEBUG, "Temporary output %s copied to final destination %s", vconf->tmpout, vconf->out);
  } else {
    v_error(vconf, LOG_DEBUG, "Outputfile %s closed", vconf->out);
  }
#ifdef LIBFTP
  if(vconf->ftp.enable == TRUE)
    ftp_upload(vconf);
#endif
  return;
}

