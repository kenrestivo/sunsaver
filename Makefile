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


test:: all
	./sunsaverEEPROM -h /dev/rfcomm0

test_set:: all
	./sunsaverEEPROM -h -d -s EV_float2=13.40 /dev/rfcomm0


test_dry:: all
	./sunsaverEEPROM -h -d -n -s EV_float2=13.40 /dev/rfcomm0
