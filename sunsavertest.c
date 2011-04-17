/*
 *  sunsavertest.c
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
along with this program.  If not, see .

*/

/* Compile with: gcc sunsavertest.c -o sunsavertest -lmodbus */

#include <stdio.h>
#include <stdlib.h>
#include <modbus/modbus.h>

#define SUNSAVERMPPT    0x01	/* Default address of the SunSaver MPPT */

int main(int argc, char** argv)
{
	modbus_param_t mb_param;
	int ret;
	float adc_vb_f,adc_va_f,adc_vl_f,adc_ic_f,adc_il_f;
	uint16_t data[10];

    if(argc < 2){
        printf("need to give serial port on command line\n");
        return(1);
    }
	/* Setup the serial port parameters */
	modbus_init_rtu(&mb_param, argv[1], 9600, "none", 8, 2);

	/* Open the MODBUS connection */
	if (modbus_connect(&mb_param) == -1) {
		printf("ERROR Connection failed\n");
		exit(1);
	}

	/* Read the first five RAM Registers (0x04) and convert the results to
		their proper floating point values */
	ret = read_input_registers(&mb_param, SUNSAVERMPPT, 0x0008, 5, data);

	adc_vb_f=data[0]*100.0/32768.0;
	printf("adc_vb_f=%.2f\n",adc_vb_f);

	adc_va_f=data[1]*100.0/32768.0;
	printf("adc_va_f=%.2f\n",adc_va_f);

	adc_vl_f=data[2]*100.0/32768.0;
	printf("adc_vl_f=%.2f\n",adc_vl_f);

	adc_ic_f=data[3]*79.16/32768.0;
	printf("adc_ic_f=%.2f\n",adc_ic_f);

	adc_il_f=data[4]*79.16/32768.0;
	printf("adc_il_f=%.2f\n",adc_il_f);

	/* Close the MODBUS connection */
	modbus_close(&mb_param);

	return(0);
}
