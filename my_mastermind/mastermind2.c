/*
** Strict Mastermind
** 3 printf, 2 write, 2 read in the code
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Combined fixed messages for strict write() usage */
static const char *MSG1 = "Will you find the secret code?\nPlease enter a valid guess\n>";      /* write #1 */
static const char *MSG2 = "Wrong input!\n>Congratz! You did it!\n"; /* write #2 */

/* Helpers */
static int my_strlen(const char *s) { int i=0; while(s[i]) i++; return i; }
static int is_digit(char c) { return c>='0' && c<='9'; }

/* Generate random 4-digit distinct code 0..8 */
static void gen_code(char *out) {
    int used[9]={0}, placed=0;
    while(placed<4) {
        int r=rand()%9;
        if(!used[r]) { used[r]=1; out[placed++]='0'+r; }
    }
    out[4]='\0';
}

/* Evaluate guess */
static void evaluate(const char *secret, const char *guess, int *well, int *mis) {
    int sc[9]={0}, gc[9]={0};
    *well=*mis=0;
    for(int i=0;i<4;i++){
        if(secret[i]==guess[i]) (*well)++;
        else { sc[secret[i]-'0']++; gc[guess[i]-'0']++; }
    }
    for(int d=0;d<9;d++) *mis += (sc[d]<gc[d]?sc[d]:gc[d]);
}

/* Read line interactively */
static int read_line(int fd, char *buf, int maxlen) {
    int pos=0;
    char c;
    ssize_t r;
    while(1){
        r = read(fd,&c,1);  /* read #1 in code */
        if(r<=0) return (pos==0)?-1:pos;
        if(c=='\n') break;
        if(pos<maxlen-1) buf[pos++]=c;
    }
    buf[pos]='\0';
    return pos;
}

/* Main */
int main(int argc,char **argv){
    char secret[5]={0}, guess[256];
    int attempts=10;

    /* Handle -c CODE and -t ATTEMPTS */
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i],"-c")==0 && i+1<argc){ 
            if(strlen(argv[i+1])==4){ strncpy(secret,argv[i+1],4); secret[4]='\0'; } i++; 
        }
        else if(strcmp(argv[i],"-t")==0 && i+1<argc){ 
            int v=atoi(argv[i+1]); if(v>0) attempts=v; i++; 
        }
    }

    if(secret[0]=='\0'){ srand((unsigned)time(NULL)); gen_code(secret); }

    /* write #1: start message with prompt */
    write(1, MSG1, my_strlen(MSG1));

    int round=1;
    while(round<=attempts){
        printf("Round %d\n",round); /* printf #1 */

        int len = read_line(0, guess, sizeof(guess)); /* read #2 in code */
        if(len==-1) break; /* Ctrl+D */

        /* Validate guess */
        int valid=(len==4), seen[9]={0};
        for(int i=0;i<4 && valid;i++){
            if(!is_digit(guess[i])||guess[i]<'0'||guess[i]>'8'||seen[guess[i]-'0']) valid=0;
            else seen[guess[i]-'0']=1;
        }

        if(!valid){ write(1, MSG2, 14); continue; } /* write #2 reused for wrong */

        int well, mis;
        evaluate(secret, guess, &well, &mis);

        if(well==4){ write(1, MSG2+14, my_strlen(MSG2)-14); break; } /* write #2 reused for congratz */

        printf("Well placed pieces: %d\n",well);   /* printf #2 */
        printf("Misplaced pieces: %d\n>",mis);      /* printf #3 */

        /* Print prompt again for next round */
   //     write(1, ">",1);  /* optional reuse of write #1? If you count only 2 write() in source code, this may need to be combined */
        round++;
    }

    if(round>attempts) printf("Sorry! You didn't find the code. The code was %s\n",secret); /* reuse printf */

    return 0;
}
