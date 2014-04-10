#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <joystick.h>

FILE *f;

void open_hid()
{
	f = fopen("/dev/hidraw0","rb+");
}

unsigned char write_i2c(unsigned char address, unsigned char bytes, unsigned char *buf)
{
	static unsigned char msg_id = 0;
        unsigned char x[13];
        memset(x,0,13);

	x[1] = x[2] = 0xff; // GPIO hold value
	x[3] = 0x00; // Watchdog
	x[4] = msg_id;
	x[5] = 0x08; // write
	x[6] = address;
	x[7] = bytes;
	for(int i = 0; i< bytes; i++)
		x[8+i] = buf[i];

	fwrite(x,13,sizeof(unsigned char),f);
	fflush(f);

	msg_id++;
	return msg_id;
}

unsigned char read_i2c(unsigned char address, unsigned char reg)
{
        static unsigned char msg_id = 0;
        unsigned char x[13];
        memset(x,0,13);

        x[1] = x[2] = 0xff; // GPIO hold value
        x[3] = 0x00; // Watchdog
        x[4] = msg_id;
        x[5] = 0x0c; // write
        x[6] = address;
        x[7] = 1;
	x[8] = reg;
        fwrite(x,13,sizeof(unsigned char),f);
        fflush(f);

	unsigned char c[20];
	c[11] = fread(c,10,sizeof(unsigned char),f);
	for(int i = 0; i < 10; i++)
		printf("%3d ",c[i]);
	printf("\n");

	msg_id++;
        return c[5];
}


void setPWMFreq(float freq) {
	unsigned char prescale;
/* 
	float prescalevel = 25000000 / 4096 / freq -1;
	prescale = floor(prescalevel + 0.5);
 */
	#define PCA9685_MODE1 0x0
	#define PCA9685_PRESCALE 0xFE
	prescale = 134;

	unsigned char oldmode = read_i2c(0x40, PCA9685_MODE1);
	unsigned char newmode = (oldmode&0x7F) | 0x10; // sleep
	unsigned char b[2];

	b[0] =  PCA9685_MODE1;
	b[1] =  newmode;
	write_i2c(0x40, 2, b); 

  	b[0] = PCA9685_PRESCALE; 
	b[1] = prescale; // set the prescaler
        write_i2c(0x40, 2, b); 

	b[0] =  PCA9685_MODE1;
	b[1] =  oldmode | 0xa1; 
        write_i2c(0x40, 2, b); 
}

void setPWM(unsigned char num, unsigned int on, unsigned int off) 
{
	unsigned char b[5];

	b[0] = 0x06+4*num;
	b[1] = on&0xff;
	b[2] = on>>8;
	b[3] = off&0xff;
	b[4] = off>>8;
	write_i2c(0x40, 5, b);
}

int main()
{
	open_hid();

	unsigned char data[3];
	data[0] = 0x00;//ID
	data[1] = 0x00;//CMD 8 write 0c read
	data[2] = 0x00;

	write_i2c(0x40,2,data);
//	read_i2c(0x40,1,data);

	setPWMFreq(1.0f);


	for (unsigned char pwmnum=0; pwmnum < 16; pwmnum++) 
		setPWM(pwmnum, 0,  0)  ;
	setPWM(0, 0,  308); //1500 308 - 1500
        setPWM(1, 0,  308); //1500

	fclose(f);

	return 0;

}
