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

#ifndef __URPC_EXPORTS_H__
#define __URPC_EXPORTS_H__

#if defined (_WIN32)
  #if defined (urpc_EXPORTS)
    #define URPC_EXPORT __declspec (dllexport)
  #else
    #define URPC_EXPORT __declspec (dllimport)
  #endif
#else
  #define URPC_EXPORT
#endif

#endif /* __URPC_EXPORTS_H__ */
