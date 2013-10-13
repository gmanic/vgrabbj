/* Simple Video4Linux image grabber. Made for my Philips Vesta Pro
 *
 * Copyright (C) 2000, 2001 Robert Wessels, Hengelo, The Netherlands
 * eMail: techie@GrassAndCows.eu.org
 * Certain Changes (C) 2001 Jens Gecius, devel@gecius.de
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


#include "v_ftp.h"

#ifdef LIBFTP

static char mode = 'I';
static netbuf *conn = NULL;

void ftp_upload(struct vconfig *vconf){
  switch(vconf->ftp.state){
    case STATE_UNINITIALIZED:
      FtpInit();
      vconf->ftp.state = STATE_CONNECT;
      v_error(vconf, LOG_INFO, "ftp state 0: upload initialized");
      ftp_upload(vconf);
      break;
    case STATE_CONNECT:
      if(conn)
	  FtpClose(conn);
      if(FtpConnect( vconf->ftp.remoteHost, &conn)){
	if (vconf->ftp.passive) {
	    FtpOptions(FTPLIB_CONNMODE, FTPLIB_PASSIVE, conn);
	} else {
	    FtpOptions(FTPLIB_CONNMODE, FTPLIB_PORT, conn);
	}
        vconf->ftp.state = STATE_LOGIN;
        v_error(vconf, LOG_INFO, "ftp state 1: connection successfull");
      } else {
        v_error(vconf, LOG_WARNING, "ftp state 1: connection failed with reason: %s", FtpLastResponse(conn));
	return;
      }
      ftp_upload(vconf);
      break;
    case STATE_LOGIN:
      if(FtpLogin( vconf->ftp.username, vconf->ftp.password, conn)){
        vconf->ftp.state = STATE_CHDIR;
        v_error(vconf, LOG_INFO, "ftp state 2: login successfull");
      } else {
        v_error(vconf, LOG_WARNING, "ftp state 2: login failed with reason: %s", FtpLastResponse(conn));
	return;
      }
      ftp_upload(vconf);
      break;
    case STATE_CHDIR:
      if (vconf->ftp.remoteDir) {
	if(FtpChdir(vconf->ftp.remoteDir, conn)){
	  vconf->ftp.state = STATE_PUT;
	  v_error(vconf, LOG_INFO, "ftp state 3: chdir successfull");
	} else {
	  v_error(vconf, LOG_WARNING, "ftp state 3: chdir failed with reason: %s", FtpLastResponse(conn));
	  return;
	}
      }
      else {
	vconf->ftp.state = STATE_PUT;
	v_error(vconf, LOG_INFO, "fpt state 3: no chdir");
      }
      ftp_upload(vconf);
      break;
    case STATE_PUT:
      if(FtpPut(vconf->out, "vgrabbj.tmp", mode, conn)){
        vconf->ftp.state = STATE_RENAME;
        v_error(vconf, LOG_INFO, "ftp state 4: image upload successfull");
      } else {
        v_error(vconf, LOG_WARNING, "ftp state 4: image upload failed with reason: %s", FtpLastResponse(conn));
	return;
      }
      ftp_upload(vconf);
      break;
    case STATE_RENAME:
      if(FtpDelete(vconf->ftp.remoteImageName, conn)){
	  v_error(vconf, LOG_INFO, "ftp state 5: delete previous image completed");
      } else {
	  v_error(vconf, LOG_WARNING, "ftp state 5: delete previous image failed");
      }
      if(FtpRename("vgrabbj.tmp", vconf->ftp.remoteImageName, conn)){
        vconf->ftp.state = STATE_FINISH;
        v_error(vconf, LOG_INFO, "ftp state 5: rename image successfull");
      } else {
        v_error(vconf, LOG_WARNING, "ftp state 5: rename image failed with reason %s",FtpLastResponse(conn));
	return;
      }
      ftp_upload(vconf);
      break;
    case STATE_FINISH:
      if(vconf->loop && vconf->ftp.keepalive){
	  vconf->ftp.state = STATE_PUT;
      } else {
	if (conn)
	  FtpClose(conn);
	vconf->ftp.state = STATE_CONNECT;
      }
      v_error(vconf, LOG_INFO, "ftp state 6: upload complete");
      break;
    default:
      /* this should never happen */
      if(conn)
	  FtpClose(conn);
      vconf->ftp.state = STATE_CONNECT;
      break;
  }
  return;
}

#endif
