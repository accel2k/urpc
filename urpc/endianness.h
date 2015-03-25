/* Based on gtypes.h file from GLIB library.
 *
 * GLIB - Library of useful routines for C programming
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 *
 */


#define UINT32_SWAP_LE_BE(val)	((uint32_t) ( \
    (((uint32_t) (val) & (uint32_t) 0x000000ffU) << 24) | \
    (((uint32_t) (val) & (uint32_t) 0x0000ff00U) <<  8) | \
    (((uint32_t) (val) & (uint32_t) 0x00ff0000U) >>  8) | \
    (((uint32_t) (val) & (uint32_t) 0xff000000U) >> 24)))


#define UINT64_SWAP_LE_BE(val)	((uint64_t) ( \
      (((uint64_t) (val) &                          \
        (uint64_t) (0x00000000000000ffU)) << 56) |  \
      (((uint64_t) (val) &                          \
        (uint64_t) (0x000000000000ff00U)) << 40) |  \
      (((uint64_t) (val) &                          \
        (uint64_t) (0x0000000000ff0000U)) << 24) |  \
      (((uint64_t) (val) &                          \
        (uint64_t) (0x00000000ff000000U)) <<  8) |  \
      (((uint64_t) (val) &                          \
        (uint64_t) (0x000000ff00000000U)) >>  8) |  \
      (((uint64_t) (val) &                          \
        (uint64_t) (0x0000ff0000000000U)) >> 24) |  \
      (((uint64_t) (val) &                          \
        (uint64_t) (0x00ff000000000000U)) >> 40) |  \
      (((uint64_t) (val) &                          \
        (uint64_t) (0xff00000000000000U)) >> 56)))


#define INT32_TO_BE(val)      (UINT32_SWAP_LE_BE (val))
#define UINT32_TO_BE(val)     (UINT32_SWAP_LE_BE (val))
#define INT64_TO_BE(val)      (UINT64_SWAP_LE_BE (val))
#define UINT64_TO_BE(val)     (UINT64_SWAP_LE_BE (val))

#define INT32_FROM_BE(val)    (UINT32_SWAP_LE_BE (val))
#define UINT32_FROM_BE(val)   (UINT32_SWAP_LE_BE (val))
#define INT64_FROM_BE(val)    (UINT64_SWAP_LE_BE (val))
#define UINT64_FROM_BE(val)   (UINT64_SWAP_LE_BE (val))
