// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>

#include "libunix.h"

#define _SVID_SOURCE
#include <dirent.h>
static const char *ttyusb_prefixes[] = {
    "ttyUSB",	// linux
    "ttyACM",   // linux
    "cu.SLAB_USB", // mac os
    "cu.usbserial", // mac os
    // if your system uses another name, add it.
	0
};

static int filter(const struct dirent *d) {
    // scan through the prefixes, returning 1 when you find a match.
    // 0 if there is no match.
    for(const char **prefix = ttyusb_prefixes; *prefix; prefix++) {
        if(strncmp(d->d_name, *prefix, strlen(*prefix)) == 0)
            return 1;
    }
    return 0;
}

// find the TTY-usb device (if any) by using <scandir> to search for
// a device with a prefix given by <ttyusb_prefixes> in /dev
// returns:
//  - device name.
// error: panic's if 0 or more than 1 devices.
char *find_ttyusb(void) {
    // use <alphasort> in <scandir>
    // return a malloc'd name so doesn't corrupt.
    struct dirent **namelist;
    int n;

    n = scandir("/dev", &namelist, filter, alphasort);
    if(n < 0){
        panic("scandir failed\n");
    }
    if(n == 0){
        panic("no ttyusb device found\n");
    }
    if(n > 1){
        panic("more than one ttyusb device found\n");
    }
    char *name;
    name = malloc(strlen("/dev/") + strlen(namelist[0]->d_name) + 1);
    strcpy(name, "/dev/");
    strcat(name, namelist[0]->d_name);
    printf("name: %s\n", name);
    free(namelist);
    return name;
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) {
    struct dirent **namelist;
    int n;

    n = scandir("/dev", &namelist, filter, alphasort);
    if(n == 0){
        panic("no ttyusb device found\n");
    }

    int lastest = 0;
    char* lastest_name = NULL;
    for(int i = 0; i < n; i++){
        char *name = malloc(strlen("/dev/") + strlen(namelist[i]->d_name) + 1);
        strcpy(name, "/dev/");
        strcat(name, namelist[i]->d_name);

        struct stat st;
        stat(name, &st);
        if(st.st_mtime > lastest){
            lastest = st.st_mtime;
            // free the previous lastest_name if it exists
            if(lastest_name){
                free(lastest_name);
            }
            // allocate memory for the new lastest_name
            lastest_name = malloc(strlen(name) + 1);
            strcpy(lastest_name, name);
        }
        free(name);
    }
    free(namelist);
    return lastest_name;
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
    struct dirent **namelist;
    int n;

    n = scandir("/dev", &namelist, filter, alphasort);
    if(n == 0){
        panic("no ttyusb device found\n");
    }

    int oldest = INT_MAX;
    char* oldest_name = NULL;
    for(int i = 0; i < n; i++){
        char *name = malloc(strlen("/dev/") + strlen(namelist[i]->d_name) + 1);
        strcpy(name, "/dev/");
        strcat(name, namelist[i]->d_name);

        struct stat st;
        stat(name, &st);
        if(st.st_mtime < oldest){
            oldest = st.st_mtime;
            // free the previous oldest_name if it exists
            if(oldest_name){
                free(oldest_name);
            }
            // allocate memory for the new oldest_name
            oldest_name = malloc(strlen(name) + 1);
            strcpy(oldest_name, name);
        }
        free(name);
    }
    free(namelist);
    return oldest_name;
}
