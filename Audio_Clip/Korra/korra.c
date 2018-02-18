#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <sphinxbase/err.h>
#include <sphinxbase/ad.h>
#include <pocketsphinx.h>

#define PS_BUF_SIZE 4096

ps_decoder_t *ps;
cmd_ln_t *config;
ad_rec_t *ad; // Audio Device
int16_t buffer[PS_BUF_SIZE];
int32_t n_samples;
uint8_t utt_started = 0;
uint8_t in_speech;
char const* phrase;
char const* output;
char sentence[100];
char fileNames[10][20];
// forward declaration
void Add(char const* phrase);

void DictionarySetup() {

  // Create config object with path to language, .lm file, and .dic file
  config = cmd_ln_init(NULL, ps_args(), TRUE,
                       "-hmm", "/usr/local/share/pocketsphinx/model/en-us/en-us"
,
                       "-lm", "./korra.lm",
                       "-dict", "./korra.dic",
                       NULL);

  if (config == NULL) {
    fprintf(stderr, "Failed to create config object, see log for details\n");
    exit(-1);
  }

  // ps (PocketSphinx) is your main reference value
  ps = ps_init(config);
  if (ps == NULL) {
    fprintf(stderr, "Failed to create recognizer, see log for details\n");
    exit(-1);
  }
}

void GetPhrase() {

  // plughw:0,2 is capture on dragonboard by default
  if ((ad = ad_open_dev("plughw:0,2", 16000)) == NULL) {
    fprintf(stderr, "Failed to open audio device\n");
    exit(-1);
  }

  // start recording
  if (ad_start_rec(ad) < 0) {
    fprintf(stderr, "Failed to start recording\n");
    return;
  }

  // start utterance
  if (ps_start_utt(ps) < 0) {
    fprintf(stderr, "Failed to start utterance\n");
    return;
  }

  printf("\nSAY YOUR PHRASE AVATAR...\n\n");

  // Loop until word is fetched
  while(1) {

    // Start getting data
    if ((n_samples = ad_read(ad, buffer, PS_BUF_SIZE)) < 0) {
      fprintf(stderr, "Failed to read audio\n");
      ad_close(ad);
      return;
    }

    ps_process_raw(ps, buffer, n_samples, FALSE, FALSE);
    in_speech = ps_get_in_speech(ps);

    if (in_speech && !utt_started) {
      utt_started = 1;
    }

    if (!in_speech && utt_started) {
      ps_end_utt(ps);
      phrase = ps_get_hyp(ps, NULL);
	  strcpy(sentence,phrase);
/*       if(phrase == NULL){
        fprintf(stderr, "Phrase was null\n");
        ad_close(ad);
        return;
      }

      if (ps_start_utt(ps) < 0) {
        fprintf(stderr, "Failed to start utterance\n");
        ad_close(ad);
        return;
      } */
          return;
    }
  } // while(1)
}

void CleanUp() {
  ps_free(ps);
  cmd_ln_free_r(config);
}


int main(int argc, char* argv[]) {

  DictionarySetup();
  while(1){
  GetPhrase();
  	//This is the string that is getting split
    char sentence[1024];
	char cstring[1024];
	const char wav[9] = ".wav -i";
	char commandString[1024];
	strcpy(commandString, "ffmpeg -i ");
	printf("\n\n");
	printf(phrase);
	printf("\n\n");
	//This is the new array of strings which is used to hold the words.
    int i,j,k,cctr,sInt,wctr;
       printf("\n\n Split string by space into words :\n");
       printf("---------------------------------------\n");    
	for(k=0; phrase[k]!='\0';k++){
		sentence[k]=toupper(phrase[k]);
	}
    j=0; cctr=0; sInt = 0; wctr = 0;
    for(i=0;i<=(strlen(sentence));i++)
    {
        // if space or NULL found, assign NULL into newString[ctr]
        if(sentence[i]==' '||sentence[i]=='\0')
        {
			//This copies from the base string 
			if(i == strlen(sentence)){
				memcpy(cstring+cctr,sentence+sInt,j);
				cctr = cctr +j;//Changes the index for the insertion
				memcpy(cstring+cctr,wav,5);
				cctr = cctr + 5;
			}
			else{
				memcpy(cstring+cctr,sentence+sInt,j);
				cctr = cctr +j;//Changes the index for the insertion
				memcpy(cstring+cctr,wav,7);
				cctr = cctr + 7;
			}
			sInt = sInt + j;
			//printf("\nsInt = %i\n",sInt);
			//printf(cstring);
            j=1;    //for next word, init index to 0
			wctr++;
        }
        else
        {
            //newString[ctr][j]=sentence[i];
            j++;
        }
	}
    
	//cstring[cctr] = '\0';
	cstring[cctr]='\0';
	printf("\n Strings or words after split by space are :\n");
	//printf(cstring);
	strcat(commandString,cstring);
	strcat(commandString, "-filter_complex \"");
	//printf(commandString);
	//printf(commandString);
	int l;
	
	for(l = 0; l < wctr; l++){
		char minichar[6];
		sprintf(minichar, "[%i:0]",l);
		strcat(commandString, minichar);
		
	}
	char anotherMini[11];
	sprintf(anotherMini,"concat=n=%i",wctr);
	strcat(commandString,anotherMini);
	strcat(commandString,":v=0:a=1[outa]\" -map \"[outa]\" output.wav");
	system(commandString);
	system("aplay -D plughw:0,1 output.wav");
	system("rm output.wav");
	printf("\n\n");
	printf(commandString);
	printf("\n");
	//printf(commandString);
    printf("\n");
	printf(phrase);
	printf("\n\n");

	CleanUp();
	}
	return 0;

	  
}
