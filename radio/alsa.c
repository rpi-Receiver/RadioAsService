#include <unistd.h>
#include <fcntl.h>
#include <alsa/asoundlib.h>

int audio_volume(long *outvol, char *alsaCard, char *alsaMixer)
{
    snd_mixer_t* handle;
    snd_mixer_elem_t* elem;
    snd_mixer_selem_id_t* sid;

    static int mix_index = 0;
    int ret = 0;
    long minv, maxv;

    snd_mixer_selem_id_alloca(&sid);

    //sets simple-mixer index and name
    snd_mixer_selem_id_set_index(sid, mix_index);
    snd_mixer_selem_id_set_name(sid, alsaMixer);

    if ((snd_mixer_open(&handle, 0)) < 0)
        return -1;
    if ((snd_mixer_attach(handle, alsaCard)) < 0) {
        snd_mixer_close(handle);
        return -2;
    }
    if ((snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
        snd_mixer_close(handle);
        return -3;
    }
    ret = snd_mixer_load(handle);
    if (ret < 0) {
        snd_mixer_close(handle);
        return -4;
    }
    elem = snd_mixer_find_selem(handle, sid);
    if (!elem) {
        snd_mixer_close(handle);
        return -5;
    }

    snd_mixer_selem_get_playback_volume_range (elem, &minv, &maxv);
    fprintf(stderr, "Volume range <%li,%li>\n", minv, maxv);

    if(*outvol < minv || *outvol > maxv) // out of bounds
        return -7;

    if(snd_mixer_selem_set_playback_volume_all(elem, *outvol) < 0) {
        snd_mixer_close(handle);
        return -8;
    }
    fprintf(stderr, "Set volume %li with status %i\n", *outvol, ret);

    snd_mixer_close(handle);
    return 0;
    }

int findSoundcard(void) {
    register int  err;      //Used for checking the return status of functions.
    int cardNum;            //The card number is stored here.
    cardNum = -1;           //ALSA starts numbering at 0 so this is intially set to -1.
    for (;;)
    {
        snd_ctl_t *cardHandle;      //This is your sound card. But not yet. Now it is just a variable.
        //Get next sound card's card number.
        //When "cardNum" == -1, then ALSA
        //fetches the first card
        if ((err = snd_card_next(&cardNum)) < 0)
        {
            fprintf(stderr, "Can't get the next card number: %s\n", snd_strerror(err));
            break;
        }

        //No more cards? ALSA sets "cardNum" to -1 if so
        if (cardNum < 0) break;

        //Open this card's (cardNum's) control interface.
        //We specify only the card number -- not any device nor sub-device too
        {
            char   str[64];
            sprintf(str, "hw:%i", cardNum);
            if ((err = snd_ctl_open(&cardHandle, str, 0)) < 0)  //Now cardHandle becomes your sound card.
            {
                fprintf(stderr, "Can't open card %i: %s\n", cardNum, snd_strerror(err));
                continue;
            }
        }

        {
            snd_ctl_card_info_t *cardInfo;  //Used to hold card information
            //We need to get a snd_ctl_card_info_t. Just alloc it on the stack
            snd_ctl_card_info_alloca(&cardInfo);
            //Tell ALSA to fill in our snd_ctl_card_info_t with info about this card
            if ((err = snd_ctl_card_info(cardHandle, cardInfo)) < 0)
                fprintf(stderr, "Can't get info for card %i: %s\n", cardNum, snd_strerror(err));
            else
                fprintf(stdout, "Card %i = %s\n", cardNum, snd_ctl_card_info_get_name(cardInfo));
        }
        // Close the card's control interface after we're done with it
        snd_ctl_close(cardHandle);
    }

    //ALSA allocates some mem to load its config file when we call some of the
    //above functions. Now that we're done getting the info, let's tell ALSA
    //to unload the info and free up that mem
    snd_config_update_free_global();
    return 0;
}
