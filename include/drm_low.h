#ifndef DRM_LOW_H
#define DRM_LOW_H

#include <unistd.h>
#include <drm/drm.h>
#include <drm/drm_mode.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include <inttypes.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <string.h>
#include <time.h>


int drmGetCap(int fd, uint64_t capability, uint64_t* value);

int sequence_colors_fill_screen();

int sequence_oscillate_square();

void draw_square(int color, int buff_full);

int adjust_crtc_offset(float x_off, float y_off);

int drm_setup();

int close_dri();

#endif

