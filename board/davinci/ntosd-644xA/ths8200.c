
#include <common.h>

#define THS8200_DRIVER "ths8200"
#define THS8200_I2C_ADDR 0x20

#define	__u8	unsigned char

#define davinci_i2c_read(len, data, chip) i2c_read(chip, 0, 0, data, len)
#define davinci_i2c_write(len, data, chip) i2c_write(chip, 0, 0, data, len)

#define uchar unsigned char
#define uint unsigned int

/*
 * Read/Write interface:
 *   chip:    I2C chip address, range 0..127
 *   addr:    Memory (register) address within the chip
 *   alen:    Number of bytes to use for addr (typically 1, 2 for larger
 *              memories, 0 for register type devices with only one
 *              register)
 *   buffer:  Where to read/write the data
 *   len:     How many bytes to read/write
 *
 *   Returns: 0 on success, not 0 on failure
 */
int i2c_read(uchar chip, uint addr, int alen, uchar *buffer, int len);
int i2c_write(uchar chip, uint addr, int alen, uchar *buffer, int len);

static int write (int i2cfd, char * buf, int len)
{
	if (len > 2)
	{
		printf("ERROR: trying to write more than one register is not supported\n");
		return 2;
	} 

	return davinci_i2c_write(len, buf, THS8200_I2C_ADDR);
}

int ths8200_is_present(void)
{
	__u8 data;
	int err;

	data = 0x02;

	err = davinci_i2c_write(1, &data, THS8200_I2C_ADDR);
	if ( err != 0)
	{
	     return 0;
	}
  
	err = davinci_i2c_read(1, &data,  THS8200_I2C_ADDR);
	if ( err != 0)
	{
	     return 0;
	}

	if (data != 4)  // Value read by Neal
	{
	     return 0;
	}
  
	// Everthing check out, return 1 (TRUE)
	return 1;
}

int ths8200_set_1080I(void)
{
	int     i2cfd = 0;
	int     i;  // loop counter
	__u8    buf[256];

	// place ths8200 in reset state
	buf[0] = 0x03;
	buf[1] = 0x10;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x03\n");

	//JG Added: take ths8200 out of reset and in normal operation mode
	buf[0] = 0x03;
	buf[1] = 0x11;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x03\n");

	//JG Added:
	for (i=4; i < 0x19; i++)
	{
		buf[0] = i;
		buf[1] = 0x00;
		if(write(i2cfd, buf, 2) != 0)
			printf("Write Error Address = 0x%x\n", i);
	}

	buf[0] = 0x19;
	buf[1] = 0x03;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x%x\n", buf[0]);

	// Turn off THS8200 Test Modes
	buf[0] = 0x1a;
	buf[1] = 0x00;
	if((i = write(i2cfd, buf, 2)) != 0)
		printf("Write Error Address = 0x1a %d\n",i);
    
	buf[0] = 0x1b;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x1b\n");

	// Turn CSM Off
	buf[0] = 0x4a;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x4a\n");

	// Set YCx20 External Sync
	buf[0] = 0x82;
	buf[1] = 0x1f;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x82\n");
    
	buf[0] = 0x1c;
	buf[1] = 0x03;  
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x1c\n");
    

	buf[0] = 0x1d;
	buf[1] = 0xff;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x1d\n");

	buf[0] = 0x1e;
	buf[1] = 0x49;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x1e\n");

	buf[0] = 0x1f;
	buf[1] = 0xb6;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x1f\n");

	buf[0] = 0x20;
	buf[1] = 0xFF;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x20\n");

	buf[0] = 0x21;
	buf[1] = 0xFF;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x21\n");

	buf[0] = 0x22;
	buf[1] = 0xFF;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x22\n");

	buf[0] = 0x23;
	buf[1] = 0x13;  
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x23\n");

	buf[0] = 0x24;
	buf[1] = 0x15;  
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x24\n");

	// Negative Hsync width (half of total width)
	buf[0] = 0x25;
	buf[1] = 0x2C;  //0x50
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x25\n");
    
	// End of Active Video to start of negative sync
	buf[0] = 0x26;
	buf[1] = 0x58;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x26\n");

	// positive hsync width (half of total width)
	buf[0] = 0x27;
	buf[1] = 0x2C;  //0x50
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x27\n");
	// LSBs of sync to broad pulse [0:7]
	buf[0] = 0x28;
	buf[1] = 0x84;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x28\n");

	buf[0] = 0x29;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x28\n");

	// LSBs of sync to active video [0:7]
	buf[0] = 0x2a;
	buf[1] = 0xC0;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x2a\n");

	// MSB bit of sync to active video width[6]/sync to broad pulse [7]
	buf[0] = 0x2b;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x2b\n");
	// Broad pulse duration for SDTV (NA)
	buf[0] = 0x2c;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x2c\n");

	// End of active video to sync LSBs [7:0]
	buf[0] = 0x2f;
	buf[1] = 0x58;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x2f\n");
	// End of active video to sync MSBs [2:0]
	buf[0] = 0x30;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x30\n");

	buf[0] = 0x32;
	buf[1] = 0x58;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x34\n");
      
	buf[0] = 0x33;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x34\n");      

	buf[0] = 0x34;
	buf[1] = 0x08;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x34\n");

	buf[0] = 0x35;
	buf[1] = 0x98;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x35\n");

	buf[0] = 0x36;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x36\n");

	buf[0] = 0x37;
	buf[1] = 0x01;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x37\n");

	buf[0] = 0x38;
	buf[1] = 0x81;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x38\n");

	buf[0] = 0x39;
	buf[1] = 0x42;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x39\n");

	buf[0] = 0x3a;
	buf[1] = 0x65;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3a\n");

	buf[0] = 0x3b;
	buf[1] = 0x33;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");

	buf[0] = 0x70;
	buf[1] = 0x58;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");
	//JG: Added
	buf[0] = 0x71;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");
	//JG: Added
	buf[0] = 0x72;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");
	//JG: Added
	buf[0] = 0x73;
	buf[1] = 0x05;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");

	//JG: Added
	buf[0] = 0x74;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");

	//JG: Added
	buf[0] = 0x75;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");

	//JG: Added
	buf[0] = 0x76;
	buf[1] = 0x05;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");
	//JG: Added
	buf[0] = 0x77;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");
	//JG: Added
	buf[0] = 0x78;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");

	buf[0] = 0x79;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x79\n");

	buf[0] = 0x7a;
	buf[1] = 0x44;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x7a\n");

	buf[0] = 0x7b;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x7b\n");

	buf[0] = 0x7c;
	buf[1] = 0x01;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x7c\n");

	buf[0] = 0x03;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x03\n");

	buf[0] = 0x03;
	buf[1] = 0x01;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x03\n");

	return 0;
}

int ths8200_set_720P(void)
{
	int     i2cfd = 0;
	int     i;  // loop counter
	__u8    buf[256];

	// place ths8200 in reset state
	buf[0] = 0x03;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x03\n");

	//JG Added: take ths8200 out of reset and in normal operation mode
	buf[0] = 0x03;
	buf[1] = 0x01;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x03\n");

	//JG Added:
	for (i=4; i < 0x19; i++)
	{
		buf[0] = i;
		buf[1] = 0x00;
		if(write(i2cfd, buf, 2) != 0)
			printf("Write Error Address = 0x%x\n", i);
	}

	buf[0] = 0x19;
	buf[1] = 0x03;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x%x\n", buf[0]);

	// Set YCx20 External Sync
	buf[0] = 0x82;
	buf[1] = 0x1b;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x82\n");
    
	buf[0] = 0x1c;
	buf[1] = 0x03;  
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x1c\n");
    

	buf[0] = 0x1d;
	buf[1] = 0xff;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x1d\n");

	buf[0] = 0x1e;
	buf[1] = 0x49;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x1e\n");

	buf[0] = 0x1f;
	buf[1] = 0xb6;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x1f\n");

	buf[0] = 0x20;
	buf[1] = 0xFF;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x20\n");

	buf[0] = 0x21;
	buf[1] = 0xFF;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x21\n");

	buf[0] = 0x22;
	buf[1] = 0xFF;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x22\n");

	buf[0] = 0x23;
	buf[1] = 0x13;  
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x23\n");

	buf[0] = 0x24;
	buf[1] = 0x15;  
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x24\n");

	// Negative Hsync width (half of total width)
	buf[0] = 0x25;
	buf[1] = 0x28;  //0x50
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x25\n");
    
	// End of Active Video to start of negative sync
	buf[0] = 0x26;
	buf[1] = 0x46;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x26\n");

	// positive hsync width (half of total width)
	buf[0] = 0x27;
	buf[1] = 0x28;  //0x50
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x27\n");
	// LSBs of sync to broad pulse [0:7]
	buf[0] = 0x28;
	buf[1] = 0x2c;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x28\n");

	// LSBs of sync to active video [0:7]
	buf[0] = 0x2a;
	buf[1] = 0x2c;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x2a\n");

	// MSB bit of sync to active video width[6]/sync to broad pulse [7]
	buf[0] = 0x2b;
	buf[1] = 0xc0;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x2b\n");
	// Broad pulse duration for SDTV (NA)
	buf[0] = 0x2c;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x2c\n");

	// End of active video to sync LSBs [7:0]
	buf[0] = 0x2f;
	buf[1] = 0x46;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x2f\n");
	// End of active video to sync MSBs [2:0]
	buf[0] = 0x30;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x30\n");

	buf[0] = 0x34;
	buf[1] = 0x06;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x34\n");

	buf[0] = 0x35;
	buf[1] = 0x72;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x35\n");

	buf[0] = 0x36;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x36\n");

	buf[0] = 0x37;
	buf[1] = 0x01;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x37\n");

	buf[0] = 0x38;
	buf[1] = 0x82;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x38\n");

	buf[0] = 0x39;
	buf[1] = 0x27;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x39\n");

	buf[0] = 0x3a;
	buf[1] = 0xee;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3a\n");

	buf[0] = 0x3b;
	buf[1] = 0xff;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x3b\n");

	buf[0] = 0x79;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x79\n");

	buf[0] = 0x7a;
	buf[1] = 0x60;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x7a\n");

	buf[0] = 0x7b;
	buf[1] = 0x08;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x7b\n");

	buf[0] = 0x7c;
	buf[1] = 0x06;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x7c\n");

	buf[0] = 0x03;
	buf[1] = 0x00;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x03\n");

	buf[0] = 0x03;
	buf[1] = 0x01;
	if(write(i2cfd, buf, 2) != 0)
		printf("Write Error Address = 0x03\n");

	return 0;
}

