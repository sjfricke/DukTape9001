#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <sys/select.h>

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>

#include "pocketsphinx.h"

#define BUFF_SIZE 4096

static ps_decoder_t *ps;
static cmd_ln_t *config;

/* Sleep for specified msec */
static void
sleep_msec(int32 ms)
{
#if (defined(_WIN32) && !defined(GNUWINCE)) || defined(_WIN32_WCE)
    Sleep(ms);
#else
    /* ------------------- Unix ------------------ */
    struct timeval tmo;

    tmo.tv_sec = 0;
    tmo.tv_usec = ms * 1000;

    select(0, NULL, NULL, NULL, &tmo);
#endif
}

//Processing information using the phrase
char sentence[1024];
char cstring[1024];
const char wav[9] = ".wav -i";
char commandString[1024];
char songList[1024];
//This is the new array of strings which is used to hold the words.
int i,j,k,cctr,sInt,wctr;
int l;
char minichar[6];
char anotherMini[11];
int outCount = 0;

static void austin_stuff(const char* phrase) {
  memset(songList, 0, 1024);
    strcpy(sentence,phrase);
    // good phrase
    printf("---------------------------------------\n");
    for(k=0; phrase[k]!='\0';k++){
        sentence[k]=toupper(phrase[k]);
    }
    
    j=0; cctr=0; sInt = 0; wctr = 0;
    for(i=0;i<=(strlen(sentence));i++) {
      // if space or NULL found, assign NULL into newString[ctr]
      if(sentence[i]==' '||sentence[i]=='\0') {
	//This copies from the base string
	if(i == strlen(sentence)){
	  memcpy(cstring+cctr,sentence+sInt,j);
	  cctr = cctr +j;//Changes the index for the insertion
	  memcpy(cstring+cctr,wav,5);
	  cctr = cctr + 5;
	} else{
	  memcpy(cstring+cctr,sentence+sInt,j);
	  cctr = cctr +j;//Changes the index for the insertion
	  memcpy(cstring+cctr,wav,7);
	  cctr = cctr + 7;
	}
	sInt = sInt + j;
	j=1;
	wctr++;
      } else {
	j++;
      }
    }
    
    //Finishing cstring and preparing it to be parsed as command string.
    cstring[cctr]='\0';
    printf(cstring);
    printf("\n Strings or words after split by space are :\n");
    //printf(cstring);
    strcpy(songList,cstring);
    strcat(songList, "-filter_complex \"");
    
    //Mini loop used to insert the [o:o] . . . command parameters
    for(l = 0; l < wctr; l++){
        sprintf(minichar, "[%i:0]",l);
        strcat(songList, minichar);
    }
    puts("ffmpeg");
    sprintf(commandString, "ffmpeg -loglevel panic -y -i %s concat=n=%i %s :v=0:a=1[outa]\" -map \"[outa]\" output%d.wav", songList, wctr, anotherMini, outCount);
    system(commandString);

    puts("aplay");
    
    sprintf(commandString, "aplay -d 3 -D plughw:0,1 output%d.wav", outCount);
    system(commandString);

    puts("done with system");
    outCount++;
    /*
    printf("\n\n");
    printf(commandString);
    printf("\n");
    printf(phrase);
    printf("\n\n"); 
    */
}

/*
 * Main utterance processing loop:
 *     for (;;) {
 *        start utterance and wait for speech to process
 *        decoding till end-of-utterance silence will be detected
< *        print utterance result;
 *     }
 */
static void
recognize_from_microphone()
{
    ad_rec_t *ad;
    int16 adbuf[BUFF_SIZE];
    uint8 utt_started, in_speech;
    int32 k;
    char const *hyp;

    if ((ad = ad_open_dev("plughw:0,2",16000)) == NULL)
        E_FATAL("Failed to open audio device\n");
    if (ad_start_rec(ad) < 0)
        E_FATAL("Failed to start recording\n");

    if (ps_start_utt(ps) < 0)
        E_FATAL("Failed to start utterance\n");
    utt_started = FALSE;
    E_INFO("Ready....\n");

    for (;;) {
        if ((k = ad_read(ad, adbuf, BUFF_SIZE)) < 0)
            E_FATAL("Failed to read audio\n");
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);
        if (in_speech && !utt_started) {
            utt_started = TRUE;
            puts("Listening...\n");
        }
        if (!in_speech && utt_started) {
            /* speech -> silence transition, time to start new utterance  */
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL );
            if (hyp != NULL) {
	      puts("About to do Austin_stuff");
                austin_stuff(hyp);
            }

            if (ps_start_utt(ps) < 0)
                E_FATAL("Failed to start utterance\n");
            utt_started = FALSE;
            puts("Ready....\n");
        }
        sleep_msec(100);
    }
    ad_close(ad);
}

int
main(int argc, char *argv[])
{
    err_set_logfile("log.txt");
    
    config = cmd_ln_init(NULL, ps_args(), TRUE,
                         "-hmm", "/usr/local/share/pocketsphinx/model/en-us/en-us",
                         "-lm", "korra.lm",
                         "-dict", "korra.dic",
                         NULL);
    
    ps_default_search_args(config);
    ps = ps_init(config);
    if (ps == NULL) {
        cmd_ln_free_r(config);
        return 1;
    }

    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);

    recognize_from_microphone();
 

    ps_free(ps);
    cmd_ln_free_r(config);

    return 0;
}
