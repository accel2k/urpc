/*
 * uRPC - rpc (remote procedure call) library.
 *
 * Copyright 2009-2015 Andrei Fadeev (andrei@webcontrol.ru)
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

#include "urpc-network.h"


static struct {
  int errnum;
  const char *desc;
} urpc_network_win_errors [] = {
  { 1, "Unknown error" },
  { WSABASEERR, "No error" },
  { WSAEACCES, "Permission denied" },
  { WSAEADDRINUSE, "Address already in use" },
  { WSAEADDRNOTAVAIL, "Cannot assign requested address" },
  { WSAEAFNOSUPPORT, "Address family not supported by protocol family" },
  { WSAEALREADY, "Operation already in progress" },
  { WSAEBADF, "Bad file descriptor" },
  { WSAECONNABORTED, "Software caused connection abort" },
  { WSAECONNREFUSED, "Connection refused" },
  { WSAECONNRESET, "Connection reset by peer" },
  { WSAEDESTADDRREQ, "Destination address required" },
  { WSAEDQUOT, "Disc quota exceeded" },
  { WSAEFAULT, "Bad address" },
  { WSAEHOSTDOWN, "Host is down" },
  { WSAEHOSTUNREACH, "No route to host" },
  { WSAEINPROGRESS, "Operation now in progress" },
  { WSAEINTR, "Interrupted function call" },
  { WSAEINVAL, "Invalid argument" },
  { WSAEISCONN, "Socket is already connected" },
  { WSAELOOP, "Too many levels of symbolic links" },
  { WSAEMFILE, "Too many open files" },
  { WSAEMSGSIZE, "Message too long" },
  { WSAENAMETOOLONG, "File name too long" },
  { WSAENETDOWN, "Network is down" },
  { WSAENETRESET, "Network dropped connection on reset" },
  { WSAENETUNREACH, "Network is unreachable" },
  { WSAENOBUFS, "No buffer space available" },
  { WSAENOPROTOOPT, "Bad protocol option" },
  { WSAENOTCONN, "Socket is not connected" },
  { WSAENOTEMPTY, "Directory not empty" },
  { WSAENOTSOCK, "Socket operation on non-socket" },
  { WSAEOPNOTSUPP, "Operation not supported" },
  { WSAEPFNOSUPPORT, "Protocol family not supported" },
  { WSAEPROCLIM, "Too many processes" },
  { WSAEPROTONOSUPPORT, "Protocol not supported" },
  { WSAEPROTOTYPE, "Protocol wrong type for socket" },
  { WSAEREMOTE, "Too many levels of remote in path" },
  { WSAESHUTDOWN, "Cannot send after socket shutdown" },
  { WSAESOCKTNOSUPPORT, "Socket type not supported" },
  { WSAESTALE, "Stale NFS file handle" },
  { WSAETIMEDOUT, "Connection timed out" },
  { WSAETOOMANYREFS, "Too many references; can't splice" },
  { WSAEUSERS, "Too many users" },
  { WSAEWOULDBLOCK, "Resource temporarily unavailable" },
  { WSAHOST_NOT_FOUND, "Host not found" },
  { WSANOTINITIALISED, "Successful WSAStartup() not yet performed" },
  { WSANO_DATA, "Valid name, no data record of requested type" },
  { WSANO_RECOVERY, "This is a non-recoverable error" },
  { WSASYSNOTREADY, "Network subsystem is unavailable" },
  { WSATRY_AGAIN, "Non-authoritative host not found" },
  { WSAVERNOTSUPPORTED, "WINSOCKDLL version out of range" },
  { WSABASEERR, "No Error" },
  { WSAEINTR, "Interrupted system call" },
  { WSAEBADF, "Bad file number" },
  { WSAEACCES, "Permission denied" },
  { WSAEFAULT, "Bad address" },
  { WSAEINVAL, "Invalid argument" },
  { WSAEMFILE, "Too many open files" },
  { WSAEWOULDBLOCK, "Operation would block" },
  { WSAEINPROGRESS, "Operation now in progress" },
  { WSAEALREADY, "Operation already in progress" },
  { WSAENOTSOCK, "Socket operation on non-socket" },
  { WSAEDESTADDRREQ, "Destination address required" },
  { WSAEMSGSIZE, "Message too long" },
  { WSAEPROTOTYPE, "Protocol wrong type for socket" },
  { WSAENOPROTOOPT, "Bad protocol option" },
  { WSAEPROTONOSUPPORT, "Protocol not supported" },
  { WSAESOCKTNOSUPPORT, "Socket type not supported" },
  { WSAEOPNOTSUPP, "Operation not supported on socket" },
  { WSAEPFNOSUPPORT, "Protocol family not supported" },
  { WSAEAFNOSUPPORT, "Address family not supported by protocol family" },
  { WSAEADDRINUSE, "Address already in use" },
  { WSAEADDRNOTAVAIL, "Can't assign requested address" },
  { WSAENETDOWN, "Network is down" },
  { WSAENETUNREACH, "Network is unreachable" },
  { WSAENETRESET, "Net dropped connection or reset" },
  { WSAECONNABORTED, "Software caused connection abort" },
  { WSAECONNRESET, "Connection reset by peer" },
  { WSAENOBUFS, "No buffer space available" },
  { WSAEISCONN, "Socket is already connected" },
  { WSAENOTCONN, "Socket is not connected" },
  { WSAESHUTDOWN, "Can't send after socket shutdown" },
  { WSAETOOMANYREFS, "Too many references, can't splice" },
  { WSAETIMEDOUT, "Connection timed out" },
  { WSAECONNREFUSED, "Connection refused" },
  { WSAELOOP, "Too many levels of symbolic links" },
  { WSAENAMETOOLONG, "File name too long" },
  { WSAEHOSTDOWN, "Host is down" },
  { WSAEHOSTUNREACH, "No Route to Host" },
  { WSAENOTEMPTY, "Directory not empty" },
  { WSAEPROCLIM, "Too many processes" },
  { WSAEUSERS, "Too many users" },
  { WSAEDQUOT, "Disc Quota Exceeded" },
  { WSAESTALE, "Stale NFS file handle" },
  { WSASYSNOTREADY, "Network SubSystem is unavailable" },
  { WSAVERNOTSUPPORTED, "WINSOCK DLL Version out of range" },
  { WSANOTINITIALISED, "Successful WSASTARTUP not yet performed" },
  { WSAEREMOTE, "Too many levels of remote in path" },
  { WSAHOST_NOT_FOUND, "Host not found" },
  { WSATRY_AGAIN, "Non-Authoritative Host not found" },
  { WSANO_RECOVERY, "Non-Recoverable errors: FORMERR, REFUSED, NOTIMP" },
  { WSANO_DATA, "Valid name, no data record of requested type" },
  { WSANO_ADDRESS, "No address, look for MX record" },
  { 0, NULL }
};


int urpc_network_init( void )
{

  WSADATA wsa_data;

  if( WSAStartup( MAKEWORD( 2, 2 ), &wsa_data ) ) return -1;
  return 0;

}

void urpc_network_close( void )
{

  WSACleanup();

}


int urpc_network_last_error( void )
{

  return WSAGetLastError();

}


const char* urpc_network_last_error_str( void )
{

  int i = 0;
  int network_error = WSAGetLastError();

  while( urpc_network_win_errors[i].errnum )
    {
    if( urpc_network_win_errors[i].errnum == network_error )
      return urpc_network_win_errors[i].desc;
    i += 1;
    }

  return urpc_network_win_errors[0].desc;

}
