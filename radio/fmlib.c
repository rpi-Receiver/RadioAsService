/* fmlib.c - simple V4L2 compatible tuner for radio cards

   Copyright (C) 2009, 2012 Ben Pfaff <blp@cs.stanford.edu>

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

#include "fmlib.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

static void query_tuner(const struct tuner *, struct v4l2_tuner *);

void tuner_open(struct tuner *tuner, const char *device, int index)
{
    memset(tuner, 0, sizeof *tuner);

    if (!device)
        device = DEFAULT_DEVICE;

    tuner->fd = open(device, O_RDONLY);
    if (tuner->fd < 0)
        fprintf(stderr, "Error %d Unable to open %s\n", errno, device);
    tuner->index = index;

    query_tuner(tuner, &tuner->tuner);
}

void tuner_close(struct tuner *tuner)
{
    close(tuner->fd);
}

long long int tuner_get_min_freq(const struct tuner *tuner)
{
    long long int rangelow = tuner->tuner.rangelow;
    if (!(tuner->tuner.capability & V4L2_TUNER_CAP_LOW))
        rangelow *= 1000;
    return rangelow;
}

long long int tuner_get_max_freq(const struct tuner *tuner)
{
    long long int rangehigh = tuner->tuner.rangehigh;
    if (!(tuner->tuner.capability & V4L2_TUNER_CAP_LOW))
        rangehigh *= 1000;
    return rangehigh;
}

void tuner_set_freq(const struct tuner *tuner, long long int freq,
               bool override_range)
{
    long long int adj_freq;
    struct v4l2_frequency vf;

    adj_freq = freq;
    if (!(tuner->tuner.capability & V4L2_TUNER_CAP_LOW))
        adj_freq = (adj_freq + 500) / 1000;

    if ((adj_freq < tuner->tuner.rangelow
         || adj_freq > tuner->tuner.rangehigh)
         && !override_range)
            fprintf(stderr, "Error Frequency %.1f MHz out of range (%.1f - %.1f MHz)\n",
                freq / 16000.0,
                tuner_get_min_freq(tuner) / 16000.0,
                tuner_get_max_freq(tuner) / 16000.0);

    memset(&vf, 0, sizeof vf);
    vf.tuner = tuner->index;
    vf.type = tuner->tuner.type;
    vf.frequency = adj_freq;
    if (ioctl(tuner->fd, VIDIOC_S_FREQUENCY, &vf) == -1)
        fprintf(stderr, "Error VIDIOC_S_FREQUENCY %d\n", errno);
}

static void query_tuner(const struct tuner *tuner, struct v4l2_tuner *vt)
{
    memset(vt, 0, sizeof *vt);
    vt->index = tuner->index;
    if (ioctl(tuner->fd, VIDIOC_G_TUNER, vt) == -1)
        fprintf(stderr, "Error VIDIOC_G_TUNER %d\n", errno);
}
