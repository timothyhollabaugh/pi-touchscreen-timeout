#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input.h>
#include <time.h>

#define timeout 10

int main(int argc, char* argv[]){
	printf("Starting...\n");
	struct input_event event[64];
	int eventfd;
	int lightfd;
	int event_size;
	int light_size;
	int size = sizeof(struct input_event);
	char on;
	
	eventfd = open("/dev/input/event0", O_RDONLY | O_NONBLOCK);
	
	if(eventfd == -1){
		int err = errno;
		printf("Error opening touchscreen device: %d", err);
		exit(1);
	}

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

	while(1){
		event_size = read(eventfd, event, size*64);
		now = time(NULL);
		
		if(event_size != -1){
			//printf("Touched! Value: %d, Code: %x\n", event[0].value, event[0].code);
			touch = now;

			if(on == '1'){
				printf("Turning On\n");
				on = '0';
				write(lightfd, &on, sizeof(char));
			}
		}

		if(difftime(now, touch) > timeout){
			if(on == '0'){
				printf("Turning Off\n");
				on = '1';
				write(lightfd, &on, sizeof(char));
			}
		}
	}
}
