/* 
   Unix SMB/CIFS implementation.
   passdb structures and parameters
   Copyright (C) Gerald Carter 2001
   Copyright (C) Luke Kenneth Casson Leighton 1998 - 2000
   Copyright (C) Andrew Bartlett 2002
   Copyright (C) Simo Sorce 2003
   
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

#ifndef _PASSDB_H
#define _PASSDB_H


/*
 * bit flags representing initialized fields in SAM_ACCOUNT
 */
enum pdb_elements {
	PDB_UNINIT,
	PDB_SMBHOME,
	PDB_PROFILE,
	PDB_DRIVE,
	PDB_LOGONSCRIPT,
	PDB_LOGONTIME,
	PDB_LOGOFFTIME,
	PDB_KICKOFFTIME,
	PDB_CANCHANGETIME,
	PDB_MUSTCHANGETIME,
	PDB_PLAINTEXT_PW,
	PDB_USERNAME,
	PDB_FULLNAME,
	PDB_DOMAIN,
	PDB_NTUSERNAME,
	PDB_HOURSLEN,
	PDB_LOGONDIVS,
	PDB_USERSID,
	PDB_GROUPSID,
	PDB_ACCTCTRL,
	PDB_PASSLASTSET,
	PDB_UNIXHOMEDIR,
	PDB_ACCTDESC,
	PDB_WORKSTATIONS,
	PDB_UNKNOWNSTR,
	PDB_MUNGEDDIAL,
	PDB_HOURS,
	PDB_UNKNOWN3,
	PDB_BAD_PASSWORD_COUNT,
	PDB_LOGON_COUNT,
	PDB_UNKNOWN6,
	PDB_LMPASSWD,
	PDB_NTPASSWD,
	PDB_BACKEND_PRIVATE_DATA,

	/* this must be the last element */
	PDB_COUNT
};

enum pdb_group_elements {
	PDB_GROUP_NAME,
	PDB_GROUP_SID,
	PDB_GROUP_SID_NAME_USE,
	PDB_GROUP_MEMBERS,

	/* this must be the last element */
	PDB_GROUP_COUNT
};


enum pdb_value_state {
	PDB_DEFAULT=0,
	PDB_SET,
	PDB_CHANGED
};

#define IS_SAM_SET(x, flag)	(pdb_get_init_flags(x, flag) == PDB_SET)
#define IS_SAM_CHANGED(x, flag)	(pdb_get_init_flags(x, flag) == PDB_CHANGED)
#define IS_SAM_DEFAULT(x, flag)	(pdb_get_init_flags(x, flag) == PDB_DEFAULT)
		
typedef struct sam_passwd
{
	TALLOC_CTX *mem_ctx;
	
	void (*free_fn)(struct sam_passwd **);

	struct pdb_methods *methods;

	struct user_data {
		/* initialization flags */
		struct bitmap *change_flags;
		struct bitmap *set_flags;

		time_t logon_time;            /* logon time */
		time_t logoff_time;           /* logoff time */
		time_t kickoff_time;          /* kickoff time */
		time_t pass_last_set_time;    /* password last set time */
		time_t pass_can_change_time;  /* password can change time */
		time_t pass_must_change_time; /* password must change time */
		
		const char * username;     /* UNIX username string */
		const char * domain;       /* Windows Domain name */
		const char * nt_username;  /* Windows username string */
		const char * full_name;    /* user's full name string */
		const char * unix_home_dir;     /* UNIX home directory string */
		const char * home_dir;     /* home directory string */
		const char * dir_drive;    /* home directory drive string */
		const char * logon_script; /* logon script string */
		const char * profile_path; /* profile path string */
		const char * acct_desc  ;  /* user description string */
		const char * workstations; /* login from workstations string */
		const char * unknown_str ; /* don't know what this is, yet. */
		const char * munged_dial ; /* munged path name and dial-back tel number */
		
		DOM_SID user_sid;    /* Primary User SID */
		DOM_SID group_sid;   /* Primary Group SID */
		
		DATA_BLOB lm_pw; /* .data is Null if no password */
		DATA_BLOB nt_pw; /* .data is Null if no password */
		char* plaintext_pw; /* is Null if not available */
		
		uint16 acct_ctrl; /* account info (ACB_xxxx bit-mask) */
		uint32 unknown_3; /* 0x00ff ffff */
		
		uint16 logon_divs; /* 168 - number of hours in a week */
		uint32 hours_len; /* normally 21 bytes */
		uint8 hours[MAX_HOURS_LEN];
		
		/* Was unknown_5. */
		uint16 bad_password_count;
		uint16 logon_count;

		uint32 unknown_6; /* 0x0000 04ec */
		/* a tag for who added the private methods */
		const struct pdb_methods *backend_private_methods;
		void *backend_private_data; 
		void (*backend_private_data_free_fn)(void **);
	} private;

	/* Lets see if the remaining code can get the hint that you
	   are meant to use the pdb_...() functions. */
	
} SAM_ACCOUNT;

typedef struct sam_group {
	TALLOC_CTX *mem_ctx;
	
	void (*free_fn)(struct sam_group **);

	struct pdb_methods *methods;

	struct group_data {
		/* initialization flags */
		struct bitmap *change_flags;
		struct bitmap *set_flags;

		const char *name;		/* Windows group name string */

		DOM_SID sid;			/* Group SID */
		enum SID_NAME_USE sid_name_use;	/* Group type */

		uint32 mem_num;			/* Number of member SIDs */
		DOM_SID *members;		/* SID array */
	} private;

} SAM_GROUP;


/*****************************************************************
 Functions to be implemented by the new (v2) passdb API 
****************************************************************/

/*
 * This next constant specifies the version number of the PASSDB interface
 * this SAMBA will load. Increment this if *ANY* changes are made to the interface. 
 */

#define PASSDB_INTERFACE_VERSION 4

typedef struct pdb_context 
{
	struct pdb_methods *pdb_methods;
	struct pdb_methods *pwent_methods;
	
	/* These functions are wrappers for the functions listed above.
	   They may do extra things like re-reading a SAM_ACCOUNT on update */

	NTSTATUS (*pdb_setsampwent)(struct pdb_context *, BOOL update);
	
	void (*pdb_endsampwent)(struct pdb_context *);
	
	NTSTATUS (*pdb_getsampwent)(struct pdb_context *, SAM_ACCOUNT *user);
	
	NTSTATUS (*pdb_getsampwnam)(struct pdb_context *, SAM_ACCOUNT *sam_acct, const char *username);
	
	NTSTATUS (*pdb_getsampwsid)(struct pdb_context *, SAM_ACCOUNT *sam_acct, const DOM_SID *sid);

	NTSTATUS (*pdb_add_sam_account)(struct pdb_context *, SAM_ACCOUNT *sampass);
	
	NTSTATUS (*pdb_update_sam_account)(struct pdb_context *, SAM_ACCOUNT *sampass);
	
	NTSTATUS (*pdb_delete_sam_account)(struct pdb_context *, SAM_ACCOUNT *username);

	NTSTATUS (*pdb_getgrsid)(struct pdb_context *context, GROUP_MAP *map, DOM_SID sid);
	
	NTSTATUS (*pdb_getgrgid)(struct pdb_context *context, GROUP_MAP *map, gid_t gid);
	
	NTSTATUS (*pdb_getgrnam)(struct pdb_context *context, GROUP_MAP *map, const char *name);
	
	NTSTATUS (*pdb_add_group_mapping_entry)(struct pdb_context *context,
						GROUP_MAP *map);
	
	NTSTATUS (*pdb_update_group_mapping_entry)(struct pdb_context *context,
						   GROUP_MAP *map);
	
	NTSTATUS (*pdb_delete_group_mapping_entry)(struct pdb_context *context,
						   DOM_SID sid);
	
	NTSTATUS (*pdb_enum_group_mapping)(struct pdb_context *context,
					   enum SID_NAME_USE sid_name_use,
					   GROUP_MAP **rmap, int *num_entries,
					   BOOL unix_only);

	void (*free_fn)(struct pdb_context **);
	
	TALLOC_CTX *mem_ctx;
	
} PDB_CONTEXT;

typedef struct pdb_methods 
{
	const char *name; /* What name got this module */
	struct pdb_context *parent;

	/* Use macros from dlinklist.h on these two */
	struct pdb_methods *next;
	struct pdb_methods *prev;

	NTSTATUS (*setsampwent)(struct pdb_methods *, BOOL update);
	
	void (*endsampwent)(struct pdb_methods *);
	
	NTSTATUS (*getsampwent)(struct pdb_methods *, SAM_ACCOUNT *user);
	
	NTSTATUS (*getsampwnam)(struct pdb_methods *, SAM_ACCOUNT *sam_acct, const char *username);
	
	NTSTATUS (*getsampwsid)(struct pdb_methods *, SAM_ACCOUNT *sam_acct, const DOM_SID *sid);
	
	NTSTATUS (*add_sam_account)(struct pdb_methods *, SAM_ACCOUNT *sampass);
	
	NTSTATUS (*update_sam_account)(struct pdb_methods *, SAM_ACCOUNT *sampass);
	
	NTSTATUS (*delete_sam_account)(struct pdb_methods *, SAM_ACCOUNT *username);
	
	NTSTATUS (*getgrsid)(struct pdb_methods *methods, GROUP_MAP *map, DOM_SID sid);

	NTSTATUS (*getgrgid)(struct pdb_methods *methods, GROUP_MAP *map, gid_t gid);

	NTSTATUS (*getgrnam)(struct pdb_methods *methods, GROUP_MAP *map, const char *name);

	NTSTATUS (*add_group_mapping_entry)(struct pdb_methods *methods,
					    GROUP_MAP *map);

	NTSTATUS (*update_group_mapping_entry)(struct pdb_methods *methods,
					       GROUP_MAP *map);

	NTSTATUS (*delete_group_mapping_entry)(struct pdb_methods *methods,
					       DOM_SID sid);

	NTSTATUS (*enum_group_mapping)(struct pdb_methods *methods,
				       enum SID_NAME_USE sid_name_use,
				       GROUP_MAP **rmap, int *num_entries,
				       BOOL unix_only);

	void *private_data;  /* Private data of some kind */
	
	void (*free_private_data)(void **);

} PDB_METHODS;

typedef NTSTATUS (*pdb_init_function)(struct pdb_context *, 
			 struct pdb_methods **, 
			 const char *);

struct pdb_init_function_entry {
	const char *name;
	/* Function to create a member of the pdb_methods list */
	pdb_init_function init;
	struct pdb_init_function_entry *prev, *next;
};

enum sql_search_field { SQL_SEARCH_NONE = 0, SQL_SEARCH_USER_SID = 1, SQL_SEARCH_USER_NAME = 2};

#endif /* _PASSDB_H */
