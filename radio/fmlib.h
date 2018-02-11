/* fmlib.c - simple V4L2 compatible tuner for radio cards

   Copyright (C) 2009 Ben Pfaff <blp@cs.stanford.edu>

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License along with
   this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FMLIB_H
#define FMLIB_H 1

#include <linux/videodev2.h>
#include <stdint.h>
#include <stdbool.h>

#define DEFAULT_DEVICE  "/dev/radio0"

struct tuner {
        int fd;
        int index;
        struct v4l2_tuner tuner;
};

void tuner_open(struct tuner *, const char *device, int index);
void tuner_close(struct tuner *);

long long int tuner_get_min_freq(const struct tuner *);
long long int tuner_get_max_freq(const struct tuner *);
void tuner_set_freq(const struct tuner *, long long int frequency,
                    bool override_range);

#endif /* fmlib.h */
