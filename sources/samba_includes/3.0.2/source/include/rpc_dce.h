/* 
   Unix SMB/CIFS implementation.
   SMB parameters and setup
   Copyright (C) Andrew Tridgell 1992-1997
   Copyright (C) Luke Kenneth Casson Leighton 1996-1997
   Copyright (C) Paul Ashton 1997
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef _DCE_RPC_H /* _DCE_RPC_H */
#define _DCE_RPC_H 

#include "rpc_misc.h"  /* this only pulls in STRHDR */


/* DCE/RPC packet types */

enum RPC_PKT_TYPE
{
	RPC_REQUEST = 0x00,
	RPC_RESPONSE = 0x02,
	RPC_FAULT    = 0x03,
	RPC_BIND     = 0x0B,
	RPC_BINDACK  = 0x0C,
	RPC_BINDNACK = 0x0D,
	RPC_ALTCONT  = 0x0E,
	RPC_ALTCONTRESP = 0x0F,
	RPC_BINDRESP = 0x10 /* not the real name!  this is undocumented! */
};

/* DCE/RPC flags */
#define RPC_FLG_FIRST 0x01
#define RPC_FLG_LAST  0x02
#define RPC_FLG_NOCALL 0x20

#define SMBD_NTLMSSP_NEG_FLAGS 0x000082b1 /* ALWAYS_SIGN|NEG_NTLM|NEG_LM|NEG_SEAL|NEG_SIGN|NEG_UNICODE */

/* NTLMSSP signature version */
#define NTLMSSP_SIGN_VERSION 0x01

/* NTLMSSP auth type */
#define NTLMSSP_AUTH_TYPE 0xa

/* DCE-RPC standard identifiers to indicate 
   signing or sealing of an RPC pipe */
#define RPC_PIPE_AUTH_SIGN_LEVEL 0x5
#define RPC_PIPE_AUTH_SEAL_LEVEL 0x6

/* Netlogon schannel auth type and level */
#define NETSEC_AUTH_TYPE 0x44
#define NETSEC_SIGN_SIGNATURE { 0x77, 0x00, 0xff, 0xff, 0xff, 0xff, 0x00, 0x00 }
#define NETSEC_SEAL_SIGNATURE { 0x77, 0x00, 0x7a, 0x00, 0xff, 0xff, 0x00, 0x00 }
#define RPC_AUTH_NETSEC_CHK_LEN 0x20

/* The 7 here seems to be required to get Win2k not to downgrade us
   to NT4.  Actually, anything other than 1ff would seem to do... */
#define NETLOGON_NEG_AUTH2_FLAGS 0x000701ff
 
#define NETLOGON_NEG_SCHANNEL    0x40000000

enum netsec_direction
{
	SENDER_IS_INITIATOR,
	SENDER_IS_ACCEPTOR
};

/* Internal Flags to indicate what type of authentication on the pipe */
#define AUTH_PIPE_SIGN    0x0001
#define AUTH_PIPE_SEAL    0x0002
#define AUTH_PIPE_NTLMSSP 0x0004
#define AUTH_PIPE_NETSEC  0x0008

/* Maximum PDU fragment size. */
/* #define MAX_PDU_FRAG_LEN 0x1630		this is what wnt sets */
#define MAX_PDU_FRAG_LEN 0x10b8			/* this is what w2k sets */

/*
 * Actual structure of a DCE UUID
 */

typedef struct rpc_uuid
{
  uint32 time_low;
  uint16 time_mid;
  uint16 time_hi_and_version;
  uint8 remaining[8];
} RPC_UUID;

#define RPC_UUID_LEN 16

/* RPC_IFACE */
typedef struct rpc_iface_info
{
  RPC_UUID uuid;    /* 16 bytes of rpc interface identification */
  uint32 version;    /* the interface version number */

} RPC_IFACE;

#define RPC_IFACE_LEN (RPC_UUID_LEN + 4)

struct pipe_id_info
{
	/* the names appear not to matter: the syntaxes _do_ matter */

	const char *client_pipe;
	RPC_IFACE abstr_syntax; /* this one is the abstract syntax id */

	const char *server_pipe;  /* this one is the secondary syntax name */
	RPC_IFACE trans_syntax; /* this one is the primary syntax id */
};

/* RPC_HDR - dce rpc header */
typedef struct rpc_hdr_info
{
  uint8  major; /* 5 - RPC major version */
  uint8  minor; /* 0 - RPC minor version */
  uint8  pkt_type; /* RPC_PKT_TYPE - RPC response packet */
  uint8  flags; /* DCE/RPC flags */
  uint8  pack_type[4]; /* 0x1000 0000 - little-endian packed data representation */
  uint16 frag_len; /* fragment length - data size (bytes) inc header and tail. */
  uint16 auth_len; /* 0 - authentication length  */
  uint32 call_id; /* call identifier.  matches 12th uint32 of incoming RPC data. */

} RPC_HDR;

#define RPC_HEADER_LEN 16

/* RPC_HDR_REQ - ms request rpc header */
typedef struct rpc_hdr_req_info
{
  uint32 alloc_hint;   /* allocation hint - data size (bytes) minus header and tail. */
  uint16 context_id;   /* presentation context identifier */
  uint16  opnum;       /* opnum */

} RPC_HDR_REQ;

#define RPC_HDR_REQ_LEN 8

/* RPC_HDR_RESP - ms response rpc header */
typedef struct rpc_hdr_resp_info
{
  uint32 alloc_hint;   /* allocation hint - data size (bytes) minus header and tail. */
  uint16 context_id;   /* 0 - presentation context identifier */
  uint8  cancel_count; /* 0 - cancel count */
  uint8  reserved;     /* 0 - reserved. */

} RPC_HDR_RESP;

#define RPC_HDR_RESP_LEN 8

/* RPC_HDR_FAULT - fault rpc header */
typedef struct rpc_hdr_fault_info
{
  NTSTATUS status;
  uint32 reserved; /* 0x0000 0000 */
} RPC_HDR_FAULT;

#define RPC_HDR_FAULT_LEN 8

/* this seems to be the same string name depending on the name of the pipe,
 * but is more likely to be linked to the interface name
 * "srvsvc", "\\PIPE\\ntsvcs"
 * "samr", "\\PIPE\\lsass"
 * "wkssvc", "\\PIPE\\wksvcs"
 * "NETLOGON", "\\PIPE\\NETLOGON"
 */
/* RPC_ADDR_STR */
typedef struct rpc_addr_info
{
  uint16 len;   /* length of the string including null terminator */
  fstring str; /* the string above in single byte, null terminated form */

} RPC_ADDR_STR;

/* RPC_HDR_BBA */
typedef struct rpc_hdr_bba_info
{
  uint16 max_tsize;       /* maximum transmission fragment size (0x1630) */
  uint16 max_rsize;       /* max receive fragment size (0x1630) */
  uint32 assoc_gid;       /* associated group id (0x0) */

} RPC_HDR_BBA;

#define RPC_HDR_BBA_LEN 8

/* RPC_HDR_AUTHA */
typedef struct rpc_hdr_autha_info
{
	uint16 max_tsize;       /* maximum transmission fragment size (0x1630) */
	uint16 max_rsize;       /* max receive fragment size (0x1630) */

	uint8 auth_type; /* 0x0a */
	uint8 auth_level; /* 0x06 */
	uint8 stub_type_len; /* don't know */
	uint8 padding; /* padding */

	uint32 unknown; /* 0x0014a0c0 */

} RPC_HDR_AUTHA;

#define RPC_HDR_AUTHA_LEN 12

/* RPC_HDR_AUTH */
typedef struct rpc_hdr_auth_info
{
	uint8 auth_type; /* 0x0a */
	uint8 auth_level; /* 0x06 */
	uint8 padding;
	uint8 reserved; /* padding */

	uint32 auth_context; /* pointer */

} RPC_HDR_AUTH;

#define RPC_HDR_AUTH_LEN 8

/* this is TEMPORARILY coded up as a specific structure */
/* this structure comes after the bind request */
/* RPC_AUTH_NETSEC_NEG */
typedef struct rpc_auth_netsec_neg_info
{
	uint32 type1; 	/* Always zero ? */
	uint32 type2;	/* Types 0x3 and 0x13 seen. Check AcquireSecurityContext() docs.... */
	fstring domain; /* calling workstations's domain */
	fstring myname; /* calling workstation's name */
} RPC_AUTH_NETSEC_NEG;

/* attached to the end of encrypted rpc requests and responses */
/* RPC_AUTH_NETSEC_CHK */
typedef struct rpc_auth_netsec_chk_info
{
	uint8 sig  [8]; /* 77 00 7a 00 ff ff 00 00 */
	uint8 packet_digest[8]; /* checksum over the packet, MD5'ed with session key */
	uint8 seq_num[8]; /* verifier, seq num */
	uint8 confounder[8]; /* random 8-byte nonce */
} RPC_AUTH_NETSEC_CHK;

struct netsec_auth_struct
{
	uchar sess_key[16];
	uint32 seq_num;
	int auth_flags;
};

/* RPC_BIND_REQ - ms req bind */
typedef struct rpc_bind_req_info
{
  RPC_HDR_BBA bba;

  uint32 num_elements;    /* the number of elements (0x1) */
  uint16 context_id;      /* presentation context identifier (0x0) */
  uint8 num_syntaxes;     /* the number of syntaxes (has always been 1?)(0x1) */

  RPC_IFACE abstract;     /* num and vers. of interface client is using */
  RPC_IFACE transfer;     /* num and vers. of interface to use for replies */
  
} RPC_HDR_RB;

/* 
 * The following length is 8 bytes RPC_HDR_BBA_LEN, 8 bytes internals 
 * (with 3 bytes padding), + 2 x RPC_IFACE_LEN bytes for RPC_IFACE structs.
 */

#define RPC_HDR_RB_LEN (RPC_HDR_BBA_LEN + 8 + (2*RPC_IFACE_LEN))

/* RPC_RESULTS - can only cope with one reason, right now... */
typedef struct rpc_results_info
{
/* uint8[] # 4-byte alignment padding, against SMB header */

  uint8 num_results; /* the number of results (0x01) */

/* uint8[] # 4-byte alignment padding, against SMB header */

  uint16 result; /* result (0x00 = accept) */
  uint16 reason; /* reason (0x00 = no reason specified) */

} RPC_RESULTS;

/* RPC_HDR_BA */
typedef struct rpc_hdr_ba_info
{
  RPC_HDR_BBA bba;

  RPC_ADDR_STR addr    ;  /* the secondary address string, as described earlier */
  RPC_RESULTS  res     ; /* results and reasons */
  RPC_IFACE    transfer; /* the transfer syntax from the request */

} RPC_HDR_BA;

/* RPC_AUTH_VERIFIER */
typedef struct rpc_auth_verif_info
{
	fstring signature; /* "NTLMSSP".. Ok, not quite anymore */
	uint32  msg_type; /* NTLMSSP_MESSAGE_TYPE (1,2,3) and 5 for schannel */

} RPC_AUTH_VERIFIER;

/* this is TEMPORARILY coded up as a specific structure */
/* this structure comes after the bind request */
/* RPC_AUTH_NTLMSSP_NEG */
typedef struct rpc_auth_ntlmssp_neg_info
{
	uint32  neg_flgs; /* 0x0000 b2b3 */

	STRHDR hdr_myname; /* offset is against START of this structure */
	STRHDR hdr_domain; /* offset is against START of this structure */

	fstring myname; /* calling workstation's name */
	fstring domain; /* calling workstations's domain */

} RPC_AUTH_NTLMSSP_NEG;

/* this is TEMPORARILY coded up as a specific structure */
/* this structure comes after the bind acknowledgement */
/* RPC_AUTH_NTLMSSP_CHAL */
typedef struct rpc_auth_ntlmssp_chal_info
{
	uint32 unknown_1; /* 0x0000 0000 */
	uint32 unknown_2; /* 0x0000 0028 */
	uint32 neg_flags; /* 0x0000 82b1 */

	uint8 challenge[8]; /* ntlm challenge */
	uint8 reserved [8]; /* zeros */

} RPC_AUTH_NTLMSSP_CHAL;


/* RPC_AUTH_NTLMSSP_RESP */
typedef struct rpc_auth_ntlmssp_resp_info
{
	STRHDR hdr_lm_resp; /* 24 byte response */
	STRHDR hdr_nt_resp; /* 24 byte response */
	STRHDR hdr_domain;
	STRHDR hdr_usr;
	STRHDR hdr_wks;
	STRHDR hdr_sess_key; /* NULL unless negotiated */
	uint32 neg_flags; /* 0x0000 82b1 */

	fstring sess_key;
	fstring wks;
	fstring user;
	fstring domain;
	fstring nt_resp;
	fstring lm_resp;

} RPC_AUTH_NTLMSSP_RESP;

/* attached to the end of encrypted rpc requests and responses */
/* RPC_AUTH_NTLMSSP_CHK */
typedef struct rpc_auth_ntlmssp_chk_info
{
	uint32 ver; /* 0x0000 0001 */
	uint32 reserved;
	uint32 crc32; /* checksum using 0xEDB8 8320 as a polynomial */
	uint32 seq_num;

} RPC_AUTH_NTLMSSP_CHK;

#define RPC_AUTH_NTLMSSP_CHK_LEN 16

#endif /* _DCE_RPC_H */
