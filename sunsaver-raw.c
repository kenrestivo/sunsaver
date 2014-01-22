/*

  just give me the raw block of data (kr 9/12)

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
#include <unistd.h>
#include <time.h>

#define SUNSAVERMPPT    0x01	/* MODBUS Address of the SunSaver MPPT */
#define SS_DATALEN 50

int main(int argc, char** argv)
{
	modbus_param_t mb_param;
	int ret;
	uint16_t data[SS_DATALEN];
	

    if(argc < 2){
        fprintf(stderr,"need to give serial port on command line\n");
        return(1);
    }

	/* Setup the serial port parameters */
	modbus_init_rtu(&mb_param, argv[1], 9600, "none", 8, 2);	
	
	/* Open the MODBUS connection */
	if (modbus_connect(&mb_param) == -1) {
		fprintf(stderr,"ERROR Connection failed\n");
		return(1);
	}
	
	/* Read the RAM Registers */
	ret = read_input_registers(&mb_param, SUNSAVERMPPT, 0x0008, 45, data);
	
	if (ret < 0){
		fprintf(stderr,"ERROR Communication Error\n");
		return(1);
	}
    
	/* Close the MODBUS connection */
	modbus_close(&mb_param);



	// a simple header
	uint16_t	header = 0xf1c1;

	write(1, &header, sizeof(header));

	// just output the block of data
	write(1, data, SS_DATALEN * sizeof(uint16_t));
	

	// append WHEN
	time_t nowish = time(0);

	write(1, &nowish, sizeof(nowish));
	
	return(0);
}

