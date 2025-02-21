// engler, cs140e: your code to find the tty-usb device on your laptop.
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
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
  for (int i = 0; ttyusb_prefixes[i] != 0; i++) {
    const char* prefix = ttyusb_prefixes[i];
    if (strncmp(prefix, d->d_name, strlen(prefix)) == 0) {
      return 1;
    }
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
  struct dirent **name_list;
  int num_devices = scandir("/dev", &name_list, filter, alphasort);

  if (num_devices <= 0 || num_devices > 1) {
    panic("Found %d devices!\n", num_devices);
  }

  // Make a malloc'd copy of the device name and return it
  const char* dir_name = "/dev/";
  int total_len = strlen(dir_name) + strlen(name_list[0]->d_name) + 1;
  char* device_path = (char*) malloc(total_len);
  snprintf(device_path, total_len, "%s%s", dir_name, name_list[0]->d_name);
  free(name_list);
  return device_path;
}

// return the most recently mounted ttyusb (the one
// mounted last).  use the modification time 
// returned by state.
char *find_ttyusb_last(void) {
  struct dirent **name_list;
  int num_devices = scandir("/dev", &name_list, filter, alphasort);
  if (num_devices <= 0) {
    panic("Found %d devices!\n", num_devices);
  }

  const char* dir_name = "/dev/";
  time_t last_mod_time = 0;
  char* last_device;
  struct stat device_stats;

  for (int i = 0; i < num_devices; i++) {
    int total_len = strlen(dir_name) + strlen(name_list[i]->d_name) + 1;
    char* device_path = (char*) malloc(total_len);
    snprintf(device_path, total_len, "%s%s", dir_name, name_list[i]->d_name);

    assert( stat(device_path, &device_stats) == 0 );
    time_t device_mod_time = device_stats.st_mtime;

    if (device_mod_time > last_mod_time) {
      last_mod_time = device_mod_time;
      last_device = device_path;
    }
  }

  // Make a malloc'd copy of the device name and return it
  return last_device;
}

// return the oldest mounted ttyusb (the one mounted
// "first") --- use the modification returned by
// stat()
char *find_ttyusb_first(void) {
  struct dirent **name_list;

  int num_devices = scandir("/dev", &name_list, filter, alphasort);
  if (num_devices <= 0) {
    panic("Found %d devices!\n", num_devices);
  }

  const char* dir_name = "/dev/";
  time_t first_mod_time = INT_MAX;
  char* first_device;
  struct stat device_stats;

  for (int i = 0; i < num_devices; i++) {
    int total_len = strlen(dir_name) + strlen(name_list[i]->d_name) + 1;
    char* device_path = (char*) malloc(total_len);
    snprintf(device_path, total_len, "%s%s", dir_name, name_list[i]->d_name);

    assert( stat(device_path, &device_stats) == 0 );
    time_t device_mod_time = device_stats.st_mtime;

    if (device_mod_time < first_mod_time) {
      first_mod_time = device_mod_time;
      first_device = device_path;
    }
  }

  // Make a malloc'd copy of the device name and return it
  return first_device;
}
