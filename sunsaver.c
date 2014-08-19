/*
 *  sunsaver.c - This program reads all the RAM registers on a Moringstar SunSaver MPPT and prints the results.
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


/* Compile with: cc sunsaver.c -o sunsaver -lmodbus */

#include <stdio.h>
#include <stdlib.h>
#include <modbus/modbus.h>
#include <getopt.h>

#define SUNSAVERMPPT    0x01	/* MODBUS Address of the SunSaver MPPT */

int c_to_f(int c)
{
	return (c * 9/5) + 32;

}

int main(int argc, char** argv)
{
	modbus_param_t mb_param;
	int ret;
	float adc_vb_f,adc_va_f,adc_vl_f,adc_ic_f,adc_il_f, Vb_f, Vb_ref, Ahc_r, Ahc_t, kWhc;
	float V_lvd, Ahl_r, Ahl_t, Power_out, Sweep_Vmp, Sweep_Pmax, Sweep_Voc, Vb_min_daily;
	float Vb_max_daily, Ahc_daily, Ahl_daily, vb_min, vb_max;
	short T_hs, T_batt, T_amb, T_rts;
	unsigned short charge_state, load_state, led_state;
	unsigned int hourmeter;
	unsigned short array_fault, load_fault, dip_switch, array_fault_daily, load_fault_daily;
	unsigned int alarm, alarm_daily;
	uint16_t data[50];
	int half_duplex = 0;
	int c;
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
	
	/* Read the RAM Registers */
	ret = read_input_registers(&mb_param, SUNSAVERMPPT, 0x0008, 45, data);
	
	if (ret < 0){
		printf("ERROR Communication Error\n");
		return(1);
	}
    
	/* Close the MODBUS connection */
	modbus_close(&mb_param);
	
	/* Convert the results to their proper values and print them out */
	printf("RAM Registers\n\n");
	
	adc_vb_f=data[0]*100.0/32768.0;
	printf("adc_vb_f = %.2f V\n",adc_vb_f);
	
	adc_va_f=data[1]*100.0/32768.0;
	printf("adc_va_f = %.2f V\n",adc_va_f);
	
	adc_vl_f=data[2]*100.0/32768.0;
	printf("adc_vl_f = %.2f V\n",adc_vl_f);
	
	adc_ic_f=data[3]*79.16/32768.0;
	printf("adc_ic_f = %.2f A\n",adc_ic_f);
	
	adc_il_f=data[4]*79.16/32768.0;
	printf("adc_il_f = %.2f A\n",adc_il_f);
	
	T_hs=data[5];
	printf("T_hs = %d °C (%d °F)\n",T_hs, c_to_f(T_hs));
	
	T_batt=data[6];
	printf("T_batt = %d °C (%d °F)\n", T_batt, c_to_f(T_batt));
	
	T_amb=data[7];
	printf("T_amb = %d °C (%d °F)\n",T_amb, c_to_f(T_amb));
	
	T_rts=data[8];
	printf("T_rts = %d °C (%d °F)\n",T_rts, c_to_f(T_rts));
	
	charge_state=data[9];
	switch (charge_state) {
	case 0:
		printf("charge_state = %d START\n",charge_state);
		break;
	case 1:
		printf("charge_state = %d NIGHT_CHECK\n",charge_state);
		break;
	case 2:
		printf("charge_state = %d DISCONNECT\n",charge_state);
		break;
	case 3:
		printf("charge_state = %d NIGHT\n",charge_state);
		break;
	case 4:
		printf("charge_state = %d FAULT\n",charge_state);
		break;
	case 5:
		printf("charge_state = %d BULK_CHARGE\n",charge_state);
		break;
	case 6:
		printf("charge_state = %d ABSORPTION\n",charge_state);
		break;
	case 7:
		printf("charge_state = %d FLOAT\n",charge_state);
		break;
	case 8:
		printf("charge_state = %d EQUALIZE\n",charge_state);
		break;
	}
	
	array_fault=data[10];
	printf("array_fault = Solar input self-diagnostic faults:\n");
	if (array_fault == 0) {
		printf("\tNo faults\n");
	} else {
		if (array_fault & 1) printf("\tOvercurrent\n");
		if ((array_fault & (1 << 1)) >> 1) printf("\tFETs shorted\n");
		if ((array_fault & (1 << 2)) >> 2) printf("\tSoftware bug\n");
		if ((array_fault & (1 << 3)) >> 3) printf("\tBattery HVD\n");
		if ((array_fault & (1 << 4)) >> 4) printf("\tArray HVD\n");
		if ((array_fault & (1 << 5)) >> 5) printf("\tEEPROM setting edit (reset required)\n");
		if ((array_fault & (1 << 6)) >> 6) printf("\tRTS shorted\n");
		if ((array_fault & (1 << 7)) >> 7) printf("\tRTS was valid, now disconnected\n");
		if ((array_fault & (1 << 8)) >> 8) printf("\tLocal temperature sensor failed\n");
		if ((array_fault & (1 << 9)) >> 9) printf("\tFault 10\n");
		if ((array_fault & (1 << 10)) >> 10) printf("\tFault 11\n");
		if ((array_fault & (1 << 11)) >> 11) printf("\tFault 12\n");
		if ((array_fault & (1 << 12)) >> 12) printf("\tFault 13\n");
		if ((array_fault & (1 << 13)) >> 13) printf("\tFault 14\n");
		if ((array_fault & (1 << 14)) >> 14) printf("\tFault 15\n");
		if ((array_fault & (1 << 15)) >> 15) printf("\tFault 16\n");
	}
	
	Vb_f=data[11]*100.0/32768.0;
	printf("Vb_f = %.2f V\n",Vb_f);
	
	Vb_ref=data[12]*96.667/32768.0;
	printf("Vb_ref = %.2f V\n",Vb_ref);
	
	Ahc_r=((data[13] << 16) + data[14])*0.1;
	printf("Ahc_r = %.2f Ah\n",Ahc_r);
	
	Ahc_t=((data[15] << 16) + data[16])*0.1;
	printf("Ahc_t = %.2f Ah\n",Ahc_t);
	
	kWhc=data[17]*0.1;
	printf("kWhc = %.2f kWh\n",kWhc);
	
	load_state=data[18];
	switch (load_state) {
	case 0:
		printf("load_state = %d START\n",load_state);
		break;
	case 1:
		printf("load_state = %d LOAD_ON\n",load_state);
		break;
	case 2:
		printf("load_state = %d LVD_WARNING\n",load_state);
		break;
	case 3:
		printf("load_state = %d LVD\n",load_state);
		break;
	case 4:
		printf("load_state = %d FAULT\n",load_state);
		break;
	case 5:
		printf("load_state = %d DISCONNECT\n",load_state);
		break;
	}
	
	load_fault=data[19];
	printf("load_fault = Load output self-diagnostic faults:\n");
	if (load_fault == 0) {
		printf("\tNo faults\n");
	} else {
		if (load_fault & 1) printf("\tExternal short circuit\n");
		if ((load_fault & (1 << 1)) >> 1) printf("\tOvercurrent\n");
		if ((load_fault & (1 << 2)) >> 2) printf("\tFETs shorted\n");
		if ((load_fault & (1 << 3)) >> 3) printf("\tSoftware bug\n");
		if ((load_fault & (1 << 4)) >> 4) printf("\tHVD\n");
		if ((load_fault & (1 << 5)) >> 5) printf("\tHeatsink over-temperature\n");
		if ((load_fault & (1 << 6)) >> 6) printf("\tEEPROM setting edit (reset required)\n");
		if ((load_fault & (1 << 7)) >> 7) printf("\tFault 8\n");
	}
	
	V_lvd=data[20]*100.0/32768.0;
	printf("V_lvd = %.2f V\n",V_lvd);
	
	Ahl_r=((data[21] << 16) + data[22])*0.1;
	printf("Ahl_r = %.2f Ah\n",Ahl_r);
	
	Ahl_t=((data[23] << 16) + data[24])*0.1;
	printf("Ahl_t = %.2f Ah\n",Ahl_t);
	
	hourmeter=(data[25] << 16) + data[26];
	printf("hourmeter = %d h\n",hourmeter);
	
	alarm=(data[27] << 16) + data[28];
	printf("alarm = Controller self-diagnostic alarms:\n");
	if (alarm == 0) {
		printf("\tNo alarms\n");
	} else {
		if (alarm & 1) printf("\tRTS open\n");
		if ((alarm & (1 << 1)) >> 1) printf("\tRTS shorted\n");
		if ((alarm & (1 << 2)) >> 2) printf("\tRTS disconnected\n");
		if ((alarm & (1 << 3)) >> 3) printf("\tThs open\n");
		if ((alarm & (1 << 4)) >> 4) printf("\tThs shorted\n");
		if ((alarm & (1 << 5)) >> 5) printf("\tSSMPPT hot\n");
		if ((alarm & (1 << 6)) >> 6) printf("\tCurrent limit\n");
		if ((alarm & (1 << 7)) >> 7) printf("\tCurrent offset\n");
		if ((alarm & (1 << 8)) >> 8) printf("\tUndefined\n");
		if ((alarm & (1 << 9)) >> 9) printf("\tUndefined\n");
		if ((alarm & (1 << 10)) >> 10) printf("\tUncalibrated\n");
		if ((alarm & (1 << 11)) >> 11) printf("\tRTS miswire\n");
		if ((alarm & (1 << 12)) >> 12) printf("\tUndefined\n");
		if ((alarm & (1 << 13)) >> 13) printf("\tUndefined\n");
		if ((alarm & (1 << 14)) >> 14) printf("\tMiswire\n");
		if ((alarm & (1 << 15)) >> 15) printf("\tFET open\n");
		if ((alarm & (1 << 16)) >> 16) printf("\tP12\n");
		if ((alarm & (1 << 17)) >> 17) printf("\tHigh Va current limit\n");
		if ((alarm & (1 << 18)) >> 18) printf("\tAlarm 19\n");
		if ((alarm & (1 << 19)) >> 19) printf("\tAlarm 20\n");
		if ((alarm & (1 << 20)) >> 20) printf("\tAlarm 21\n");
		if ((alarm & (1 << 21)) >> 21) printf("\tAlarm 22\n");
		if ((alarm & (1 << 22)) >> 22) printf("\tAlarm 23\n");
		if ((alarm & (1 << 23)) >> 23) printf("\tAlarm 24\n");
	}
	
	dip_switch=data[29];
	printf("dip_switch = DIP switch settings:\n");
	if (dip_switch & 1) {
		printf("\tSwitch 1 ON - Battery Type: Gel or AGM\n");
	} else {
		printf("\tSwitch 1 OFF - Battery Type: Sealed or Flooded\n");
	}
	if ((dip_switch & (1 << 1)) >> 1) {
		printf("\tSwitch 2 ON - LVD = 11.00 V, LVR = 12.10 V or custom load settings\n");
	} else {
		printf("\tSwitch 2 OFF - LVD = 11.50 V, LVR = 12.60 V\n");
	}
	if ((dip_switch & (1 << 2)) >> 2) {
		printf("\tSwitch 3 ON - Auto-Equalize On\n");
	} else {
		printf("\tSwitch 3 OFF - Auto-Equalize Off\n");
	}
	if ((dip_switch & (1 << 3)) >> 3) {
		printf("\tSwitch 4 ON - MODBUS Protocol\n");
	} else {
		printf("\tSwitch 4 OFF - Meterbus Protocol\n");
	}
	
	led_state=data[30];
	switch (led_state) {
	case 0:
		printf("led_state = %d LED_START\n",led_state);
		break;
	case 1:
		printf("led_state = %d LED_START2\n",led_state);
		break;
	case 2:
		printf("led_state = %d LED_BRANCH\n",led_state);
		break;
	case 3:
		printf("led_state = %d EQUALIZE (FAST GREEN BLINK)\n",led_state);
		break;
	case 4:
		printf("led_state = %d FLOAT (SLOW GREEN BLINK)\n",led_state);
		break;
	case 5:
		printf("led_state = %d ABSORPTION (GREEN BLINK, 1HZ)\n",led_state);
		break;
	case 6:
		printf("led_state = %d GREEN_LED\n",led_state);
		break;
	case 7:
		printf("led_state = %d UNDEFINED\n",led_state);
		break;
	case 8:
		printf("led_state = %d YELLOW_LED\n",led_state);
		break;
	case 9:
		printf("led_state = %d UNDEFINED\n",led_state);
		break;
	case 10:
		printf("led_state = %d BLINK_RED_LED\n",led_state);
		break;
	case 11:
		printf("led_state = %d RED_LED\n",led_state);
		break;
	case 12:
		printf("led_state = %d R-Y-G ERROR\n",led_state);
		break;
	case 13:
		printf("led_state = %d R/Y-G ERROR\n",led_state);
		break;
	case 14:
		printf("led_state = %d R/G-Y ERROR\n",led_state);
		break;
	case 15:
		printf("led_state = %d R-Y ERROR (HTD)\n",led_state);
		break;
	case 16:
		printf("led_state = %d R-G ERROR (HVD)\n",led_state);
		break;
	case 17:
		printf("led_state = %d R/Y-G/Y ERROR\n",led_state);
		break;
	case 18:
		printf("led_state = %d G/Y/R ERROR\n",led_state);
		break;
	case 19:
		printf("led_state = %d G/Y/R x 2\n",led_state);
		break;
	}
	
	Power_out=data[31]*989.5/65536.0;
	printf("Power_out = %.2f W\n",Power_out);
	
	Sweep_Vmp=data[32]*100.0/32768.0;
	printf("Sweep_Vmp = %.2f V\n",Sweep_Vmp);
	
	Sweep_Pmax=data[33]*989.5/65536.0;
	printf("Sweep_Pmax = %.2f W\n",Sweep_Pmax);
	
	Sweep_Voc=data[34]*100.0/32768.0;
	printf("Sweep_Voc = %.2f V\n",Sweep_Voc);
	
	Vb_min_daily=data[35]*100.0/32768.0;
	printf("Vb_min_daily = %.2f V\n",Vb_min_daily);
	
	Vb_max_daily=data[36]*100.0/32768.0;
	printf("Vb_max_daily = %.2f V\n",Vb_max_daily);
	
	Ahc_daily=data[37]*0.1;
	printf("Ahc_daily = %.2f Ah\n",Ahc_daily);
	
	Ahl_daily=data[38]*0.1;
	printf("Ahl_daily = %.2f Ah\n",Ahl_daily);
	
	array_fault_daily=data[39];
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
	
	load_fault_daily=data[40];
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
	
	alarm_daily=(data[41] << 16) + data[42];
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
	
	vb_min=data[43]*100.0/32768.0;
	printf("vb_min = %.2f V\n",vb_min);
	
	vb_max=data[44]*100.0/32768.0;
	printf("vb_max = %.2f V\n",vb_max);
	
	return(0);
}

