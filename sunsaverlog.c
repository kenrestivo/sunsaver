/*
 *  sunsaverlog.c - This program reads all the log registers on a Moringstar SunSaver MPPT and prints the results.
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


/* Compile with: cc sunsaverlog.c -o sunsaverlog -lmodbus */

#include <stdio.h>
#include <stdlib.h>
#include <modbus/modbus.h>
#include <getopt.h>

#define SUNSAVERMPPT    0x01	/* MODBUS Address of the SunSaver MPPT */

int main(int argc, char** argv)
{
	modbus_param_t mb_param;
	int i, ret;
	unsigned int hourmeter, alarm_daily;
	float Vb_min_daily, Vb_max_daily, Ahc_daily, Ahl_daily, Va_max_daily;
	unsigned short array_fault_daily, load_fault_daily;
	unsigned short data[16];
	int c;
	int half_duplex = 0;
	int debug = 0;

	while( (c= getopt(argc, argv, "hd")) != -1) {
		switch(c){
		case 'h': 
			half_duplex = 1;
			break;
		case 'd': 
			debug = 1;
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
	
	if(debug > 0){
		modbus_set_debug(&mb_param, TRUE);
	}

	/* Open the MODBUS connection */
	if (modbus_connect(&mb_param) == -1) {
		printf("ERROR Connection failed\n");
		return(1);
	}
	
	for(i=0x8000; i<0x81FF; i+=0x0010) {
		
		/* Read the log registers and convert the results to their proper values */
		ret = read_input_registers(&mb_param, SUNSAVERMPPT, i, 10, data);
		
		printf("Log Record: 0x%0X\n\n",i);
		
		hourmeter=data[0] + ((data[1] & 0x00FF) << 16);
		printf("hourmeter = %d h\n",hourmeter);
		
		alarm_daily=(data[2] << 8) + (data[1] >> 8);
		printf("alarm_daily = Today's controller self-diagnostic alarms:\n");
		if (alarm_daily == 0) {
			printf("\tNo alarms\n");
		} else {
			if (alarm_daily & 1) printf("\tRTS open\n");
			if ((alarm_daily & (1 << 1)) >> 1) printf("\tRTS shorted\n");
			if ((alarm_daily & (1 << 2)) >> 2) printf("\tRTS disconnected\n");
			if ((alarm_daily & (1 << 3)) >> 3) printf("\tThs open\n");
			if ((alarm_daily & (1 << 4)) >> 4) printf("\tThs shorted\n");
			if ((alarm_daily & (1 << 5)) >> 5) printf("\tSSMPPT hot\n");
			if ((alarm_daily & (1 << 6)) >> 6) printf("\tCurrent limit\n");
			if ((alarm_daily & (1 << 7)) >> 7) printf("\tCurrent offset\n");
			if ((alarm_daily & (1 << 8)) >> 8) printf("\tUndefined\n");
			if ((alarm_daily & (1 << 9)) >> 9) printf("\tUndefined\n");
			if ((alarm_daily & (1 << 10)) >> 10) printf("\tUncalibrated\n");
			if ((alarm_daily & (1 << 11)) >> 11) printf("\tRTS miswire\n");
			if ((alarm_daily & (1 << 12)) >> 12) printf("\tUndefined\n");
			if ((alarm_daily & (1 << 13)) >> 13) printf("\tUndefined\n");
			if ((alarm_daily & (1 << 14)) >> 14) printf("\tMiswire\n");
			if ((alarm_daily & (1 << 15)) >> 15) printf("\tFET open\n");
			if ((alarm_daily & (1 << 16)) >> 16) printf("\tP12\n");
			if ((alarm_daily & (1 << 17)) >> 17) printf("\tHigh Va current limit\n");
			if ((alarm_daily & (1 << 18)) >> 18) printf("\tAlarm 19\n");
			if ((alarm_daily & (1 << 19)) >> 19) printf("\tAlarm 20\n");
			if ((alarm_daily & (1 << 20)) >> 20) printf("\tAlarm 21\n");
			if ((alarm_daily & (1 << 21)) >> 21) printf("\tAlarm 22\n");
			if ((alarm_daily & (1 << 22)) >> 22) printf("\tAlarm 23\n");
			if ((alarm_daily & (1 << 23)) >> 23) printf("\tAlarm 24\n");
		}
		
		Vb_min_daily=data[3]*100.0/32768.0;
		printf("Vb_min_daily = %.2f V\n",Vb_min_daily);
		
		Vb_max_daily=data[4]*100.0/32768.0;
		printf("Vb_max_daily = %.2f V\n",Vb_max_daily);
		
		Ahc_daily=data[5]*0.1;
		printf("Ahc_daily = %.2f Ah\n",Ahc_daily);
		
		Ahl_daily=data[6]*0.1;
		printf("Ahl_daily = %.2f Ah\n",Ahl_daily);
		
		array_fault_daily=data[7];
		printf("array_fault_daily = Today's solar input self-diagnostic faults:\n");
		if (array_fault_daily == 0) {
			printf("\tNo faults\n");
		} else {
			if (array_fault_daily & 1) printf("\tOvercurrent\n");
			if ((array_fault_daily & (1 << 1)) >> 1) printf("\tFETs shorted\n");
			if ((array_fault_daily & (1 << 2)) >> 2) printf("\tSoftware bug\n");
			if ((array_fault_daily & (1 << 3)) >> 3) printf("\tBattery HVD\n");
			if ((array_fault_daily & (1 << 4)) >> 4) printf("\tArray HVD\n");
			if ((array_fault_daily & (1 << 5)) >> 5) printf("\tEEPROM setting edit (reset required)\n");
			if ((array_fault_daily & (1 << 6)) >> 6) printf("\tRTS shorted\n");
			if ((array_fault_daily & (1 << 7)) >> 7) printf("\tRTS was valid, now disconnected\n");
			if ((array_fault_daily & (1 << 8)) >> 8) printf("\tLocal temperature sensor failed\n");
			if ((array_fault_daily & (1 << 9)) >> 9) printf("\tFault 10\n");
			if ((array_fault_daily & (1 << 10)) >> 10) printf("\tFault 11\n");
			if ((array_fault_daily & (1 << 11)) >> 11) printf("\tFault 12\n");
			if ((array_fault_daily & (1 << 12)) >> 12) printf("\tFault 13\n");
			if ((array_fault_daily & (1 << 13)) >> 13) printf("\tFault 14\n");
			if ((array_fault_daily & (1 << 14)) >> 14) printf("\tFault 15\n");
			if ((array_fault_daily & (1 << 15)) >> 15) printf("\tFault 16\n");
		}
		
		load_fault_daily=data[8];
		printf("load_fault_daily = Today's load output self-diagnostic faults:\n");
		if (load_fault_daily == 0) {
			printf("\tNo faults\n");
		} else {
			if (load_fault_daily & 1) printf("\tExternal short circuit\n");
			if ((load_fault_daily & (1 << 1)) >> 1) printf("\tOvercurrent\n");
			if ((load_fault_daily & (1 << 2)) >> 2) printf("\tFETs shorted\n");
			if ((load_fault_daily & (1 << 3)) >> 3) printf("\tSoftware bug\n");
			if ((load_fault_daily & (1 << 4)) >> 4) printf("\tHVD\n");
			if ((load_fault_daily & (1 << 5)) >> 5) printf("\tHeatsink over-temperature\n");
			if ((load_fault_daily & (1 << 6)) >> 6) printf("\tEEPROM setting edit (reset required)\n");
			if ((load_fault_daily & (1 << 7)) >> 7) printf("\tFault 8\n");
		}
		
		Va_max_daily=data[9]*100.0/32768.0;
		printf("Va_max_daily = %.2f V\n\n",Va_max_daily);
	}
	
	/* Close the MODBUS connection */
	modbus_close(&mb_param);
	
	return(0);
}

