/* config.h - far build configuration */

/* Define if you have the <stdint.h> header file. */
#define HAVE_STDINT_H

/* compile-in support */
#define FAR_ARCHIVEREAD
#define FAR_ARCHIVEWRITE
#define FAR_REFPACK_DECOMPRESS
#define FAR_REFPACK_COMPRESS
#define FAR_SUPPORT_FAR
#define FAR_SUPPORT_DBPF
#define FAR_SUPPORT_PERSIST
#define FAR_DEBUGSUPPORT
#define FAR_FILEIO
#define FAR_EMBEDDEDFUNCTIONS
/* end of compile-in support */

/* preferences -- on non-numerical definitions, define to 1 for "yes", 0 for "no" */
#define FAR_DEFAULT_1A		        	    0
#define FAR_DEFAULT_DBPF_COMPRESSED     	    0
#define FAR_DEFAULT_MAX_FILE_NAME_LENGTH		255
#define FAR_DEFAULT_REFPACK_HNSV             0xFB
/* end of default preferences */
