#ifndef TAR_H
#define TAR_H

/* Macros for Header */
#define NAME_LEN 100
#define MODE_LEN 8
#define UID_LEN 8
#define GID_LEN 8
#define SIZE_LEN 12
#define MTIME_LEN 12
#define CHKSUM_LEN 8
#define TYPEFLAG_LEN 1
#define LINKNAME_LEN 100
#define MAGIC_LEN 6
#define VERSION_LEN 2
#define UNAME_LEN 32
#define GNAME_LEN 32
#define DEVMAJOR_LEN 8
#define DEVMINOR_LEN 8
#define PREFIX_LEN 155

/* Macros for Tar */
#define OPT_SEL 6

typedef struct Header {
    char name[NAME_LEN];
    char mode[MODE_LEN];
    char uid[UID_LEN];
    char gid[GID_LEN];
    char size[SIZE_LEN];
    char mtime[MTIME_LEN];
    char chksum[CHKSUM_LEN];
    char typeflag[TYPEFLAG_LEN];
    char linkname[LINKNAME_LEN];
    char magic[MAGIC_LEN];
    char version[VERSION_LEN];
    char uname[UNAME_LEN];
    char gname[GNAME_LEN];
    char devmajor[DEVMAJOR_LEN];
    char devminor[DEVMINOR_LEN];
    char prefix[PREFIX_LEN];
} Header;

typedef struct Tar {
    char *tarName;
    char **files;
    int option[OPT_SEL];
    int fileCnt;
    int headerCnt;
} Tar;

void Tar_create(Tar *tar);
void Tar_archiveDir(Tar *tar, int tarfd, DIR *directory, char *dirname);
void Tar_archiveFile(Tar *tar, int tarfd, char *filename);
Header *Tar_createHeader(Tar *tar, struct stat *fileEntry, char *filename);
int insert_special_int(char *where, size_t size, int32_t val); 
#endif
