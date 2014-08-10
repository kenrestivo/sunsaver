/*
 *  sunsaverEEPROM.c - This program reads all the EEPROM registers on a Moringstar SunSaver MPPT and prints the results.
 *  

 Copyright 2010 Tom Rinehart.

 Updated 2014 ken restivo <ken@restivo.org>

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/


/* Compile with: cc sunsaverEEPROM.c -o sunsaverEEPROM -lmodbus */

#include <stdio.h>
#include <stdlib.h>
#include <modbus/modbus.h>
#include <getopt.h>
#include <string.h>
#include <unistd.h>

#define SUNSAVERMPPT    0x01	/* MODBUS Address of the SunSaver MPPT */
#define NO_SUCH_REG 0xFFFF

#define FORCE_EEPROM_UPDATE 0x16
#define RESET_CONTROL 0xff

int debug  =  0;
int dry  =  0;

typedef struct reg_t {
	int addr;
	char * name;
	uint16_t (* from_display)(float);
	float (* to_display)(uint16_t);
} reg;


void usage()
{
	printf("usage: [options] serial_port_path\noptions:\n\t-h help\n\t-d debug\n\t-n dry run (no comms)\n\t-s reg_name=val set register\n");

}

uint16_t straight_cast_int(float f)
{
	return (uint16_t)f;
}


float straight_cast_float(uint16_t i)
{
	return (float)i;
}


uint16_t float_to_int(float f)
{
	return (uint16_t)((f / 100) * 32768);
}


float to_float(uint16_t val)
{
	return (val * 100.0) / 32768.0;
}


float unsigned_to_float(uint16_t val)
{
	return (val * 100.0) / 65536.0;
}


float fl_conv_2(uint16_t val)
{
	return (val * 79.16) / 32768.0;
}

uint32_t shift_32(uint16_t hi, uint16_t lo)
{
	return (hi << 16) + lo;
}


float shift_float(uint16_t hi, uint16_t lo)
{

	return shift_32(hi, lo) * 0.1;
}


#define NUM_REGS 5
static const reg regs[NUM_REGS] = {
	{ 0xE00E, "EV_float2",  float_to_int, to_float     },
	{ 0xE011, "EV_floatlb_trip2", float_to_int, to_float},
	{ 0xE012, "EV_float_cancel2", float_to_int, to_float},
	{ 0xE013, "Et_float_exit_cum2", straight_cast_int, straight_cast_float},
	// TODO: the rest of the registers!
	{ NO_SUCH_REG, "NO SUCH REG", NULL,  NULL     } // last one must always be the incorrect one
};

reg lookup_reg(char * name)
{
	int i;
	for(i = 0; i < NUM_REGS; i++){
		if(0 == strcmp(regs[i].name, name)){
			// TODO: just pass the pointer, it's a const struct anyway
			return regs[i];
		}
	}
	return regs[NUM_REGS - 1]; // last one is the illegal reg
}



void commit_options(modbus_param_t * mb_param)
{

	int ret;

	// XXX Redundant? seems to do nothing
	if(debug > 0){
		printf ("now forcing the eeprom\n");
	}	       
	ret = force_single_coil(mb_param, SUNSAVERMPPT, FORCE_EEPROM_UPDATE, 1);

	if(debug > 0){
		printf("set returnd %d\n", ret);
	}

	sleep(2); // just to be sure

	if(debug > 0){
		printf ("now forcing the reset\n");
	}	       
	ret = force_single_coil(mb_param, SUNSAVERMPPT, RESET_CONTROL, 1);
			       
	if(debug > 0){
		printf("set returnd %d\n", ret);
	}

}


void set_options(modbus_param_t * mb_param, char * arg){
	int ret;
	uint16_t set_val;
	reg r;
	char key[256];
	char * val;

	strcpy (key, arg);
	strtok_r (key, "=", &val);

	if(debug > 0){
		printf("arg %s, key %s, val %s\n", arg, key, val);
	}


	r = lookup_reg(key); 


	if(NO_SUCH_REG == r.addr){
		printf("Error, %s is not a register\n", key);
		usage();
		return;
	}


	set_val = (*(r.from_display))(strtof(val, NULL));

	if(debug > 0){
		printf("%s is %s at 0x%04x, as int is %d\n", key, r.name, r.addr, set_val);
	}						 

	if(dry > 0 ){
		printf("Dry run, not setting anything\n");
	} else {
		// Now actually set the stuff
		ret = preset_single_register(mb_param, SUNSAVERMPPT, r.addr, set_val);
	
		if(debug > 0){
			printf("set returnd %d\n", ret);
		}

		sleep(2); // just to be sure
		
		commit_options(mb_param);
		
	}

}


void read_values(modbus_param_t * mb_param)
{
	int ret;
	float EV_reg, EV_float, EV_floatlb_trip, EV_float_cancel, EV_eq;
	unsigned short Et_float, Et_floatlb, Et_float_exit_cum, Et_eqcalendar, Et_eq_above, Et_eq_reg;
	float EV_reg2, EV_float2, EV_floatlb_trip2, EV_float_cancel2, EV_eq2;
	unsigned short Et_float2, Et_floatlb2, Et_float_exit_cum2, Et_eqcalendar2, Et_eq_above2, Et_eq_reg2;
	float EV_tempcomp, EV_hvd, EV_hvr, Evb_ref_lim;
	short ETb_max, ETb_min;
	float EV_lvd, EV_lvr, EV_lhvd, EV_lhvr, ER_icomp, Et_lvd_warn;
	float EV_soc_y2g, EV_soc_g2y, EV_soc_y2r0, EV_soc_r2y;
	unsigned short Emodbus_id, Emeter_id;
	float Eic_lim;
	unsigned int Ehourmeter;
	short Etmr_eqcalendar;
	float EAhl_r, EAhl_t, EAhc_r, EAhc_t, EkWhc, EVb_min, EVb_max, EVa_max;
	uint16_t data[50];

	// TODO: use the dispatch table regs, instead of this cut/paste stuff.

	
	/* Read the 0xE000 EEPROM Registers and convert the results to their proper values */
	ret  =  read_input_registers(mb_param, SUNSAVERMPPT, 0xE000, 11, data);
	
	printf("EEPROM Registers\n\n");
	
	printf("Charge Settings (bank 1)\n");
	
	EV_reg = to_float(data[0]);
	printf("EV_reg  =  %.2f V\n",EV_reg);
	
	EV_float = to_float(data[1]);
	printf("EV_float  =  %.2f V\n",EV_float);
	
	Et_float = data[2];
	printf("Et_float  =  %d s\n",Et_float);
	
	Et_floatlb = data[3];
	printf("Et_floatlb  =  %d s\n",Et_floatlb);
	
	EV_floatlb_trip = to_float(data[4]);
	printf("EV_floatlb_trip  =  %.2f V\n",EV_floatlb_trip);
	
	EV_float_cancel = to_float(data[5]);
	printf("EV_float_cancel  =  %.2f V\n",EV_float_cancel);
	
	Et_float_exit_cum = data[6];
	printf("Et_float_exit_cum  =  %d s\n",Et_float_exit_cum);
	
	EV_eq = to_float(data[7]);
	printf("EV_eq  =  %.2f V\n",EV_eq);
	
	Et_eqcalendar = data[8];
	printf("Et_eqcalendar  =  %d days\n",Et_eqcalendar);
	
	Et_eq_above = data[9];
	printf("Et_eq_above  =  %d s\n",Et_eq_above);
	
	Et_eq_reg = data[10];
	printf("Et_eq_reg  =  %d s\n",Et_eq_reg);
	
	/* Read the 0xE00D EEPROM Registers and convert the results to their proper values */
	ret  =  read_input_registers(mb_param, SUNSAVERMPPT, 0xE00D, 11, data);
	
	printf("\nCharge Settings (bank 2)\n");
	
	EV_reg2 = to_float(data[0]);
	printf("EV_reg2  =  %.2f V\n",EV_reg2);
	
	EV_float2 = to_float(data[1]);
	printf("EV_float2  =  %.2f V\n",EV_float2);
	
	Et_float2 = data[2];
	printf("Et_float2  =  %d s\n",Et_float2);
	
	Et_floatlb2 = data[3];
	printf("Et_floatlb2  =  %d s\n",Et_floatlb2);
	
	EV_floatlb_trip2 = to_float(data[4]);
	printf("EV_floatlb_trip2  =  %.2f V\n",EV_floatlb_trip2);
	
	EV_float_cancel2 = to_float(data[5]);
	printf("EV_float_cancel2  =  %.2f V\n",EV_float_cancel2);
	
	Et_float_exit_cum2 = data[6];
	printf("Et_float_exit_cum2  =  %d s\n",Et_float_exit_cum2);
	
	EV_eq2 = to_float(data[7]);
	printf("EV_eq2  =  %.2f V\n",EV_eq2);
	
	Et_eqcalendar2 = data[8];
	printf("Et_eqcalendar2  =  %d days\n",Et_eqcalendar2);
	
	Et_eq_above2 = data[9];
	printf("Et_eq_above2  =  %d s\n",Et_eq_above2);
	
	Et_eq_reg2 = data[10];
	printf("Et_eq_reg2  =  %d s\n",Et_eq_reg2);
	
	/* Read the 0xE01A EEPROM Registers and convert the results to their proper values */
	ret  =  read_input_registers(mb_param, SUNSAVERMPPT, 0xE01A, 6, data);
	
	printf("\nCharge Settings (shared)\n");
	
	EV_tempcomp = unsigned_to_float(data[0]);
	printf("EV_tempcomp  =  %.2f V\n",EV_tempcomp);
	
	EV_hvd = to_float(data[1]);
	printf("EV_hvd  =  %.2f V\n",EV_hvd);
	
	EV_hvr = to_float(data[2]);
	printf("EV_hvr  =  %.2f V\n",EV_hvr);
	
	Evb_ref_lim = to_float(data[3]);
	printf("Evb_ref_lim  =  %.2f V\n",Evb_ref_lim);
	
	ETb_max = data[4];
	printf("ETb_max  =  %d °C\n",ETb_max);
	
	ETb_min = data[5];
	printf("ETb_min  =  %d °C\n",ETb_min);
	
	/* Read the 0xE022 EEPROM Registers and convert the results to their proper values */
	ret  =  read_input_registers(mb_param, SUNSAVERMPPT, 0xE022, 6, data);
	
	printf("\nLoad Settings\n");
	
	EV_lvd = to_float(data[0]);
	printf("EV_lvd  =  %.2f V\n",EV_lvd);
	
	EV_lvr = to_float(data[1]);
	printf("EV_lvr  =  %.2f V\n",EV_lvr);
	
	EV_lhvd = to_float(data[2]);
	printf("EV_lhvd  =  %.2f V\n",EV_lhvd);
	
	EV_lhvr = to_float(data[3]);
	printf("EV_lhvr  =  %.2f V\n",EV_lhvr);
	
	ER_icomp = data[4]*1.263/65536.0;
	printf("ER_icomp  =  %.2f ohms\n",ER_icomp);
	
	Et_lvd_warn = data[5]*0.1;
	printf("Et_lvd_warn  =  %.2f s\n",Et_lvd_warn);
	
	/* Read the 0xE030 EEPROM Registers and convert the results to their proper values */
	ret  =  read_input_registers(mb_param, SUNSAVERMPPT, 0xE030, 6, data);
	
	printf("\nMisc Settings\n");
	
	EV_soc_y2g = to_float(data[0]);
	printf("EV_soc_y2g  =  %.2f V\n",EV_soc_y2g);
	
	EV_soc_g2y = to_float(data[1]);
	printf("EV_soc_g2y  =  %.2f V\n",EV_soc_g2y);
	
	EV_soc_y2r0 = to_float(data[2]);
	printf("EV_soc_y2r0  =  %.2f V\n",EV_soc_y2r0);
	
	EV_soc_r2y = to_float(data[3]);
	printf("EV_soc_r2y  =  %.2f V\n",EV_soc_r2y);
	
	Emodbus_id = data[4];
	printf("Emodbus_id  =  %d\n",Emodbus_id);
	
	Emeter_id = data[5];
	printf("Emeter_id  =  %d\n",Emeter_id);
	
	/* Read the 0xE038 EEPROM Registers and convert the results to their proper values */
	ret  =  read_input_registers(mb_param, SUNSAVERMPPT, 0xE038, 1, data);
	
	printf("\nPPT Settings\n");
	
	Eic_lim = fl_conv_2(data[0]);
	printf("Eic_lim  =  %.2f A\n",Eic_lim);
	
	/* Read the 0xE040 EEPROM Registers and convert the results to their proper values */
	ret  =  read_input_registers(mb_param, SUNSAVERMPPT, 0xE040, 15, data);
	
	printf("\nRead only section of EEPROM\n");
	
	Ehourmeter = shift_32(data[1], data[0]);
	printf("Ehourmeter  =  %d h\n",Ehourmeter);
	
	EAhl_r = shift_float(data[3], data[2]);
	printf("EAhl_r  =  %.2f Ah\n",EAhl_r);
	
	EAhl_t = shift_float(data[5], data[4]);
	printf("EAhl_t  =  %.2f Ah\n",EAhl_t);
	
	EAhc_r = shift_float(data[7], data[6]);
	printf("EAhc_r  =  %.2f Ah\n",EAhc_r);
	
	EAhc_t = shift_float(data[9], data[8]);
	printf("EAhc_t  =  %.2f Ah\n",EAhc_t);
	
	EkWhc = data[10]*0.1;
	printf("EkWhc  =  %.2f kWh\n",EkWhc);
	
	EVb_min = to_float(data[11]);
	printf("EVb_min  =  %.2f V\n",EVb_min);
	
	EVb_max = to_float(data[12]);
	printf("EVb_max  =  %.2f V\n",EVb_max);
	
	EVa_max = to_float(data[13]);
	printf("EVa_max  =  %.2f V\n",EVa_max);
	
	Etmr_eqcalendar = data[14];
	printf("Etmr_eqcalendar  =  %d days\n",Etmr_eqcalendar);

}



int main(int argc, char** argv)
{
	modbus_param_t mb_param;
	int half_duplex  =  0;
	int c;
	char *set_val  =  NULL;

	while( (c =  getopt(argc, argv, "hnds:")) !=  -1) {
		switch(c){
		case 'h': 
			half_duplex  =  1;
			break;
		case 'd': 
			debug  =  1;
			break;
		case 'n': 
			dry  =  1;
			break;
		case 's':
			set_val  =  optarg;
/*
  TODO: http://stackoverflow.com/questions/3939157/c-getopt-multiple-value
  optind--;
  for( ;optind < argc && *argv[optind] ! =  '-'; optind++){
  DoSomething( argv[optind] );         
  }
*/
		default:
			break;
		}
	}


	if(optind  ==  argc){
		printf("need to give serial port on command line\n");
		usage();
		return(1);
	}

	


	/* Setup the serial port parameters */
	modbus_init_rtu(&mb_param, argv[optind], 9600, "none", 8, 2, half_duplex);	


	if(debug > 0){
		modbus_set_debug(&mb_param, TRUE);
	}

	
	/* Open the MODBUS connection */
	if (modbus_connect(&mb_param)  ==  -1) {
		printf("ERROR Connection failed\n");
		return(1);
	}

	if(set_val){
		printf("\nSetting values...\n");
		set_options(&mb_param, set_val);
		printf("\nVerifying values...\n");
	}

	if(dry < 1){
		read_values(&mb_param);
	}
	
	/* Close the MODBUS connection */
	modbus_close(&mb_param);
	
	return(0);
}

