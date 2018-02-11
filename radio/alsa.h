#ifndef ALSA_H
#define ALSA_H ALSA_H

int findSoundcard(void);
int audio_volume(long *outvol, char *alsaCard, char *alsaMixer);

#endif /* alsa.h */
