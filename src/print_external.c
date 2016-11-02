#include <stdio.h>
#include <string.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_version.h>
#include <sys/stat.h>
#include "i3status.h"

void print_external(yajl_gen json_gen, char *buffer, const char *title, const char *command, const char *format) {
        const char *walk;
        char *outwalk = buffer;
        char pbuffer[1024] = "\0";
        FILE *fp;

        INSTANCE(title);

        fp = popen(command, "r");
        if (!fp) {
            strcpy(pbuffer, "Failed");
        } else {
            if (fgets(pbuffer, sizeof(pbuffer)-1, fp))
                pbuffer[strlen(pbuffer) - 1] = '\0';
            pclose(fp);
        }
            
        for (walk = format; *walk != '\0'; walk++) {
                if (*walk != '%') {
                        *(outwalk++) = *walk;
                        continue;
                }

                if (strncmp(walk+1, "title", strlen("title")) == 0) {
                        outwalk += sprintf(outwalk, "%s", title);
                        walk += strlen("title");
                } else if (strncmp(walk+1, "status", strlen("status")) == 0) {
                        outwalk += sprintf(outwalk, "%s", pbuffer);
                        walk += strlen("status");
                }
        }

        END_COLOR;
        OUTPUT_FULL_TEXT(buffer);
}
