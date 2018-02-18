#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/select.h>

#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>
#include <sphinxbase/prim_type.h>

#include "pocketsphinx.h"
#include "gpio.h"

#define BUFF_SIZE 1024

static ps_decoder_t *ps;
static cmd_ln_t *config;

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

#define G1 28
#define G2 33
#define Y1 36
#define Y2 12

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
}

static void
recognize_from_microphone()
{
    ad_rec_t *ad;
    int16 adbuf[BUFF_SIZE];
    uint8 utt_started, in_speech;
    int32 k;
    char const *hyp;

    if ((ad = ad_open_dev("plughw:0,2",16000)) == NULL) { puts("ERROR: Failed to open audio device"); }
    if (ad_start_rec(ad) < 0) {  puts("ERROR: Failed to start recording"); }
    if (ps_start_utt(ps) < 0) { puts("ERROR: Failed to start utterance"); }

    GpioSetValue(G1, 1);
    GpioSetValue(G2, 1);
    GpioSetValue(Y1, 0);
    GpioSetValue(Y2, 0);

    utt_started = 0;
    puts("Ready....");

    for (;;) {
        if ((k = ad_read(ad, adbuf, BUFF_SIZE)) < 0)
            puts("ERROR: Failed to read audio");
	
        ps_process_raw(ps, adbuf, k, FALSE, FALSE);
        in_speech = ps_get_in_speech(ps);

	if (in_speech && !utt_started) {
            utt_started = 1;
            puts("Listening...\n");
        }

	if (!in_speech && utt_started) {
            /* speech -> silence transition, time to start new utterance  */
            ps_end_utt(ps);
            hyp = ps_get_hyp(ps, NULL );

	    GpioSetValue(G1, 0);
	    GpioSetValue(G2, 0);
	    GpioSetValue(Y1, 1);
	    GpioSetValue(Y2, 1);

	    // need to close since we use speaker for output
	    ad_close(ad);
	    if (ps_end_utt(ps) < 0){ puts("ERROR: Failed to stop utterance"); }
	    if (ad_stop_rec(ad) < 0) { puts("ERROR: Failed to stop recording"); }
    
            if (hyp != NULL) {
	      puts("About to do Austin_stuff");
	      austin_stuff(hyp);
            }

	    if ((ad = ad_open_dev("plughw:0,2",16000)) == NULL) { puts("ERROR: Failed to open audio device"); }
	    if (ad_start_rec(ad) < 0) { puts("ERROR: Failed to start recording"); }
	    if (ps_start_utt(ps) < 0){ puts("ERROR: Failed to Start utterance"); }
    
            utt_started = 0;
	    memset(adbuf, 0, BUFF_SIZE);

	    GpioSetValue(G1, 1);
	    GpioSetValue(G2, 1);
	    GpioSetValue(Y1, 0);
	    GpioSetValue(Y2, 0);

            puts("Ready Again....");
        }
	//	usleep(1000000); // 10 ms
	//	puts("sleep over");
    }
    ad_close(ad);
}

int
main(int argc, char *argv[])
{
    err_set_logfile("log.txt");

    GpioOutput(G1, 0);
    GpioOutput(G2, 0);
    GpioOutput(Y1, 1);
    GpioOutput(Y2, 1);
    
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
