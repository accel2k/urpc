/*
 * uRPC - rpc (remote procedure call) library.
 *
 * Copyright 2015 Andrei Fadeev (andrei@webcontrol.ru)
 *
 * This file is part of uRPC.
 *
 * uRPC is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * uRPC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 *
 * Alternatively, you can license this code under a commercial license.
 * Contact the author in this case.
 *
 */

#ifndef __URPC_ENDIAN_H__
#define __URPC_ENDIAN_H__

#include <stdint.h>

#if defined (__GNUC__)

#define UINT32_SWAP_LE_BE(val)         __builtin_bswap32 ((int32_t)val)
#define UINT64_SWAP_LE_BE(val)         __builtin_bswap64 ((int64_t)val)

#elif defined (_MSC_VER)

#include <stdlib.h>
#define UINT32_SWAP_LE_BE(val)         _byteswap_ulong  ((uint32_t)val)
#define UINT64_SWAP_LE_BE(val)         _byteswap_uint64 ((uint64_t)val)

#else

#define UINT32_SWAP_LE_BE(val)         ((uint32_t)( \
        (((uint32_t)(val) & (uint32_t)0x000000ffU) << 24) | \
        (((uint32_t)(val) & (uint32_t)0x0000ff00U) <<  8) | \
        (((uint32_t)(val) & (uint32_t)0x00ff0000U) >>  8) | \
        (((uint32_t)(val) & (uint32_t)0xff000000U) >> 24)))

#define UINT64_SWAP_LE_BE(val)         ((uint64_t)( \
        (((uint64_t)(val) & (uint64_t)0x00000000000000ffU) << 56) | \
        (((uint64_t)(val) & (uint64_t)0x000000000000ff00U) << 40) | \
        (((uint64_t)(val) & (uint64_t)0x0000000000ff0000U) << 24) | \
        (((uint64_t)(val) & (uint64_t)0x00000000ff000000U) <<  8) | \
        (((uint64_t)(val) & (uint64_t)0x000000ff00000000U) >>  8) | \
        (((uint64_t)(val) & (uint64_t)0x0000ff0000000000U) >> 24) | \
        (((uint64_t)(val) & (uint64_t)0x00ff000000000000U) >> 40) | \
        (((uint64_t)(val) & (uint64_t)0xff00000000000000U) >> 56)))

#endif


#if defined( URPC_BIG_ENDIAN )

#define INT32_TO_LE(val)               (UINT32_SWAP_LE_BE (val))
#define UINT32_TO_LE(val)              (UINT32_SWAP_LE_BE (val))
#define INT64_TO_LE(val)               (UINT64_SWAP_LE_BE (val))
#define UINT64_TO_LE(val)              (UINT64_SWAP_LE_BE (val))

#define INT32_FROM_LE(val)             (UINT32_SWAP_LE_BE (val))
#define UINT32_FROM_LE(val)            (UINT32_SWAP_LE_BE (val))
#define INT64_FROM_LE(val)             (UINT64_SWAP_LE_BE (val))
#define UINT64_FROM_LE(val)            (UINT64_SWAP_LE_BE (val))

#define INT32_TO_BE(val)               (val)
#define UINT32_TO_BE(val)              (val)
#define INT64_TO_BE(val)               (val)
#define UINT64_TO_BE(val)              (val)

#define INT32_FROM_BE(val)             (val)
#define UINT32_FROM_BE(val)            (val)
#define INT64_FROM_BE(val)             (val)
#define UINT64_FROM_BE(val)            (val)

#else

#define INT32_TO_LE(val)               (val)
#define UINT32_TO_LE(val)              (val)
#define INT64_TO_LE(val)               (val)
#define UINT64_TO_LE(val)              (val)

#define INT32_FROM_LE(val)             (val)
#define UINT32_FROM_LE(val)            (val)
#define INT64_FROM_LE(val)             (val)
#define UINT64_FROM_LE(val)            (val)

#define INT32_TO_BE(val)               (UINT32_SWAP_LE_BE (val))
#define UINT32_TO_BE(val)              (UINT32_SWAP_LE_BE (val))
#define INT64_TO_BE(val)               (UINT64_SWAP_LE_BE (val))
#define UINT64_TO_BE(val)              (UINT64_SWAP_LE_BE (val))

#define INT32_FROM_BE(val)             (UINT32_SWAP_LE_BE (val))
#define UINT32_FROM_BE(val)            (UINT32_SWAP_LE_BE (val))
#define INT64_FROM_BE(val)             (UINT64_SWAP_LE_BE (val))
#define UINT64_FROM_BE(val)            (UINT64_SWAP_LE_BE (val))

#endif

#endif /* __URPC_ENDIAN_H__ */
