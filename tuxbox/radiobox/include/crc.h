/*
 * madplay - MPEG audio decoder and player
 * Copyright (C) 2000-2004 Robert Leslie
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: crc.h,v 1.1 2006/06/05 18:15:17 the_moon Exp $
 */

#ifdef INCLUDE_UNUSED_STUFF
# ifndef __CRC_H__
# define __CRC_H__

unsigned short crc_compute(char const *, unsigned int, unsigned short);

# endif /* __CRC_H__ */
#endif /* INCLUDE_UNUSED_STUFF */