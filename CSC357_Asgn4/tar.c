#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include "tar.h"
#define ARGS_OPT 1
#define ARGS_TARNAME 2
#define ARGS_FILESTART 3
#define CFLAG 0
#define TFLAG 1
#define XFLAG 2
#define VFLAG 3
#define SFLAG 4
#define FFLAG 5
#define MAGIC "ustar"
#define PERMISSIONS 0666    /* Read, Write for everyone */
#define MAX_PATH_LEN 256
#define MAX_OCTAL_7 07777777
#define MAX_OCTAL_11 077777777777
#define TYPEFLAG_REG "0"
#define TYPEFLAG_REGALT "\0"
#define TYPEFLAG_SYM "2"
#define TYPEFLAG_DIR "5"
#define MAGIC "ustar"
#define VERSION "00"

void Tar_create(Tar *tar) {
    /* Initialize */
    int tarfd, idx, fileNamelength;
    DIR *directory;
    char *filebuff;

    /* Create tar file, archive tar contents */
    /* If tar file exists, truncate to 0 */
    if ((tarfd = open(tar->tarName, O_CREAT | O_TRUNC | O_WRONLY, PERMISSIONS)) == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }   

    /* Archive directories */
    for (idx = 0; idx < tar->fileCnt; idx++) {
        fileNamelength = strlen(tar->files[idx]);
        
        /* Check if pathname exceeds limit */
        if (fileNamelength > MAX_PATH_LEN) {
            printf("Pathname is too long...skipping\n");
            continue;
        }

        if ((directory = opendir(tar->files[idx])) != NULL) {           
            /* Check whether directory has '/' */
            if (tar->files[idx][fileNamelength - 1] == '/') {
                filebuff = (char*) malloc((fileNamelength + 1) * sizeof(char));
                if (!filebuff) {
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }
                strcpy(filebuff, tar->files[idx]);
            } else {
                filebuff = (char*) malloc((fileNamelength + 2) * sizeof(char));
                if (!filebuff) {
                    perror("malloc");
                    exit(EXIT_FAILURE);
                }    
                strcpy(filebuff, tar->files[idx]);
                strcat(filebuff, "/");
            } 

            /* Archive Directory to tar */
            Tar_archiveDir(tar, tarfd, directory, filebuff);
            if ((closedir(directory)) == -1) {
                perror("closedir");
                exit(EXIT_FAILURE);
            } 
            free(filebuff);        
        }
 
    /* Archive regular files / symlinks (if not a directory) */
        else {
            Tar_archiveFile(tar, tarfd, tar->files[idx]);
        }
    }

   if ((close(tarfd)) == -1) {
        perror("close");
        exit(EXIT_FAILURE);
    }
}



void Tar_archiveDir(Tar *tar, int tarfd, DIR *directory, char *dirname) {
    /* Initialize */
    Header *dirHead;
    struct dirent *entry;
    struct stat entryinfo;
    DIR *childDirectory;
    char *childPath;

    /* Stat using fd instead of pathname to be sure */
    if ((stat(dirname, &entryinfo)) != 0) {
        perror("fstat");
        exit(EXIT_FAILURE);
    }

    /* Create Header for Directory in TAR */
    dirHead = Tar_createHeader(tar, &entryinfo, dirname);
    write(tarfd, dirHead, sizeof(Header));
    tar->headerCnt++;
    
    /* Include Verbose */
    if (tar->option[VFLAG]) {
        printf("Archived directory: %s\n", dirname);
    }

    /* Recursively archive files inside of Directory */
    while ((entry = readdir(directory))) {
        /* Do not archive "." and ".." */
        if ((strcmp(entry->d_name, ".") != 0) && (strcmp(entry->d_name, "..") != 0)) {
            /* Check if entry file path exceeds limit */
            if ((strlen(entry->d_name) ) >= MAX_PATH_LEN) {
                printf("Pathname is too long... skipping\n");
                continue;
            }

            /* If pathname conforms */
            childPath = (char*) malloc(MAX_PATH_LEN * sizeof(char));
            if (!childPath) {
                perror("malloc");
                exit(EXIT_FAILURE);
            }
            sprintf(childPath, "%s%s", dirname, entry->d_name);

            /* Check to see if it is indeed a directory, archive */
            if ((childDirectory = opendir(childPath))) {
                sprintf(childPath, "%s%c", childPath, '/');
                Tar_archiveDir(tar, tarfd, childDirectory, childPath);
                closedir(childDirectory); 
            /* If it is a regular file or symlink */
            } else {
                Tar_archiveFile(tar, tarfd, childPath);
            }

            free(childPath);
        }
    }  
}



void Tar_archiveFile(Tar *tar, int tarfd, char *filename) {
    /* Initialize */
    int childfd;
    struct stat entryinfo;
    Header *fileHead;

    /* For directory recursion */
    if ((strlen(filename)) >= MAX_PATH_LEN) {
        printf("Pathname is too long... skipping\n");
        return;
    }

    /* Open file */
    if ((childfd = open(filename, O_RDONLY)) == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    /* Stat file for info */
    if ((lstat(filename, &entryinfo)) == -1) {
        perror("lstat");
        exit(EXIT_FAILURE);
    }

    /* Create header for file and archive in TAR */
    fileHead = Tar_createHeader(tar, &entryinfo, filename);
    write(tarfd, fileHead, sizeof(Header));
    tar->headerCnt++;
   
    /* Include Verbose */
    if (tar->option[VFLAG]) {
        printf("Archived file: %s\n", filename);
    }

    /* Close file */
    close(childfd); 
} 



Header *Tar_createHeader(Tar *tar, struct stat *fileEntry, char *filename) {
    /* Initialize */
    Header *newHead;
    uid_t user_id;
    gid_t group_id;
    ssize_t linkSize;
    struct passwd *user_info;
    struct group *group_info;

    newHead = (Header*) malloc(sizeof(Header));
    if (!newHead) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    
    /* Retrieve file owner and group ID for header */
    user_id = fileEntry->st_uid;
    group_id = fileEntry->st_gid;

    /* Retrieve all file owner and group information for header */
    user_info = getpwuid(user_id);
    group_info = getgrgid(group_id);


    /* Input fields into Header */
    /* Name */
    strncpy(newHead->name, filename, NAME_LEN);

    /* Mode */
    if ((sprintf(newHead->mode, "%07o", fileEntry->st_mode)) == -1) {
        perror("sprintf");
        exit(EXIT_FAILURE);
    }

    /* UID */
    /* If UID exceeds 8 bytes */
    if (user_info->pw_uid > MAX_OCTAL_7) {
        if (tar->option[SFLAG]) {
            printf("File UID too big!");
        } else {
            /* Use special insert for non-conforming header */
            insert_special_int(newHead->uid, UID_LEN, user_info->pw_uid);
        } 
    /* If UID conforms to header */   
    } else {
        if ((sprintf(newHead->uid, "%07o", user_info->pw_uid)) == -1) {
            perror("sprintf");
            exit(EXIT_FAILURE);
        }
    }

    /* GID */
    /* If GID exceeds 8 bytes */
    if (user_info->pw_gid > MAX_OCTAL_7) {
        if (tar->option[SFLAG]) {
            printf("File GID too big!");
        } else {
            /* Use special insert for non-conforming header */
            insert_special_int(newHead->gid, GID_LEN, user_info->pw_gid);
        } 
    /* If GID conforms to header */   
    } else {
        if ((sprintf(newHead->gid, "%07o", user_info->pw_gid)) == -1) {
            perror("sprintf");
            exit(EXIT_FAILURE);
        }
    }

    /* Size  */
    /* If file is regular */
    if (S_ISREG(fileEntry->st_mode)) {
        /* If Size ecveeds 11 bytes */
        if (fileEntry->st_size > MAX_OCTAL_11) {
            if (tar->option[SFLAG]) {
                printf("File size too big!");
            } else {
            /* Use special insert for non-conforming header */
                insert_special_int(newHead->size, SIZE_LEN, fileEntry->st_size);
            }
        /* If Size conforms to header */
        } else {
            if ((sprintf(newHead->size, "%011lo", (unsigned long) fileEntry->st_size)) == -1) {
                perror("sprintf");
                exit(EXIT_FAILURE);
            }
        }
    /* If file is directory or symlink */
    } else {
        if ((sprintf(newHead->size, "%011o", 0)) == -1) {
                perror("sprintf");
                exit(EXIT_FAILURE);
        }
    }

    /* MTIME */
    /* If mtime ecveeds 11 bytes */
    if (fileEntry->st_mtime > MAX_OCTAL_11) {
        if (tar->option[SFLAG]) {
            printf("File mtime too big!");
        } else {
        /* Use special insert for non-conforming header */
            insert_special_int(newHead->mtime, MTIME_LEN, fileEntry->st_mtime);
        }
    /* If mtime conforms to header */
    } else {
        if ((sprintf(newHead->mtime, "%011lo", (unsigned long) (fileEntry->st_mtime))) == -1) {
                perror("sprintf");
                exit(EXIT_FAILURE);
        }
    }

    /* TYPEFLAG and LINKAME */
    /* If directory, typeflag is 5 */
    if (S_ISDIR(fileEntry->st_mode)) {
        strncpy(newHead->typeflag, TYPEFLAG_DIR, TYPEFLAG_LEN); 
    /* If symlink, typeflag is 2 */
    } else if (S_ISLNK(fileEntry->st_mode)) {
        strncpy(newHead->typeflag, TYPEFLAG_SYM, TYPEFLAG_LEN);
        
        /* Stores Linkname as well w/ NULL terminator */
        if ((linkSize = readlink(filename, newHead->linkname, LINKNAME_LEN)) == -1) {
            perror("readlink");
            exit(EXIT_FAILURE);
        }
    /* If regular file */   
    } else {
        strncpy(newHead->typeflag, TYPEFLAG_REG, TYPEFLAG_LEN);
    }

    /* MAGIC and VERSION */
    strncpy(newHead->magic, MAGIC, MAGIC_LEN);
    strncpy(newHead->version, VERSION, VERSION_LEN);

    /* UNAME (NULL Terminated) */
    /* If uname exceeds 32 bytes */     
    if (strlen(user_info->pw_name) > (UNAME_LEN - 1)) {
        strncpy(newHead->uname, user_info->pw_name, (UNAME_LEN - 1));
        newHead->uname[UNAME_LEN] = '\0';
    /* If uname conforms to ehader */
    } else {
        strncpy(newHead->uname, user_info->pw_name, UNAME_LEN);
    }

    /* GNAME */
    /* If gname exceeds 32 bytes */
    if (strlen(group_info->gr_name) > (GNAME_LEN - 1)) {
        strncpy(newHead->gname, group_info->gr_name, (GNAME_LEN - 1));
        newHead->gname[GNAME_LEN] = '\0';
    /* If gname conforms to ehader */
    } else {
        strncpy(newHead->gname, group_info->gr_name, GNAME_LEN);
    }

    /* DEVMAJOR and DEVMINOR for special files*/
    if (S_ISCHR(fileEntry->st_mode) || S_ISBLK(fileEntry->st_mode)) {
        /* If devmajor exceeds 8 bytes */
        if ((major(fileEntry->st_rdev)) > MAX_OCTAL_7) {
            if (tar->option[SFLAG]) {
                printf("Devmajor is too big!");
            } else {
                insert_special_int(newHead->devmajor, DEVMAJOR_LEN, major(fileEntry->st_rdev));
            } 
        /* If devmajor confroms to header */
        } else {
            if ((sprintf(newHead->devmajor, "%07o", major(fileEntry->st_rdev))) == -1) {
                perror("sprintf");
                exit(EXIT_FAILURE);
            }
        }
        /* If devminor exceeds 8 bytes */
        if ((minor(fileEntry->st_rdev)) > MAX_OCTAL_7) {
            if (tar->option[SFLAG]) {
                printf("Devminor is too big!");
            } else {
                insert_special_int(newHead->devminor, DEVMINOR_LEN, minor(fileEntry->st_rdev));
            } 
        /* If devminor confroms to header */
        } else {
            if ((sprintf(newHead->devminor, "%07o", minor(fileEntry->st_rdev))) == -1) {
                perror("sprintf");
                exit(EXIT_FAILURE);
            }
        }
    }

    /* PREFIX */
    if (strlen(filename) > NAME_LEN) {
        strncpy(newHead->prefix, (filename + NAME_LEN), PREFIX_LEN); 
    }

    /* CHKSUM */

    return newHead;    
}


int insert_special_int(char *where, size_t size, int32_t val) {
    int err = 0;
    if (val < 0 || (size < sizeof(val))) {
        err++;
    } else {
        memset(where, 0, size);
    *(int32_t*)(where + size - sizeof(val)) = htonl(val);
    *where |= 0x80;
    }

    return err;
}
