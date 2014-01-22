/*
 *  sunsaverEEPROM.c - This program reads all the EEPROM registers on a Moringstar SunSaver MPPT and prints the results.
 *  

Copyright 2010 Tom Rinehart.

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

#define SUNSAVERMPPT    0x01	/* MODBUS Address of the SunSaver MPPT */

int main(int argc, char** argv)
{
	modbus_param_t mb_param;
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
	int half_duplex = 0;
	int c;
	

	while( (c= getopt(argc, argv, "h")) != -1) {
		switch(c){
		case 'h': 
			half_duplex = 1;
			break;
		default:
			break;
		}
	}


    if(optind == argc){
        printf("need to give serial port on command line\n");
        return(1);
    }

	


	/* Setup the serial port parameters */
	modbus_init_rtu(&mb_param, argv[optind], 9600, "none", 8, 2, half_duplex);	
	
	/* Open the MODBUS connection */
	if (modbus_connect(&mb_param) == -1) {
		printf("ERROR Connection failed\n");
		return(1);
	}
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	ret = read_input_registers(&mb_param, SUNSAVERMPPT, 0xE000, 11, data);
	
	printf("EEPROM Registers\n\n");
	
	printf("Charge Settings (bank 1)\n");
	
	EV_reg=data[0]*100.0/32768.0;
	printf("EV_reg = %.2f V\n",EV_reg);
	
	EV_float=data[1]*100.0/32768.0;
	printf("EV_float = %.2f V\n",EV_float);
	
	Et_float=data[2];
	printf("Et_float = %d s\n",Et_float);
	
	Et_floatlb=data[3];
	printf("Et_floatlb = %d s\n",Et_floatlb);
	
	EV_floatlb_trip=data[4]*100.0/32768.0;
	printf("EV_floatlb_trip = %.2f V\n",EV_floatlb_trip);
	
	EV_float_cancel=data[5]*100.0/32768.0;
	printf("EV_float_cancel = %.2f V\n",EV_float_cancel);
	
	Et_float_exit_cum=data[6];
	printf("Et_float_exit_cum = %d s\n",Et_float_exit_cum);
	
	EV_eq=data[7]*100.0/32768.0;
	printf("EV_eq = %.2f V\n",EV_eq);
	
	Et_eqcalendar=data[8];
	printf("Et_eqcalendar = %d days\n",Et_eqcalendar);
	
	Et_eq_above=data[9];
	printf("Et_eq_above = %d s\n",Et_eq_above);
	
	Et_eq_reg=data[10];
	printf("Et_eq_reg = %d s\n",Et_eq_reg);
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	ret = read_input_registers(&mb_param, SUNSAVERMPPT, 0xE00D, 11, data);
	
	printf("\nCharge Settings (bank 2)\n");
	
	EV_reg2=data[0]*100.0/32768.0;
	printf("EV_reg2 = %.2f V\n",EV_reg2);
	
	EV_float2=data[1]*100.0/32768.0;
	printf("EV_float2 = %.2f V\n",EV_float2);
	
	Et_float2=data[2];
	printf("Et_float2 = %d s\n",Et_float2);
	
	Et_floatlb2=data[3];
	printf("Et_floatlb2 = %d s\n",Et_floatlb2);
	
	EV_floatlb_trip2=data[4]*100.0/32768.0;
	printf("EV_floatlb_trip2 = %.2f V\n",EV_floatlb_trip2);
	
	EV_float_cancel2=data[5]*100.0/32768.0;
	printf("EV_float_cancel2 = %.2f V\n",EV_float_cancel2);
	
	Et_float_exit_cum2=data[6];
	printf("Et_float_exit_cum2 = %d s\n",Et_float_exit_cum2);
	
	EV_eq2=data[7]*100.0/32768.0;
	printf("EV_eq2 = %.2f V\n",EV_eq2);
	
	Et_eqcalendar2=data[8];
	printf("Et_eqcalendar2 = %d days\n",Et_eqcalendar2);
	
	Et_eq_above2=data[9];
	printf("Et_eq_above2 = %d s\n",Et_eq_above2);
	
	Et_eq_reg2=data[10];
	printf("Et_eq_reg2 = %d s\n",Et_eq_reg2);
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	ret = read_input_registers(&mb_param, SUNSAVERMPPT, 0xE01A, 6, data);
	
	printf("\nCharge Settings (shared)\n");
	
	EV_tempcomp=data[0]*100.0/65536.0;
	printf("EV_tempcomp = %.2f V\n",EV_tempcomp);
	
	EV_hvd=data[1]*100.0/32768.0;
	printf("EV_hvd = %.2f V\n",EV_hvd);
	
	EV_hvr=data[2]*100.0/32768.0;
	printf("EV_hvr = %.2f V\n",EV_hvr);
	
	Evb_ref_lim=data[3]*100.0/32768.0;
	printf("Evb_ref_lim = %.2f V\n",Evb_ref_lim);
	
	ETb_max=data[4];
	printf("ETb_max = %d °C\n",ETb_max);
	
	ETb_min=data[5];
	printf("ETb_min = %d °C\n",ETb_min);
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	ret = read_input_registers(&mb_param, SUNSAVERMPPT, 0xE022, 6, data);
	
	printf("\nLoad Settings\n");
	
	EV_lvd=data[0]*100.0/32768.0;
	printf("EV_lvd = %.2f V\n",EV_lvd);
	
	EV_lvr=data[1]*100.0/32768.0;
	printf("EV_lvr = %.2f V\n",EV_lvr);
	
	EV_lhvd=data[2]*100.0/32768.0;
	printf("EV_lhvd = %.2f V\n",EV_lhvd);
	
	EV_lhvr=data[3]*100.0/32768.0;
	printf("EV_lhvr = %.2f V\n",EV_lhvr);
	
	ER_icomp=data[4]*1.263/65536.0;
	printf("ER_icomp = %.2f ohms\n",ER_icomp);
	
	Et_lvd_warn=data[5]*0.1;
	printf("Et_lvd_warn = %.2f s\n",Et_lvd_warn);
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	ret = read_input_registers(&mb_param, SUNSAVERMPPT, 0xE030, 6, data);
	
	printf("\nMisc Settings\n");
	
	EV_soc_y2g=data[0]*100.0/32768.0;
	printf("EV_soc_y2g = %.2f V\n",EV_soc_y2g);
	
	EV_soc_g2y=data[1]*100.0/32768.0;
	printf("EV_soc_g2y = %.2f V\n",EV_soc_g2y);
	
	EV_soc_y2r0=data[2]*100.0/32768.0;
	printf("EV_soc_y2r0 = %.2f V\n",EV_soc_y2r0);
	
	EV_soc_r2y=data[3]*100.0/32768.0;
	printf("EV_soc_r2y = %.2f V\n",EV_soc_r2y);
	
	Emodbus_id=data[4];
	printf("Emodbus_id = %d\n",Emodbus_id);
	
	Emeter_id=data[5];
	printf("Emeter_id = %d\n",Emeter_id);
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	ret = read_input_registers(&mb_param, SUNSAVERMPPT, 0xE038, 1, data);
	
	printf("\nPPT Settings\n");
	
	Eic_lim=data[0]*79.16/32768.0;
	printf("Eic_lim = %.2f A\n",Eic_lim);
	
	/* Read the EEPROM Registers and convert the results to their proper values */
	ret = read_input_registers(&mb_param, SUNSAVERMPPT, 0xE040, 15, data);
	
	printf("\nRead only section of EEPROM\n");
	
	Ehourmeter=(data[1] << 16) + data[0];
	printf("Ehourmeter = %d h\n",Ehourmeter);
	
	EAhl_r=((data[3] << 16) + data[2])*0.1;
	printf("EAhl_r = %.2f Ah\n",EAhl_r);
	
	EAhl_t=((data[5] << 16) + data[4])*0.1;
	printf("EAhl_t = %.2f Ah\n",EAhl_t);
	
	EAhc_r=((data[7] << 16) + data[6])*0.1;
	printf("EAhc_r = %.2f Ah\n",EAhc_r);
	
	EAhc_t=((data[9] << 16) + data[8])*0.1;
	printf("EAhc_t = %.2f Ah\n",EAhc_t);
	
	EkWhc=data[10]*0.1;
	printf("EkWhc = %.2f kWh\n",EkWhc);
	
	EVb_min=data[11]*100.0/32768.0;
	printf("EVb_min = %.2f V\n",EVb_min);
	
	EVb_max=data[12]*100.0/32768.0;
	printf("EVb_max = %.2f V\n",EVb_max);
	
	EVa_max=data[13]*100.0/32768.0;
	printf("EVa_max = %.2f V\n",EVa_max);
	
	Etmr_eqcalendar=data[14];
	printf("Etmr_eqcalendar = %d days\n",Etmr_eqcalendar);
	
	/* Close the MODBUS connection */
	modbus_close(&mb_param);
	
	return(0);
}

