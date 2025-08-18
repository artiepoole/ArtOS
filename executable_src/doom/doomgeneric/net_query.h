// ArtOS - hobby operating system by Artie Poole
// Copyright (C) 2025 Stuart Forbes Poole <artiepoole>
//
//     This program is free software: you can redistribute it and/or modify
//     it under the terms of the GNU General Public License as published by
//     the Free Software Foundation, either version 3 of the License, or
//     (at your option) any later version.
//
//     This program is distributed in the hope that it will be useful,
//     but WITHOUT ANY WARRANTY; without even the implied warranty of
//     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//     GNU General Public License for more details.
//
//     You should have received a copy of the GNU General Public License
//     along with this program.  If not, see <https://www.gnu.org/licenses/>

#ifndef NET_QUERY_H
#define NET_QUERY_H

#include "net_defs.h"

typedef void (*net_query_callback_t)(net_addr_t* addr,
                                     net_querydata_t* querydata,
                                     unsigned int ping_time,
                                     void* user_data);

extern int NET_StartLANQuery(void);
extern int NET_StartMasterQuery(void);

extern void NET_LANQuery(void);
extern void NET_MasterQuery(void);
extern void NET_QueryAddress(char* addr);
extern net_addr_t* NET_FindLANServer(void);

extern int NET_Query_Poll(net_query_callback_t callback, void* user_data);

extern net_addr_t* NET_Query_ResolveMaster(net_context_t* context);
extern void NET_Query_AddToMaster(net_addr_t* master_addr);
extern boolean NET_Query_CheckAddedToMaster(boolean* result);
extern void NET_Query_MasterResponse(net_packet_t* packet);

#endif /* #ifndef NET_QUERY_H */
