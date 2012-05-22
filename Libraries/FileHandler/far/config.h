/* config.h - libfar build configuration */

/* Define if you have the <stdint.h> header file. */
#define HAVE_STDINT_H

/* compile-in support */
#define LIBFAR_ARCHIVEREAD
#define LIBFAR_ARCHIVEWRITE
#define LIBFAR_REFPACK_DECOMPRESS
#define LIBFAR_REFPACK_COMPRESS
#define LIBFAR_SUPPORT_FAR
#define LIBFAR_SUPPORT_DBPF
#define LIBFAR_SUPPORT_PERSIST
#define LIBFAR_DEBUGSUPPORT
#define LIBFAR_FILEIO
#define LIBFAR_EMBEDDEDFUNCTIONS
/* end of compile-in support */

/* preferences -- on non-numerical definitions, define to 1 for "yes", 0 for "no"; */
#define LIBFAR_DEFAULT_1A		        	    0
#define LIBFAR_DEFAULT_DBPF_COMPRESSED     	    0
#define LIBFAR_DEFAULT_MAX_FILE_NAME_LENGTH		255
#define LIBFAR_DEFAULT_REFPACK_HNSV             0xFB
/* end of default preferences */
