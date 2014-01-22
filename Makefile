#names

#build stuff
TARGETS= sunsaver sunsaverEEPROM sunsaverlog sunsavertest sunsaver-raw
CFLAGS:= -g -Wall  -O3  -I. -I/usr/local/include 
LDFLAGS=-lmodbus

# so i can get ssl cflags in there

%.o:%.c
	$(CC) $(CFLAGS) $(SSLCFLAGS) -c $< 

# 

all: $(TARGETS)

clean::
	rm -f $(TARGETS) *.o

install: $(TARGETS)
	(for i in  $(TARGETS); do cp $$i /usr/local/bin/; done)
