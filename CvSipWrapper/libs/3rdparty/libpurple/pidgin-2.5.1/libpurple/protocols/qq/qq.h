/**
 * @file qq.h
 *
 * purple
 *
 * Purple is the legal property of its developers, whose names are too numerous
 * to list here.  Please refer to the COPYRIGHT file distributed with this
 * source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111-1301  USA
 */

#ifndef _QQ_QQ_H_
#define _QQ_QQ_H_

#include <glib.h>
#include "internal.h"
#include "ft.h"
#include "circbuffer.h"
#include "dnsquery.h"
#include "dnssrv.h"
#include "proxy.h"
#include "roomlist.h"

#define QQ_KEY_LENGTH       16

typedef struct _qq_data qq_data;
typedef struct _qq_buddy qq_buddy;
typedef struct _qq_interval qq_interval;

struct _qq_interval {
	gint resend;
	gint keep_alive;
	gint update; 
};

struct _qq_buddy {
	guint32 uid;
	guint16 face;		/* index: 0 - 299 */
	guint8 age;
	guint8 gender;
	gchar *nickname;
	struct in_addr ip;
	guint16 port;
	guint8 status;
	guint8 ext_flag;
	guint8 comm_flag;	/* details in qq_buddy_list.c */
	guint16 client_version;
	guint8 onlineTime;
	guint16 level;
	guint16 timeRemainder;
	time_t signon;
	time_t idle;
	time_t last_refresh;

	gint8  role;		/* role in group, used only in group->members list */
};

struct _qq_data {
	PurpleConnection *gc;

	/* common network resource */
	GList *servers;
	gchar *user_server;
	gint user_port;
	gboolean use_tcp;		/* network in tcp or udp */
	
	gchar *server_name;
	gboolean is_redirect;
	gchar *real_hostname;	/* from real connction */
	guint16 real_port;
	guint reconnect_timeout;
	gint reconnect_times;

	PurpleProxyConnectData *connect_data;
	gint fd;				/* socket file handler */
	gint tx_handler; 	/* socket can_write handle, use in udp connecting and tcp send out */

	qq_interval itv_config;
	qq_interval itv_count;
	guint network_timeout;
	
	GList *transactions;	/* check ack packet and resend */

	/* tcp related */
	PurpleCircBuffer *tcp_txbuf;
	guint8 *tcp_rxqueue;
	int tcp_rxlen;
	
	/* udp related */
	PurpleDnsQueryData *udp_query_data;

	guint32 uid;			/* QQ number */
	guint8 *token;		/* get from server*/
	int token_len;
	guint8 inikey[QQ_KEY_LENGTH];			/* initial key to encrypt login packet */
	guint8 password_twice_md5[QQ_KEY_LENGTH];			/* password in md5 (or md5' md5) */
	guint8 session_key[QQ_KEY_LENGTH];		/* later use this as key in this session */
	guint8 session_md5[QQ_KEY_LENGTH];		/* concatenate my uid with session_key and md5 it */

	guint16 send_seq;		/* send sequence number */
	guint8 login_mode;		/* online of invisible */
	gboolean logged_in;		/* used by qq-add_buddy */

	PurpleXfer *xfer;			/* file transfer handler */

	/* get from login reply packet */
	time_t login_time;
	time_t last_login_time;
	gchar *last_login_ip;
	/* get from keep_alive packet */
	struct in_addr my_ip;			/* my ip address detected by server */
	guint16 my_port;		/* my port detected by server */
	guint16 my_icon;		/* my icon index */
	guint16 my_level;		/* my level */
	guint32 total_online;		/* the number of online QQ users */
	time_t last_get_online;		/* last time send get_friends_online packet */

	PurpleRoomlist *roomlist;
	gint channel;			/* the id for opened chat conversation */

	GList *groups;
	GSList *joining_groups;
	GSList *adding_groups_from_server; /* internal ids of groups the server wants in my blist */
	GList *buddies;
	GList *contact_info_window;
	GList *group_info_window;
	GList *info_query;
	GList *add_buddy_request;

	/* TODO pass qq_send_packet_get_info() a callback and use signals to get rid of these */
	gboolean modifying_info;
	gboolean modifying_face;
};

#endif
