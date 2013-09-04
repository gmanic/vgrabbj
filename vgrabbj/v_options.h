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

/* Includes */

struct v_options l_opt[] = {
  /* Here's the whole definition of options of vgrabbj with needed information: (it fits MY screen ;-)
   *
   * arg_type: 1=int, 2=longint, 3=bool, 4=char, 0=unspecified
   * art_req: 1=required, 2=none, 3=optional, 0=unspecified
   *
   * Long Option,  short,argreq,which var,arg_type,Min,  Max,MaxLen,Default
   */
  {"DebugLevel",      "D", req,  NULL, opt_int,     0,     7,   0, "4", NULL},
  {"Daemon",          "L", req,  NULL, opt_longint, 0,     0,   0, "0", NULL},
  {"DaemonSeconds",   "l", req,  NULL, opt_int_s,   0,     0,   0, "0", NULL},
  {"AutoBrightness",  "a", none, NULL, opt_bool,    0,     1,   0, "Off", NULL},
  {"ImageQuality",    "q", req,  NULL, opt_int,     0,   100,   0, "75", NULL},
  {"ImageSize",       "i", req,  NULL, opt_size,    0,     0,   0, "cif", NULL},
  {"ImageWidth",      "w", req,  NULL, opt_int,     0, 65535,   0, "352", NULL},
  {"ImageHeight",     "H", req,  NULL, opt_int,     0, 65535,   0, "288", NULL},
  {"OutputFormat",    "o", req,  NULL, opt_format,  0,     3,   0, "jpg", NULL},
  {"OutputFile",      "f", req,  NULL, opt_charptr, 0,     0, 255, "/dev/stdout", NULL},
  {"VideoDevice",     "d", req,  NULL, opt_charptr, 0,     0, 255, "/dev/video", NULL},
  {"OpenOnce",        "C", none, NULL, opt_bool,    0,     1,   0, "On", NULL},
  {"SwitchColor",     "S", none, NULL, opt_bool,    0,     1,   0, "Off", NULL},
  {"SetImageSize",    "g", none, NULL, opt_bool,    0,     1,   0, "On", NULL},
  {"DiscardFrames",   "z", req,  NULL, opt_int,     0,   255,   0, "Off", NULL},
  {"ForcePalette",    "F", req,  NULL, opt_int,     1,    16,   0, "0", NULL},
  {"UseTmpOut",       "n", none, NULL, opt_bool,    0,     1,   0, "On", NULL},
  {"UseTimeStamp",    "e", none, NULL, opt_bool,    0,     1,   0, "Off", NULL},
#ifdef LIBTTF
  {"FontFile",        "t", req,  NULL, opt_charptr, 0,     0, 255, "/usr/share/fonts/truetype/Arialn.ttf", NULL},
  {"TimeStamp",       "p", req,  NULL, opt_charptr, 0,     0, 255, "%A, %e. %B %Y - %T", NULL},
  {"FontSize",        "T", req,  NULL, opt_int,     3,   100,   0, "12", NULL},
  {"Position",        "P", req,  NULL, opt_position,0,     5,   0, "upperright", NULL},
  {"Blend",           "m", req,  NULL, opt_int,     1,   100,   0, "60", NULL},
  {"BorderSize",      "B", req,  NULL, opt_int,     0,   255,   0, "2", NULL},
#endif
#ifdef LIBFTP
  {"EnableFtp",       NULL,none, NULL, opt_bool,    0,     1,   0, "Off", NULL},
  {"RemoteHost",      NULL,none, NULL, opt_charptr, 0,     0, 255, "ftp.foo.bar", NULL},
  {"RemoteImageName", NULL,none, NULL, opt_charptr, 0,     0, 255, "camimage.jpg", NULL},
  {"Username",        NULL,none, NULL, opt_charptr, 0,     0, 255, "foo", NULL},
  {"Password",        NULL,none, NULL, opt_charptr, 0,     0, 255, "bar", NULL},
  {"KeepAlive",       NULL,none, NULL, opt_bool,    0,     1,   0, "Off", NULL},
  {"TryHarder",       NULL,none, NULL, opt_int,     0,     0,   0, "2", NULL},
  {"RemoteDir",       NULL,none, NULL, opt_charptr, 0,     0, 255, "/", NULL},
  {"Passive",         NULL,none, NULL, opt_bool,    MIN_BOOL,     MAX_BOOL,     0, NULL, NULL  },
#endif
    /* and here's the commandline options which have no conf-file equivalent */
  {NULL,              "c", req,  NULL, opt_conf,    0,     0,   0, NULL, NULL},
  {NULL,              "V", none, NULL, opt_version, 0,     0,   0, NULL, NULL},
  {NULL,              "s", req,  NULL, opt_setting, 0,     0, 255, NULL, NULL},
  {NULL,              "h", none, NULL, opt_help,    0,     0,   0, NULL, NULL},
  {"Hue",             "u", req,  NULL, opt_int,    -1, 65535,   0, "-1", NULL},
  {"Brightness",      "b", req,  NULL, opt_int,    -1, 65535,   0, "-1", NULL},
  {"Contrast",        "x", req,  NULL, opt_int,    -1, 65535,   0, "-1", NULL},
  {"Colour",          "r", req,  NULL, opt_int,    -1, 65535,   0, "-1", NULL},
  {"Whiteness",       "W", req,  NULL, opt_int,    -1, 65535,   0, "-1", NULL},
  /* New options are to be added in front of this line, which serves as a    */
  /* marker for the config-parser (no long- and no short-option)             */
  {"Archive",         "A", req,  NULL, opt_charptr, 0,     0, 255, "/tmp/arch/cam-%Y-%d-%m-%H-%M-%S.jpg", NULL},
  {"ArchiveEach",     "E", req,  NULL, opt_int,     0, 65535,   0, "10", NULL},
  {"ArchiveMax",      "M", req,  NULL, opt_int,     0, 65535,   0, "100", NULL},
  {"SwapRL",          "R", none, NULL, opt_bool,    0,     1,   0, "Off", NULL},
  {"NoUseMmap",       "G", none, NULL, opt_bool,    0,     1,   0, "Off", NULL},
  {"SwapTB",          "U", none, NULL, opt_bool,    0,     1,   0, "Off", NULL},
  {NULL,              "X", none, NULL, opt_bool,    0,     1,   0, "Off", NULL},
  {NULL, NULL, none, NULL, 0, 0, 0, 0, NULL, NULL}
};
