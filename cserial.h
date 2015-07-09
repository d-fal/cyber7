#ifndef CSERIAL_H
#define CSERIAL_H

#define FC_DTRDSR       0x01
#define FC_RTSCTS       0x02
#define FC_XONXOFF      0x04
#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13

#include "windows.h"
#include <iostream>

using namespace std;

class CSerial
{

public:
    CSerial();
    ~CSerial();

    BOOL Open( int nPort = 2, int nBaud = 9600 );
    BOOL Close( void );

    int ReadData(char *, int );
    int SendData( const char *, int );
    int ReadDataWaiting( void );
    bool findAvailablePorts(int portNo);
    BOOL IsOpened( void ){ return( m_bOpened ); }

protected:
    BOOL WriteCommByte( unsigned char );

    HANDLE m_hIDComDev;
    OVERLAPPED m_OverlappedRead, m_OverlappedWrite;
    BOOL m_bOpened;

};

#endif // CSERIAL_H
