/*
 * Copyright (c) 2021 Ivan J. <parazyd@dyne.org>
 *
 * This file is part of tor-applet
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

/*
#define str(x) #x
#define xstr(x) str(x)
*/

#define TOR_DATADIRS  "/var/lib/tor-applet"
#define TOR_RUNDIRS   "/run/tor-applet"

#define GC_TOR         "/system/maemo/tor"
#define GC_TOR_ACTIVE  GC_TOR"/active_config"

#define GC_CFG_TPENABLED       "transproxy-enabled"
#define GC_CFG_SOCKSPORT       "socks-port"
#define GC_CFG_CONTROLPORT     "control-port"
#define GC_CFG_TRANSPORT       "trans-port"
#define GC_CFG_DNSPORT         "dns-port"
#define GC_CFG_DATADIR         "datadir"
#define GC_CFG_RUNDIR          "rundir"
#define GC_CFG_BRIDGES         "bridges"
#define GC_CFG_BRIDGESENABLED  "bridges-enabled"
#define GC_CFG_HS              "hiddenservices"
#define GC_CFG_HSENABLED       "hiddenservices-enabled"

#define BRIDGES_CFG \
	"UseBridges 1\n" \
	"ClientTransportPlugin obfs3 exec /usr/bin/obfsproxy managed\n" \
	"ClientTransportPlugin obfs4 exec /usr/bin/obfs4proxy managed"

/* TODO: Implement something with the below structs */
typedef enum {
	BRIDGE_OBFS3,
	BRIDGE_OBFS4,
	BRIDGE_MEEK,
	BRIDGE_SCRAMBLESUIT
} BridgeType;

typedef struct {
	BridgeType type;
	char *hostport;
	char *fingerprint;
	char *password;		/* Needed for BRIDGE_SCRAMBLESUIT */
} BridgeEntry;

typedef struct {
	char *hostport;
	int hs_port;
} HSPortMap;

typedef struct {
	char *hs_dir;
	HSPortMap *portmap[65535];
} HiddenService;

typedef struct {
	int socks_port;
	int control_port;
	int dns_port;
	int trans_port;
	BridgeEntry *bridges[10];
	HiddenService *hidden_services[10];
} TorConfiguration;

#endif
