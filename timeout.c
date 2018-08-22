/* timeout.c - a little program to blank the RPi touchscreen and unblank it
   on touch.  Original by https://github.com/timothyhollabaugh

   2018-04-16 - Joe Hartley, https://github.com/JoeHartley3
     Added command line parameters for input device and timeout in seconds
     Added nanosleep() to the loop to bring CPU usage from 100% on a single core to around 1%

   Note that when not running X Windows, the console framebuffer may blank and not return on touch.
   Use one of the following fixes:

   * Raspbian Jessie
     Add the following line to /etc/rc.local (on the line before the final exit 0) and reboot:
         sh -c "TERM=linux setterm -blank 0 >/dev/tty0"
     Even though /dev/tty0 is used, this should propagate across all terminals.

   * Raspbian Wheezy
     Edit /etc/kbd/config and change the values for the variable shown below, then reboot:
       BLANK_TIME=0

   2018-04-23 - Moved nanosleep() outside of last if statement, fixed help screen to be consistent with binary name

   2018-08-22 - CJ Vaughter, https://github.com/cjvaughter
     Added support for multiple input devices
*/

#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char* argv[]){
        if (argc < 3) {
                printf("Usage: timeout <timeout_sec> <device> [<device>...]\n");
                printf("    Use lsinput to see input devices.\n");
                printf("    Device to use is shown as /dev/input/<device>\n");
                exit(1);
        }
        int i;
        int tlen;
        int timeout;
        tlen = strlen(argv[1]);
        for (i=0;i<tlen; i++)
                if (!isdigit(argv[1][i])) {
                        printf ("Entered timeout value is not a number\n");
                        exit(1);
                }
        timeout = atoi(argv[1]);

        int num_dev = argc - 2;
        int eventfd[num_dev];
        char device[num_dev][32];
        for (i = 0; i < num_dev; i++) {
                device[i][0] = '\0';
                strcat(device[i], "/dev/input/");
                strcat(device[i], argv[i + 2]);

                int event_dev = open(device[i], O_RDONLY | O_NONBLOCK);
                if(event_dev == -1){
                        int err = errno;
                        printf("Error opening %s: %d\n", device[i], err);
                        exit(1);
                }
                eventfd[i] = event_dev;
        }
        printf("Using input device%s: ", (num_dev > 1) ? "s" : "");
        for (i = 0; i < num_dev; i++) {
                printf("%s ", device[i]);
        }
        printf("\n");

        printf("Starting...\n");
        struct input_event event[64];
        int lightfd;
        int event_size;
        int light_size;
        int size = sizeof(struct input_event);
        char on;

        /* new sleep code to bring CPU usage down from 100% on a core */
        struct timespec sleepTime;
        sleepTime.tv_sec = 0;
        sleepTime.tv_nsec = 10000000L;  /* 0.1 seconds - larger values may reduce load even more */

        lightfd = open("/sys/class/backlight/rpi_backlight/bl_power", O_RDWR);

        if(lightfd == -1){
                int err = errno;
                printf("Error opening backlight file: %d", err);
                exit(1);
        }

        light_size = read(lightfd, &on, sizeof(char));

        if(light_size == -1){
                int err = errno;
                printf("Error reading backlight file: %d", err);
                exit(1);
        }

        time_t now = time(NULL);
        time_t touch = now;

        while(1) {
                for (i = 0; i < num_dev; i++) {               
                        event_size = read(eventfd[i], event, size*64);
                        now = time(NULL);

                        if(event_size != -1) {
                                printf("%s Value: %d, Code: %x\n", device[i], event[0].value, event[0].code);
                                touch = now;

                                if(on == '1') {
                                        printf("Turning On\n");
                                        on = '0';
                                        write(lightfd, &on, sizeof(char));
                                }
                        }

                        if(difftime(now, touch) > timeout) {
                                if(on == '0') {
                                        printf("Turning Off\n");
                                        on = '1';
                                        write(lightfd, &on, sizeof(char));
                                }
                        }
                        nanosleep(&sleepTime, NULL);
                }
        }
}
