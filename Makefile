#names

#build stuff
TARGETS= sunsaver sunsaverEEPROM sunsaverlog sunsavertest sunsaver-raw
CFLAGS:= -g -Wall  -O3  -I. -I/usr/local/include 
LDFLAGS=-lmodbus
TEST_OPTIONS=-h -d 
DRY_OPTIONS=-h -d -n
COMM=/dev/rfcomm0

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
	./sunsaverEEPROM -h $(COMM)

test_set:: all
	./sunsaverEEPROM $(TEST_OPTIONS) -s EV_float2=13.40 $(COMM)
	./sunsaverEEPROM $(TEST_OPTIONS) -s EV_floatlb_trip2=12.7 $(COMM)
	./sunsaverEEPROM $(TEST_OPTIONS) -s EV_float_cancel2=12.5 $(COMM)
	./sunsaverEEPROM $(TEST_OPTIONS) -s Et_float_exit_cum2=1500 $(COMM)
	./sunsaverEEPROM $(TEST_OPTIONS) -s EV_float2=13.4 $(COMM)


test_dry:: all
	./sunsaverEEPROM $(DRY_OPTIONS) -s EV_float2=13.40 $(COMM)
	./sunsaverEEPROM $(DRY_OPTIONS) -s EV_floatlb_trip2=12.7 $(COMM)
	./sunsaverEEPROM $(DRY_OPTIONS) -s EV_float_cancel2=12.5 $(COMM)
	./sunsaverEEPROM $(DRY_OPTIONS) -s Et_float_exit_cum2=1500 $(COMM)
	./sunsaverEEPROM $(DRY_OPTIONS) -s EV_float2=13.4 $(COMM)
