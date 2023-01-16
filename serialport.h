#ifndef __SERIAL_PORT_H__
#define __SERIAL_PORT_H__

#if defined(__WIN32__)||defined(__WINNT__)
#define WINDOWS_TARGET
#elif defined(__linux__)||defined(linux)
#define LINUX_TARGET
#endif

#if !defined(WINDOWS_TARGET) && !defined(LINUX_TARGET)
#error Unsupported target
#endif

#ifdef WINDOWS_TARGET
    #include <windows.h>
    #define IS_INVALID_HANDLE(x) (x==INVALID_HANDLE_VALUE)
#else
    #include <termios.h>
    #define IS_INVALID_HANDLE(x) (x<0)
    #define HANDLE int
#endif

HANDLE open_serial_port(const char *portname);
int ConfigureSerialPort(HANDLE hPort, DWORD baud);
void serial_write(HANDLE hPort, const unsigned char *buf, size_t len);
long serial_read(HANDLE hPort, char *buf, size_t len);
void switch_to_baud_9600(HANDLE hPort);
void flush_serial(HANDLE hPort);
void close_serial(HANDLE hPort);

#endif /* ! __SERIAL_PORT_H__ */