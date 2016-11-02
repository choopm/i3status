#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/inotify.h>
#include <yajl/yajl_gen.h>
#include <yajl/yajl_version.h>
#include <sys/stat.h>
#include "i3status.h"

#define BUF_LEN (10 * (sizeof(struct inotify_event) + NAME_MAX + 1))

char pbuffer[1024] = "\0";
bool initialRead = true;
int inotifyFd = 0, wd = 0;
char buf[BUF_LEN] __attribute__ ((aligned(8)));

void copy_to_buf(const char *filepath) {
    FILE* fp = fopen(filepath, "r");
    if (!fp) {
        strcpy(pbuffer, "Failed to open file");
    } else {
        if (fgets(pbuffer, sizeof(pbuffer)-1, fp))
            pbuffer[strlen(pbuffer) - 1] = '\0';
        pclose(fp);
    }
}

void print_file_contents(yajl_gen json_gen, char *buffer, const char *title, const char *filepath, bool update_on_change, const char *format) {
        const char *walk;
        char *outwalk = buffer;

        INSTANCE(title);

        if (initialRead) {
            if (!inotifyFd && update_on_change) {
                inotifyFd = inotify_init();
                fcntl(inotifyFd, F_SETFL, fcntl(inotifyFd, F_GETFL) | O_NONBLOCK);
                wd = inotify_add_watch(inotifyFd, filepath, IN_MODIFY);
                if (wd == -1)
                    strcpy(pbuffer, "Failed to add event listener");
            }

            copy_to_buf(filepath);

            initialRead = false;
        } else {
            if (update_on_change) {
                int numRead = read(inotifyFd, buf, BUF_LEN);
                char *p;
                for (p = buf; p < buf + numRead; ) {
                    struct inotify_event *event = (struct inotify_event *) p;
                    if (event->mask & IN_MODIFY) {
                        copy_to_buf(filepath);
                    }
                    p += sizeof(struct inotify_event) + event->len;
                }
            }
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
