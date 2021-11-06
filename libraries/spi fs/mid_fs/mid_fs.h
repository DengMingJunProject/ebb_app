/*********************************************************************************
File info   :               mid_fs.c
Mark        :
*********************************************************************************/
/********************************************************************************
                            Head files
*********************************************************************************/
#ifndef __MID_FS_H__
#define __MID_FS_H__

#include <stdint.h>
#include "sfud.h"

#define FS_USE_TYPE     FS_USE_LFS

#define FS_START_ADDR	1024*1024
#define FS_MB_SIZE		15

#define FS_USE_SPIFFS   1
#define FS_USE_LFS      2

#define LFSCFG_BLOCK_COUNT  ( FS_MB_SIZE * 1024 * 1024 / 4096 )
#define LFSCFG_LOOKAHEAD    (32 * ( ( LFSCFG_BLOCK_COUNT + 31 ) / 32 ))

#define FNAME_MARK_LFS_FORMAT   "lfsformated"


#if (FS_USE_TYPE == FS_USE_SPIFFS)
    #include "spiffs.h"
    #define FILE_HANDLER    spiffs_file

    #define FS_TYPE_STRUCT  spiffs
    //extern spiffs fs;
    #define MID_O_CREAT     (SPIFFS_CREAT | SPIFFS_RDWR | SPIFFS_TRUNC)
    #define MID_O_MODIFY    (SPIFFS_RDWR | SPIFFS_CREAT)
    #define MID_O_RDONLY    SPIFFS_RDONLY

    #define MIDFS_SEEK_SET      SPIFFS_SEEK_SET
    #define MIDFS_SEEK_CUR      SPIFFS_SEEK_CUR
    #define MIDFS_SEEK_END      SPIFFS_SEEK_END
#endif

#if (FS_USE_TYPE == FS_USE_LFS)
    #include "lfs.h"
    #define FILE_HANDLER    lfs_file_t
    #define FS_TYPE_STRUCT  lfs_t
    //extern lfs_t lfs;

    #define MIDFS_O_CREAT    	( LFS_O_RDWR | LFS_O_TRUNC | LFS_O_CREAT)
    #define MIDFS_O_MODIFY    	(LFS_O_RDWR | LFS_O_CREAT)
    #define MIDFS_O_RDONLY    	LFS_O_RDONLY

    #define MIDFS_SEEK_SET      LFS_SEEK_SET
    #define MIDFS_SEEK_CUR      LFS_SEEK_CUR
    #define MIDFS_SEEK_END      LFS_SEEK_END
#endif

extern FS_TYPE_STRUCT 		mid_fs;
extern const sfud_flash *flash;

extern int midfs_fopen( FILE_HANDLER *fp, char *path, char *mode );
extern int midfs_fclose( FILE_HANDLER *fp );
extern uint32_t midfs_fread( FILE_HANDLER *fp, uint32_t size, void *dst );
extern uint32_t midfs_fwrite( FILE_HANDLER *fp, uint32_t size, void *src );
extern int32_t midfs_fseek( FILE_HANDLER *fp, uint32_t offset, uint32_t from );
extern uint32_t midfs_fsize( FILE_HANDLER *fp );
extern int midfs_fremove( char *fname );
extern void mid_fs_init( void );
extern void midfs_ls( void );
extern uint32_t midfs_lfs_usage( void );
extern int midfs_isexist(char *name );
#endif

/*********************************************************************************
                             End of file
*********************************************************************************/

