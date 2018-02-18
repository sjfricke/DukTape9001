#include <stdio.h>
#include <string.h>
#include <stdlib.h>
int main()
{
	//This is the string that is getting split
    char sentence[1024];
	char cstring[1024];
	const char wav[9] = ".wav -i";
	char commandString[1024];
	strcpy(commandString, "ffmpeg -i ");
	//This is the new array of strings which is used to hold the words.
    int i,j,k,cctr,sInt,wctr;
       printf("\n\n Split string by space into words :\n");
       printf("---------------------------------------\n");    
 
    fgets(sentence, sizeof(sentence), stdin);	
	strupr(sentence);
    j=0; cctr=0; sInt = 0; wctr = 0;
    for(i=0;i<=(strlen(sentence));i++)
    {
        // if space or NULL found, assign NULL into newString[ctr]
        if(sentence[i]==' '||sentence[i]=='\0')
        {
			//This copies from the base string 
			if(i == strlen(sentence)){
				memcpy(cstring+cctr,sentence+sInt,j-1);
				cctr = cctr +j -1;//Changes the index for the insertion
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
	system("ffplay -autoexit output.wav");
	system("rm output.wav");
	printf("\n\n");
	printf(commandString);
	printf("\n");
	//printf(commandString);
    return 0;
}

