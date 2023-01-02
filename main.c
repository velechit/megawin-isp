// Megawin ISP programmer v0.1
//
// Commandline utility
//
// interceptty based reverse-engineered ISP programmer
// currently tested only on linux (Fedora 36) with MG82F6D17 only
//     Author: Vishwanath Elechithaya B S (elechi@gmail.com)
//
//
//   TODO:
//      1. Code cleanup
//      2. Support more Baud Rates
//      3. Support for RTS/DTR based reset control
//      4. Support more Megawin controllers??? probably not?
//
//      NOTE: MG82F6D17 has only HWBS programmed, which causes 
//                      ISP code to boot only on power cycling.
//                      To jump to ISP code on reset pin program
//                      HWBS2 fuse (USING the ICP programmer)
//
// C library headers
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <getopt.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

#define CMD_0 0
#define CMD_1 1
#define CMD_2 2
int send_64(int serial_port, uint8_t *buffer, uint16_t *cur_addr, uint16_t buffer_len);

const uint8_t *get_header(uint8_t cmd, uint16_t addr, uint8_t chkcum){
   static uint8_t header[8] = {0x5a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x69};
   memset(header,0x00,8);
   header[0]=0x5a; header[7]=0x69;

   switch (cmd)
   {
   case CMD_0:
      header[2] = 0x83;
      if(addr==1){ // using this to look for baud rate 9600
      // 0xdcfd for 9600 baud
      header[4] = 0xdc;
      header[5] = 0xff;
      } else {
      // 0xdffe for 1200 baud
      header[4] = 0xdf;
      header[5] = 0xfe;
      }
      break;
   case CMD_1:
      header[2]=0x1d;
      break;
   case CMD_2: 
      header[2] = addr &0xFF;
      header[3] = (addr>>8) &0xFF;
      header[4] = chkcum;
      break;
   default:
      break;
   }
   header[1]=cmd;
   return header;
}

void banner(){
  printf("\nMegawin ISP programmer v0.1 CLI\n\n");
  printf("NOTE: This program only supports MG82F6D17 with baud rates of 1200/9600 only\n");
  printf("Author: Vishwanath Elechithaya B S (elechi@gmail.com)\n\n");
}
void usage(const char *argv0)
{       banner();
	fprintf(stderr, "Usage: %s [options]\n", argv0);
	fprintf(stderr, "  -f / --file      <file.bin>\n");
	fprintf(stderr, "  -p / --port      <serial dev>  (ex: /dev/ttyUSB0)\n");
	fprintf(stderr, "  -9 / --9600baud                Use 9600 Baud\n");
	fprintf(stderr, "\n");
}

int main(int argc, char **argv) {
  int baud_9600 = 0;
  char *binfile = "t.bin";
  char *serfile = "/dev/ttyUSB0";
  int quiet =0;


        if (argc < 2)
	{
		usage(argv[0]);
		return -1;
	}
struct option opts[]=
	{
		{ "file",		required_argument,		0,		'f' },
		{ "port",		required_argument,		0,		'p' },
		{ "9600baud",		no_argument,	        	0,		'9' },
		{ "help",		no_argument,		        0,		'h' },
		{ "quiet",		no_argument,		        0,		'q' },
		{ 0 }
	};
int opt;
	while ( (opt=getopt_long(argc, argv, "q-f:p:9-h-", opts, 0)) > 0)
	{
            switch (opt)
		{
			case 'h':
				usage(argv[0]);
				return 0;
			case '9':
				baud_9600=1;
				break;
			case 'q':
				quiet++;
				break;

			case 'f':{
				int tfd=open(optarg, O_RDWR);
				if (tfd<0)
				{
					perror(optarg);
					return -1;
				}
				close(tfd);
                                binfile = strdup(optarg);
				 }
				break;
			case 'p':{
				int tfd=open(optarg, O_RDWR);
				if (tfd<0)
				{
					perror(optarg);
					return -1;
				}
				close(tfd);
                                serfile = strdup(optarg);
				 }
				break;
			case '?':
			default:
				usage(argv[0]);
				return -1;
		}
	}


if(!quiet) banner();





 if(quiet < 2)  printf("Serial Port : %s\n",serfile);
 if(quiet < 2)  printf("Baud Rate: %s\n",baud_9600?"9600":"1200");
 if(quiet < 2)  printf("Program file: %s\n",binfile);

	int fd= open(binfile,O_RDONLY); free(binfile);
	if(fd<0) {printf("Error: Could not open %s\n",binfile); exit(-1); }
	uint8_t buffer[0x4000];
	struct stat sb;
        fstat(fd,&sb);
        uint16_t len = sb.st_size;
	memset(buffer,0xFF,0x4000);
	read(fd,buffer,0x4000);
  	close(fd);



  // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
  int serial_port = open(serfile, O_RDWR); free(serfile);
  if(serial_port<0) {printf("Error: Could not open %s\n",serfile); exit(-1); }

  // Create new termios struct, we call it 'tty' for convention
  struct termios tty;

  // Read in existing settings, and handle any error
  if(tcgetattr(serial_port, &tty) != 0) {
      printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
  tty.c_cflag |= CS8; // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO; // Disable echo
  tty.c_lflag &= ~ECHOE; // Disable erasure
  tty.c_lflag &= ~ECHONL; // Disable new-line echo
  tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed

  tty.c_cc[VTIME] = 0;   // Non blocking
  tty.c_cc[VMIN] = 0;

  // start at Baud 1200
  cfsetispeed(&tty, B1200);
  cfsetospeed(&tty, B1200);

  // Save tty settings, also checking for error
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
      printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      return 1;
  }
  printf("Waiting for controller ISP...\n");

  uint8_t ch = 0x80;
  uint8_t ch1;
  int count=0;
  while(1) {
    write(serial_port,&ch,1); usleep(10000000L/1200);
    if(read(serial_port, &ch1, 1)>0)
        if(ch1==0x69) break;
  }
  usleep(1000);
  tcflush(serial_port, TCOFLUSH);

  ch1=25;
  while(--ch1) write(serial_port,&ch,1);
  ch=0x5a; write(serial_port,&ch,1);
  ch=0x69; write(serial_port,&ch,1);

  while(1) {
    if(read(serial_port, &ch1, 1)>0)
        if(ch1==0x05) break;
  }
  if(quiet < 2) printf("Controller ISP responded...\n");
  if(quiet < 2) printf("Negotiating Baud Rate...\n");

   const uint8_t *header = get_header(CMD_0, baud_9600,0);write(serial_port,header,8); 
   // wait for 500ms

   usleep(500000);
   usleep(200000);

  if(baud_9600){
  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);

  // Save tty settings, also checking for error
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
      printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      return 1;
  }
   }
  if(quiet < 2) printf("Begin Programming...\n");
   header = get_header(CMD_1, 0,0);write(serial_port,header,8);

  while(1) {
    if(read(serial_port, &ch1, 1)>0)
        if(ch1==0x00) break;
  }
  // can change to new baud here
  //

  if(quiet < 2) printf("Progress (# = 64 bytes): ");
  uint16_t cur_addr=0;
  while(send_64(serial_port, buffer,&cur_addr,len)){
    if(quiet < 3) {printf("#");fflush(stdout);}
    while(1) { if(read(serial_port, &ch1, 1)>0) if(ch1==0x00) break; }
  }
  if(quiet < 3) printf("\n");
  if(quiet < 2) printf("Programming complete...\nResetting the controller...\n");
  ch=0xff; write(serial_port,&ch,1); write(serial_port,&ch,1);

  close(serial_port);
  return 0; // success
};


int send_64(int serial_port, uint8_t *buffer, uint16_t *cur_addr, uint16_t buffer_len){
   uint8_t send_buf[64] ;
   int ret_status;
   if(((*cur_addr) + 64) > buffer_len){
      uint8_t chars_to_copy=(buffer_len-(*cur_addr));
      memcpy(send_buf,(buffer+(*cur_addr)),chars_to_copy);
      memset(send_buf+chars_to_copy,0xff,64-chars_to_copy);
      ret_status=0;
   } else {
      memcpy(send_buf,(buffer+(*cur_addr)),64);
      ret_status = 1;
   }

   uint8_t sum = 0;

   for(int i=0;i<64;i++) sum += send_buf[i];

   const uint8_t *header = get_header(CMD_2, (*cur_addr),sum);
   write(serial_port,header,8);
   write(serial_port, send_buf, 64);

   (*cur_addr) +=64;
   return ret_status;
}
