#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include "httplib.h"

#ifndef MAX_CHAR_SIZE
#define MAX_CHAR_SIZE   0x7ff
#endif

/**
 * Copied from: https://rosettacode.org/wiki/URL_encoding#C
 * Note: Never use this in production
*/
static char rfc3986[256];
static char html5[256];

static char *encode_url(const char *url)
{
    char *enc = malloc((strlen(url) * 3) + 1);
    char *tenc;
    int i;

    for (i = 0; i < 256; i++) {
        rfc3986[i] = isalnum(i) || i == '~' || i == '-' || 
            i == '.' || i == '_' ? i : 0;

        html5[i] = isalnum(i) || i == '*' || i == '-' ||
            i == '.' || i == '_' ? i : (i == ' ') ? '+' : 0;
    }

    tenc = enc;
    while (*url) {
        if (rfc3986[*url])
            sprintf(tenc, "%c", rfc3986[*url]);
        else
            sprintf(tenc, "%%%02X", *url);
        url++;

        while (*++tenc);
    }

    return enc;
}

static void web_translate(char *to, char *txt)
{
    int sd;
    struct http_message msg;
    char *enc;
    char req_url[MAX_CHAR_SIZE];

    memset(req_url, '\0', sizeof(req_url));

    strcat(req_url, "https://translate.google.com/translate_a/t?client=p&sl=auto&tl=");
    strcat(req_url, to);
    strcat(req_url, "&dt=t&q=");

    enc = encode_url(txt);
    strcat(req_url, enc);

    sd = http_request(req_url);

    memset(&msg, '\0', sizeof(msg));

    while (http_response(sd, &msg) > 0)
        if (msg.content != NULL) {
            for (int i = 0; i < 3; i++)
                memmove(msg.content, msg.content + i, strlen(msg.content));

            msg.content[strlen(msg.content) - 8] = '\0';
            fprintf(stdout, "%s", msg.content);
        }

    fprintf(stdout, "\n");
    close(sd);
    free(enc);
}

static void help(void)
{
    fprintf(stdout,
        "late-c - A ugly text translator\n\n"
        "Usage\n"
        "late -l [LANG_CODE] [TEXT]\n"
    );
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        help();
        exit(EXIT_SUCCESS);
    }

    char args[MAX_CHAR_SIZE], *lang;
    int opt, i;

    while ((opt = getopt(argc, argv, "l:f:")) != -1) {
        switch (opt) {
        case 'l':
            lang = argv[optind - 1];
            break;

        case 'h':
        default:
            help();
            exit(EXIT_SUCCESS);
        }
    }

    if (strlen(lang) == 0) {
        help();
        exit(EXIT_SUCCESS);
    }

    for (i = 3; i < argc; i++) {
        if (i != 3 && (strcmp(argv[i], " ") != 0))
            strcat(args, " ");
        strcat(args, argv[i]);
    }

    web_translate(lang, args);
}
