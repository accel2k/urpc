/*
 * uRpc - rpc (remote procedure call) library.
 *
 * Copyright 2015 Andrei Fadeev
 *
 * This file is part of uRPC.
 *
 * uRPC is free software: you can redistribute it and/or modify
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
*/

#ifndef _urpc_semaphore_h
#define _urpc_semaphore_h

#include <urpc-exports.h>

#ifdef __cplusplus
extern "C" {
#endif


typedef enum { URPC_SEM_LOCKED = 0, URPC_SEM_UNLOCKED = 1 } uRpcSemStat;

typedef struct uRpcSem uRpcSem;


URPC_EXPORT uRpcSem *urpc_sem_create( const char *name, uRpcSemStat stat, int queue );
URPC_EXPORT uRpcSem *urpc_sem_open( const char *name );
URPC_EXPORT void urpc_sem_destroy( uRpcSem *sem );
URPC_EXPORT void urpc_sem_remove( const char *name );

URPC_EXPORT void urpc_sem_lock( uRpcSem *sem );
URPC_EXPORT int urpc_sem_trylock( uRpcSem *sem );
URPC_EXPORT int urpc_sem_timedlock( uRpcSem *sem, double time );
URPC_EXPORT void urpc_sem_unlock( uRpcSem *sem );


#ifdef __cplusplus
} // extern "C"
#endif

#endif // _urpc_semaphore_h
