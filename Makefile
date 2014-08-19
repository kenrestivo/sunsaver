#names

#build stuff
TARGETS= sunsaver sunsaverEEPROM sunsaverlog sunsavertest sunsaver-raw
CFLAGS:= -g -Wall  -O3  -I. -I/usr/local/include 
LDFLAGS=-lmodbus
COMM=/dev/rfcomm0
TEST_OPTIONS=-h -v -d $(COMM)
DRY_OPTIONS=-h -v -n -d $(COMM)

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
	./sunsaverEEPROM $(TEST_OPTIONS)


test_read: sunsaver
	./sunsaver -h $(COMM)

test_bad_opt_dry:: all
	./sunsaverEEPROM $(DRY_OPTIONS) -s yeehaw=938293



test_set:: all
	./sunsaverEEPROM $(TEST_OPTIONS) -s EV_float2=13.40 
	./sunsaverEEPROM $(TEST_OPTIONS) -s EV_floatlb_trip2=12.7 
	./sunsaverEEPROM $(TEST_OPTIONS) -s EV_float_cancel2=12.5 
	./sunsaverEEPROM $(TEST_OPTIONS) -s Et_float_exit_cum2=1500 
	./sunsaverEEPROM $(TEST_OPTIONS) -s Et_floatlb2=28800


test_dry:: all
	./sunsaverEEPROM $(DRY_OPTIONS) -s EV_float2=13.40 
	./sunsaverEEPROM $(DRY_OPTIONS) -s EV_floatlb_trip2=12.7 
	./sunsaverEEPROM $(DRY_OPTIONS) -s EV_float_cancel2=12.5 
	./sunsaverEEPROM $(DRY_OPTIONS) -s Et_float_exit_cum2=1500 
	./sunsaverEEPROM $(DRY_OPTIONS) -s Et_floatlb2=28800

test_multiopts_dry:: all
	./sunsaverEEPROM $(DRY_OPTIONS) -s EV_float2=13.4  EV_floatlb_trip2=12.7  EV_float_cancel2=12.6  Et_float_exit_cum2=1500 Et_floatlb2=28800

test_multiopts:: all
	./sunsaverEEPROM $(TEST_OPTIONS) -s  EV_float2=13.4  EV_floatlb_trip2=12.7  EV_float_cancel2=12.6  Et_float_exit_cum2=1500 Et_floatlb2=28800


fix_botched:: all
	./sunsaverEEPROM $(TEST_OPTIONS) -s  EV_reg=14.00 EV_float=13.70  Et_float=10800 Et_floatlb=14400  EV_floatlb_trip=12.50 EV_float_cancel=12.30  Et_float_exit_cum=3600

