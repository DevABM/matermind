/*
** my_mastermind.c
**
** Allowed syscalls/functions used:
**   printf, write, read, rand, srand, time, atoi, strcmp
**
** Reads input 1 char at a time using read(0, &c, 1).
** Supports: -c "CODE" (4 distinct digits from '0'..'8') and -t ATTEMPTS (positive int)
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* helpers (we avoid using disallowed functions) */
static int my_strlen(const char *s)
{
    int i = 0;
    while (s && s[i]) i++;
    return i;
}

static int is_digit(char c) { return (c >= '0' && c <= '9'); }

/* validate that code is 4 distinct chars from '0'..'8' */
static int valid_code(const char *s)
{
    if (!s) return 0;
    if (my_strlen(s) != 4) return 0;
    int used[9] = {0};
    for (int i = 0; i < 4; ++i)
    {
        char c = s[i];
        if (c < '0' || c > '8') return 0;
        int idx = c - '0';
        if (used[idx]) return 0;
        used[idx] = 1;
    }
    return 1;
}

/* generate a random code of 4 distinct digits 0..8 */
static void gen_code(char *out)
{
    int used[9] = {0};
    int placed = 0;
    while (placed < 4)
    {
        int r = rand() % 9; /* 0..8 */
        if (!used[r])
        {
            used[r] = 1;
            out[placed++] = '0' + r;
        }
    }
    out[4] = '\0';
}

/* read a single line from stdin using read(0,...), return length (excl newline)
   - If EOF (read returns 0 immediately), returns -1
   - Buffer must be big enough (we use 256)
*/
static int read_line(char *buf, int maxlen)
{
    int pos = 0;
    char c;
    ssize_t r;
    while (1)
    {
        r = read(0, &c, 1);
        if (r == 0) /* EOF */
        {
            if (pos == 0) return -1;
            /* treat as end of line */
            break;
        }
        if (r < 0) return -1;
        if (c == '\n') break;
        if (pos < maxlen - 1)
            buf[pos++] = c;
        /* else drop extra chars */
    }
    buf[pos] = '\0';
    return pos;
}

/* count well placed and misplaced pieces
   secret and guess are 4-char strings of digits '0'..'8'
*/
static void evaluate_guess(const char *secret, const char *guess, int *well, int *misplaced)
{
    *well = 0;
    *misplaced = 0;
    int secret_count[9] = {0};
    int guess_count[9] = {0};

    for (int i = 0; i < 4; ++i)
    {
        if (secret[i] == guess[i])
            (*well)++;
        else
        {
            secret_count[secret[i] - '0']++;
            guess_count[guess[i] - '0']++;
        }
    }
    for (int d = 0; d < 9; ++d)
        *misplaced += (secret_count[d] < guess_count[d]) ? secret_count[d] : guess_count[d];
}

int main(int argc, char **argv)
{
    char secret[5] = {0};
    int attempts = 10; /* default attempts */

    /* parse args - simple linear parse */
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(argv[i], "-c") == 0 && i + 1 < argc)
        {
            if (valid_code(argv[i+1]))
            {
                strncpy(secret, argv[i+1], 4);
                secret[4] = '\0';
            }
            else
            {
                /* invalid provided code -> ignore and generate later */
                secret[0] = '\0';
            }
            i++;
        }
        else if (strcmp(argv[i], "-t") == 0 && i + 1 < argc)
        {
            int val = atoi(argv[i+1]);
            if (val > 0) attempts = val;
            i++;
        }
        /* unknown args ignored */
    }

    /* generate code if none valid provided */
    if (secret[0] == '\0')
    {
        srand((unsigned)time(NULL));
        gen_code(secret);
    }

    /* Game start prompt (matches example) */
   printf("Please enter a valid guess\n");
// printf("Will you find the secret code?\n");

    /* We'll use a buffer for guesses */
    char buf[256];

    int round = 0;
    while (round < attempts)
    {
        printf("---\n");
        printf("Round %d\n", round);
        printf(">");

        int len = read_line(buf, sizeof(buf));
        if (len == -1)
        {
            /* EOF (Ctrl+D) during input - treat as normal termination */
            /* just exit main normally */
            return 0;
        }

        /* Validate guess: must be 4 chars, each between '0' and '8' */
        int valid = 1;
        if (len != 4) valid = 0;
        else
        {
            for (int k = 0; k < 4; ++k)
                if (!is_digit(buf[k]) || buf[k] < '0' || buf[k] > '8')
                {
                    valid = 0;
                    break;
                }
        }

        if (!valid)
        {
            printf("Wrong input!\n");
            /* do not consume an attempt, re-ask same round */
            continue;
        }

        /* Evaluate */
        int well = 0, misplaced = 0;
        evaluate_guess(secret, buf, &well, &misplaced);

        if (well == 4)
        {
            printf("Congratz! You did it!\n");
            return 0;
        }
        else
        {
            printf("Well placed pieces: %d\n", well);
            printf("Misplaced pieces: %d\n", misplaced);
        }

        round++;
    }

    /* If here, attempts exhausted - reveal code */
    printf("Sorry! You didn't find the code. The code was %s\n", secret);
    return 0;
}

