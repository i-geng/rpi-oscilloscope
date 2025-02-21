#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libunix.h"

// allocate buffer, read entire file into it, return it.   
// buffer is zero padded to a multiple of 4.
//
//  - <size> = exact nbytes of file.
//  - for allocation: round up allocated size to 4-byte multiple, pad
//    buffer with 0s. 
//
// fatal error: open/read of <name> fails.
//   - make sure to check all system calls for errors.
//   - make sure to close the file descriptor (this will
//     matter for later labs).
// 
void *read_file(unsigned *size, const char *name) {
    // How: 
    //    - use stat() to get the size of the file.
    //    - round up to a multiple of 4.
    //    - allocate a buffer
    //    - zero pads to a multiple of 4.
    //    - read entire file into buffer (read_exact())
    //    - fclose() the file descriptor
    //    - make sure any padding bytes have zeros.
    //    - return it.   
    struct stat st;
    // use stat() to get the size of the file.
    stat(name, &st);

    *size = st.st_size;
    // round up to a multiple of 4
    unsigned padded_size = (*size + 3) & ~3;

    // allocate a buffer
    char *buf = calloc(padded_size, sizeof(char));
  
    int fd = open(name, O_RDONLY);
    // pad the buffer with zeros
    if(*size > 0) {
        read_exact(fd, buf, *size);
    }
    close(fd);

    return buf;
}
