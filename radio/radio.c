/* radio.c simly switches on the radio, derived from:
   fm.c - simple V4L2 compatible tuner for radio cards

   Copyright (C) 2004, 2006, 2009, 2012 Ben Pfaff <blp@cs.stanford.edu>
   Copyright (C) 1998 Russell Kroll <rkroll@exploits.org>
   Copyright (C) 2017 rpi Receiver <rpi-receiver@htl-steyr.ac.at>

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
   https://github.com/zonque/simple-alsa-loop/blob/master/loop.c
*/

#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "fmlib.h"
#include "alsa.h"

#define MAX_STRING_LEN 30

void signal_handler(int);
bool charDeviceIsAccessible(const char* filename);
bool charDeviceIsAccessibleAndContainsString(const char* filename, const char* content);
int findSoundcard(void);

static void usage(void)
{
    fprintf(stdout, "radio version %s\n\n", VERSION);
    fprintf(stdout, "usage: radio [-h] [-d <dev>] [-t <tuner>] <freq>\n\n");
    fprintf(stdout, "A small controller for Video for Linux radio devices.\n\n");
    fprintf(stdout, "  -h         display this help\n");
    fprintf(stdout, "  -d <dev>   select device (default: /dev/radio0)\n");
    fprintf(stdout, "  -t <tuner> select tuner (default: 0)\n");
    fprintf(stdout, "  <freq>     frequency in MHz (i.e. 94.3)\n");
    exit(EXIT_SUCCESS);
}

void signal_handler(int sig)
{
    return;
}

static void getconfig(const char *fn,
          long *volume, double *frequency, char *radioDevice,
          char *alsaCard, char *alsaMixer)
{
    FILE    *conf;
    char    buf[256];

    if (!fn) {
        snprintf(buf, sizeof buf, "%s/.radio", getenv("HOME"));
        fn = buf;
    }
    conf = fopen(fn, "r");

    if (!conf)
        return;

    while(fgets(buf, sizeof(buf), conf)) {
        buf[strlen(buf)-1] = 0; // discard LF
        if (strncmp("VOLUME", buf, sizeof("VOLUME") - 1) == 0)
            if(sscanf(buf, "%*s %lu", volume) != 1)
                fprintf(stderr, "Error reading volume\n");
        if (strncmp("FREQUENCY", buf, sizeof("FREQUENCY") - 1) == 0)
            if(sscanf(buf, "%*s %lf", frequency) != 1)
                fprintf(stderr, "Error reading frequency\n");
        if (strncmp("RADIODEVICE", buf, sizeof("RADIODEVICE") - 1) == 0)
            if(sscanf(buf, "%*s %30s", radioDevice) != 1)
                fprintf(stderr, "Error reading radioDevice\n");
        if (strncmp("ALSACARD", buf, sizeof("ALSACARD") - 1) == 0)
            if(sscanf(buf, "%*s %30s", alsaCard) != 1)
                fprintf(stderr, "Error reading alsaCard\n");
        if (strncmp("ALSAMIXER", buf, sizeof("ALSAMIXER") - 1) == 0)
            if(sscanf(buf, "%*s %30s", alsaMixer) != 1)
                fprintf(stderr, "Error reading alsaMixer\n");
    }

    fclose(conf);
}

bool charDeviceIsAccessible(const char* filename) {
    struct stat buffer;

    if(stat(filename,&buffer) == 0) {
        if ((buffer.st_mode & (S_IFCHR & S_IWGRP)) == (S_IFCHR & S_IWGRP)) {
            fprintf(stdout, "writeable character device\n");
            return true;
        }
    }
    return false;
}

bool charDeviceIsAccessibleAndContainsString(const char* filename, const char* content) {
    struct stat buffer;
    FILE *fp;
    char mybuffer[255];

    if(stat(filename,&buffer) == 0) {
        fp = fopen(filename, "r");
        fgets(mybuffer, 255, (FILE*)fp);
        fclose(fp);
        if(strstr(mybuffer, content)) {
            return true;
        }
    }
    return false;
}

int main(int argc, char **argv)
{
    struct tuner tuner;
    const char *device = NULL;
    int err;
    int index = 0;
    long volume = 10;
    double frequency = 0;
    const char *config_file = NULL;
    char radioDevice[MAX_STRING_LEN] = "/dev/radio0";
    char alsaCard[MAX_STRING_LEN] = "default";
    char alsaMixer[MAX_STRING_LEN] = "Master";
    pid_t pid;

    for (;;) {
        int option = getopt(argc, argv, "+hc::t:d:");
        if (option == -1)
            break;

    switch (option) {
        case 't':
            index = atoi(optarg);
            break;
        case 'd':
            device = optarg;
            break;
        case 'c':
            config_file = optarg;
            getconfig(config_file, &volume, &frequency, radioDevice,
                      alsaCard, alsaMixer);
            break;
        case 'h':
        default:
            usage();
            break;
        }
    }

    argc -= optind;
    argv += optind;

    if ((argc == 0) && (frequency == 0))      /* no frequency given */
        usage();

    while(!charDeviceIsAccessible(radioDevice)) {
        sleep(0.2);
    }
    findSoundcard();
    err = audio_volume(&volume, alsaCard, alsaMixer);
    if (err)
        fprintf(stderr, "Error setting volume %d\n", err);
    pid = fork ();
    if (pid == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "/usr/bin/arecord -D hw:rpiReceiver,1 --format=S16_LE --channels=2 --rate=44100 | /usr/bin/aplay -D hw:rpiReceiver,0", (char *) NULL);
        _exit (EXIT_FAILURE);
    }
    else if (pid < 0)
        /* The fork failed.  Report failure.  */
        exit(EXIT_FAILURE);
    else {
        /* This is the parent process.  Wait for the child to complete.  */

        while(!charDeviceIsAccessibleAndContainsString("/proc/asound/card0/pcm1c/sub0/status", "RUNNING")) {
            sleep(0.2);
        }

        signal(SIGINT, signal_handler);
        signal(SIGTERM, signal_handler);
        signal(SIGABRT, signal_handler);
        signal(SIGUSR1, signal_handler);
        tuner_open(&tuner, device, index);

        if (argc > 0)
            if (atof(argv[0]))
                frequency = atof(argv[0]);
        if (frequency != 0) {
            tuner_set_freq(&tuner, frequency * 16000.0, 0);
            fprintf(stdout, "Radio tuned to %2.2f MHz\n", frequency);
        } else {
            fprintf(stderr, "Error unrecognized command syntax; use --help for help\n");
        }
        fprintf(stdout, "Waiting for CTRL-C to exit\n");
        pause();
        tuner_close(&tuner);
        kill(pid,SIGINT);
        return (EXIT_SUCCESS);
    }
}
