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

/* Includes */

static struct v_options l_opt[] = {
  /* Here's the whole definition of options of vgrabbj with needed information: (it fits MY screen ;-)
   *
   * arg_type: 1=int, 2=longint, 3=bool, 4=char, 0=unspecified
   * art_req: 1=required, 2=none, 3=optional, 0=unspecified
   *
   * Long Option,  short, arg req, which variable,arg_type,Minimum,  Maximum,      MaxLen
   */
  {"DebugLevel",      "D", req,  NULL, opt_int,     MIN_DEBUG,    MAX_DEBUG,    0  },
  {"Daemon",          "L", req,  NULL, opt_longint, MIN_LOOP,     0,            0  },
  {"DaemonSeconds",   "l", req,  NULL, opt_int_s,   MIN_LOOP,     0,            0  },
  {"Brightness",      "b", none, NULL, opt_bool,    MIN_BOOL,     MAX_BOOL,     0  },
  {"ImageQuality",    "q", req,  NULL, opt_int,     MIN_QUALITY,  MAX_QUALITY,  0  },
  {"ImageSize",       "i", req,  NULL, opt_size,    0,            0,            0  },
  {"ImageWidth",      "w", req,  NULL, opt_int,     MIN_SIZE,     MAX_SIZE,     0  },
  {"ImageHeight",     "H", req,  NULL, opt_int,     MIN_SIZE,     MAX_SIZE,     0  },
  {"OutputFormat",    "o", req,  NULL, opt_format,  MIN_FORMAT,   MAX_FORMAT,   0  },
  {"OutputFile",      "f", req,  NULL, opt_charptr, 0,            0,            255},
  {"VideoDevice",     "d", req,  NULL, opt_charptr, 0,            0,            255},
  {"OpenOnce",        "C", none, NULL, opt_bool,    MIN_BOOL,     MAX_BOOL,     0  },
  {"SwitchColor",     "S", none, NULL, opt_bool,    MIN_BOOL,     MAX_BOOL,     0  },
  {"SetImageSize",    "g", none, NULL, opt_bool,    MIN_BOOL,     MAX_BOOL,     0  },
  {"DiscardFrames",   "z", req,  NULL, opt_int,     MIN_DISCARD,  MAX_DISCARD,  0  },
  {"ForcePalette",    "F", req,  NULL, opt_int,     MIN_PALETTE,  MAX_PALETTE,  0  },
  {"UseTmpOut",       "n", none, NULL, opt_bool,    MIN_BOOL,     MAX_BOOL,     0  },
  {"UseTimeStamp",    "e", none, NULL, opt_bool,    MIN_BOOL,     MAX_BOOL,     0  },
#ifdef LIBTTF
  {"FontFile",        "t", req,  NULL, opt_charptr, 0,            0,            255},
  {"TimeStamp",       "p", req,  NULL, opt_charptr, 0,            0,            255},
  {"FontSize",        "T", req,  NULL, opt_int,     MIN_FONTSIZE, MAX_FONTSIZE, 0  },
  {"Position",        "a", req,  NULL, opt_position,MIN_ALIGN,    MAX_ALIGN,    0  },
  {"Blend",           "m", req,  NULL, opt_int,     MIN_BLEND,    MAX_BLEND,    0  },
  {"BorderSize",      "B", req,  NULL, opt_int,     MIN_BORDER,   MAX_BORDER,   0  },
#endif
#ifdef LIBFTP
  {"EnableFtp",       NULL,none, NULL, opt_bool,    MIN_BOOL,     MAX_BOOL,     0  },
  {"RemoteHost",      NULL,none, NULL, opt_charptr, 0,            0,            255},
  {"RemoteImageName", NULL,none, NULL, opt_charptr, 0,            0,            255},
  {"Username",        NULL,none, NULL, opt_charptr, 0,            0,            255},
  {"Password",        NULL,none, NULL, opt_charptr, 0,            0,            255},
  {"KeepAlive",       NULL,none, NULL, opt_bool,    MIN_BOOL,     MAX_BOOL,     255},
  {"TryHarder",       NULL,none, NULL, opt_int,     0,            0,            0  },
  {"RemoteDir",       NULL,none, NULL, opt_charptr, 0,            0,            255},
#endif
    /* and here's the commandline options which have no conf-file equivalent */
  {NULL,              "c", none, NULL, opt_conf,    0,            0,            0  },
  {NULL,              "V", none, NULL, opt_version, 0,            0,            0  },
  {NULL,              "s", req,  NULL, opt_setting, 0,            0,            255},
  {NULL,              "h", none, NULL, opt_help,    0,            0,            0  },
  /* New options are to be added in front of this line, which serves as a    */
  /* marker for the config-parser (no long- and no short-option)             */
  {"Archive",         "A", req,  NULL, opt_charptr, 0,            0,            255},
  {NULL, NULL, none, NULL, 0, 0, 0, 0}
};
