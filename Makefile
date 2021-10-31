CFLAGS=-Wall

BINDIR=/usr/local/bin
MANDIR=/usr/local/share/man/man1

LINK_LIBS=-lopenal -lm -lpthread -lflite_cmu_us_kal16 -lflite_cmu_us_slt \
 -lflite_usenglish -lflite_cmulex -lflite -lm -lgpiod -lasound -lcurl

lampbox : main.c phrases.h phrases.c speak.h speak.c listen.h listen.c mute.h mute.c \
 arduino.h arduino.c play.h play.c record.h record.c sound.h sound.c gpio_ctrl.h gpio_ctrl.c sonar.h sonar.c \
 parse_web.h parse_web.c
	gcc $(CFLAGS) -o lampbox main.c phrases.c speak.c listen.c mute.c arduino.c play.c record.c sound.c sonar.c gpio_ctrl.c parse_web.c \
 $(LINK_LIBS)

clean :
	rm -f lampbox *.o
