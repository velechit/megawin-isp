#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

#include "serialport.h"

void flush_serial(HANDLE hPort){
    #ifndef WINDOWS_TARGET
      tcflush(serial_port, TCOFLUSH);
    #endif
}

int ConfigureSerialPort(HANDLE hPort, DWORD baud) {
#ifdef WINDOWS_TARGET
  int Status;
  DCB dcb = {0};
 
  dcb.DCBlength = sizeof(dcb);
  Status = GetCommState(hPort, & dcb);
  dcb.BaudRate = baud;
  dcb.Parity = NOPARITY;
  dcb.fBinary = TRUE; // Binary mode; no EOF check 
  dcb.fParity = FALSE; // Enable parity checking 
  dcb.fOutxCtsFlow = FALSE; // No CTS output flow control 
  dcb.fOutxDsrFlow = FALSE; // No DSR output flow control 
  dcb.fDtrControl = DTR_CONTROL_DISABLE; // DTR flow control type 
  dcb.fDsrSensitivity = FALSE; // DSR sensitivity 
  dcb.fTXContinueOnXoff = FALSE; // XOFF continues Tx 
  dcb.fOutX = FALSE; // No XON/XOFF out flow control 
  dcb.fInX = FALSE; // No XON/XOFF in flow control
  dcb.fErrorChar = FALSE; // Disable error replacement 
  dcb.fNull = FALSE; // Disable null stripping 
  dcb.fRtsControl = RTS_CONTROL_DISABLE; // RTS flow control 
  
  dcb.fAbortOnError = FALSE; // Do not abort reads/writes on err
  dcb.ByteSize = 8; // Number of bits/byte, 4-8 
  dcb.StopBits = ONESTOPBIT; // 0,1,2 = 1, 1.5, 2


    COMMTIMEOUTS timeouts = { 0, //interval timeout. 0 = not used
                              0, // read multiplier
                             10, // read constant (milliseconds)
                              0, // Write multiplier
                              0  // Write Constant
                            };
   SetCommTimeouts(hPort, &timeouts);

  if (!SetCommState(hPort, & dcb)) {
    printf("Unable to configure serial port!\n");
    return -1;
  }
  #elif defined(LINUX_TARGET)

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
      return -1;
  }
  #endif
  return 0;
}
void close_serial(HANDLE hPort) {
#ifdef WINDOWS_TARGET
  CloseHandle(hPort);
#elif defined(LINUX_TARGET)
  close(hPort);
#endif
}

void switch_to_baud_9600(HANDLE hPort){
    #ifdef WINDOWS_TARGET
      ConfigureSerialPort(hPort,9600);
    #elif defined(LINUX_TARGET)
        cfsetispeed(&tty, B9600);
        cfsetospeed(&tty, B9600);
       // Save tty settings, also checking for error
        tcsetattr(serial_port, TCSANOW, &tty)
    #endif
}


// TODO: Re-Write this for linux
HANDLE open_serial_port(const char *portname){
#ifdef WINDOWS_TARGET
// open in BAUD 1200
       HANDLE hPort;

       hPort = CreateFileA(portname,                //port name
                      GENERIC_READ | GENERIC_WRITE, //Read/Write
                      0,                            // No Sharing
                      NULL,                         // No Security
                      OPEN_EXISTING,// Open existing port only
                      0,            // Non Overlapped I/O
                      NULL);        // Null for Comm Devices

  

  return hPort;
#elif defined(LINUX_TARGET)
   return open(portname, O_RDWR);
#endif
}

void serial_write(HANDLE hPort,const unsigned char *buf, size_t len){
#ifdef WINDOWS_TARGET
    long i;
    WriteFile(hPort,        // Handle to the Serial port
                   buf,     // Data to be written to the port
                   len,  //No of bytes to write
                   &i, //Bytes written
                   NULL);
#elif defined(LINUX_TARGET)
   write(hPort,buf,len);
#endif
}

long serial_read(HANDLE hPort, char *buf, size_t len){
    long i;
#ifdef WINDOWS_TARGET
    unsigned char ch;
    ReadFile(hPort,(LPVOID)buf,len,&i,NULL);
#elif defined(LINUX_TARGET)
   i=read(hPort,buf,len);
#endif
    return i;
}
