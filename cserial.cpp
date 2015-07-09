#include "cserial.h"


CSerial::CSerial()
{

    memset( &m_OverlappedRead, 0, sizeof( OVERLAPPED ) );
    memset( &m_OverlappedWrite, 0, sizeof( OVERLAPPED ) );
    m_hIDComDev = NULL;
    m_bOpened = FALSE;

}

CSerial::~CSerial()
{

    Close();

}

BOOL CSerial::Open( int nPort, int nBaud )
{
    if( m_bOpened ) return( TRUE );
    char szPort[15];
    char szComParams[50];
    DCB dcb;

    wsprintfA( szPort, "COM%d", nPort );
    m_hIDComDev = CreateFileA( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
    if( m_hIDComDev == NULL ) return( FALSE );

    memset( &m_OverlappedRead, 0, sizeof( OVERLAPPED ) );
    memset( &m_OverlappedWrite, 0, sizeof( OVERLAPPED ) );

    COMMTIMEOUTS CommTimeOuts;
    CommTimeOuts.ReadIntervalTimeout = 0xFFFFFFFF;
    CommTimeOuts.ReadTotalTimeoutMultiplier = 0;
    CommTimeOuts.ReadTotalTimeoutConstant = 0;
    CommTimeOuts.WriteTotalTimeoutMultiplier = 0;
    CommTimeOuts.WriteTotalTimeoutConstant = 5000;
    SetCommTimeouts( m_hIDComDev, &CommTimeOuts );

    wsprintfA( szComParams, "COM%d:%d,n,8,1", nPort, nBaud );

    m_OverlappedRead.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_OverlappedWrite.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    dcb.DCBlength = sizeof( DCB );

    GetCommState( m_hIDComDev, &dcb );
    SetCommMask(m_hIDComDev,EV_BREAK | EV_CTS | EV_DSR | EV_ERR | EV_RING | EV_RLSD | EV_RXCHAR |  EV_TXEMPTY );
    PurgeComm(m_hIDComDev,PURGE_RXABORT | PURGE_RXCLEAR | PURGE_TXABORT | PURGE_TXCLEAR);

    dcb.BaudRate = CBR_115200;
    dcb.ByteSize = 8;
    dcb.XoffLim = 256;
    dcb.XonLim  = 256;
    dcb.EofChar = 0x1A;
    dcb.fDtrControl = DTR_CONTROL_ENABLE;
    dcb.fRtsControl = RTS_CONTROL_DISABLE;
    dcb.fAbortOnError = ABORTDOC;

    unsigned char ucSet;
    ucSet = (unsigned char) ( ( FC_RTSCTS & FC_DTRDSR ) != 0 );
    ucSet = (unsigned char) ( ( FC_RTSCTS & FC_RTSCTS ) != 0 );
    ucSet = (unsigned char) ( ( FC_RTSCTS & FC_XONXOFF ) != 0 );
    if( !SetCommState( m_hIDComDev, &dcb ) ||
        !SetupComm( m_hIDComDev, 1024, 512 ) ||
        m_OverlappedRead.hEvent == NULL ||
        m_OverlappedWrite.hEvent == NULL ){
        DWORD dwError = GetLastError();
        if( m_OverlappedRead.hEvent != NULL ) CloseHandle( m_OverlappedRead.hEvent );
        if( m_OverlappedWrite.hEvent != NULL ) CloseHandle( m_OverlappedWrite.hEvent );
        CloseHandle( m_hIDComDev );
        return( FALSE );
        }

    m_bOpened = TRUE;

    return( m_bOpened );

}

BOOL CSerial::Close( void )
{

    if( !m_bOpened || m_hIDComDev == NULL ) return( TRUE );

    if( m_OverlappedRead.hEvent != NULL ) CloseHandle( m_OverlappedRead.hEvent );
    if( m_OverlappedWrite.hEvent != NULL ) CloseHandle( m_OverlappedWrite.hEvent );
    CloseHandle( m_hIDComDev );
    m_bOpened = FALSE;
    m_hIDComDev = NULL;

    return( TRUE );

}

BOOL CSerial::WriteCommByte( unsigned char ucByte )
{

    BOOL bWriteStat;
    DWORD dwBytesWritten;

    bWriteStat = WriteFile( m_hIDComDev, (LPSTR) &ucByte, 1, &dwBytesWritten, &m_OverlappedWrite );
    if( !bWriteStat && ( GetLastError() == ERROR_IO_PENDING ) ){
        if( WaitForSingleObject( m_OverlappedWrite.hEvent, 1000 ) ) dwBytesWritten = 0;
        else{
            GetOverlappedResult( m_hIDComDev, &m_OverlappedWrite, &dwBytesWritten, FALSE );
            m_OverlappedWrite.Offset += dwBytesWritten;
            }
        }

    return( TRUE );

}

int CSerial::SendData( const char *buffer, int size )
{

    if( !m_bOpened || m_hIDComDev == NULL ) return( 0 );

    DWORD dwBytesWritten = 0;
    int i;
    for( i=0; i<size; i++ ){
        WriteCommByte( buffer[i] );
        dwBytesWritten++;
        }

    return( (int) dwBytesWritten );

}

int CSerial::ReadDataWaiting( void )
{

    if( !m_bOpened || m_hIDComDev == NULL ) return( 0 );

    DWORD dwErrorFlags;
    COMSTAT ComStat;

    ClearCommError( m_hIDComDev, &dwErrorFlags, &ComStat );

    return( (int) ComStat.cbInQue );

}

int CSerial::ReadData(char *buffer, int limit )
{


    if( !m_bOpened || m_hIDComDev == NULL ) return( 0 );

    BOOL bReadStatus;
    DWORD dwBytesRead, dwErrorFlags;
    COMSTAT ComStat;

    ClearCommError( m_hIDComDev, &dwErrorFlags, &ComStat );
    if( !ComStat.cbInQue ) return( 0 );

    dwBytesRead = (DWORD) ComStat.cbInQue;
    if( limit < (int) dwBytesRead ) dwBytesRead = (DWORD) limit;

    bReadStatus = ReadFile( m_hIDComDev, buffer, dwBytesRead, &dwBytesRead, &m_OverlappedRead );
    if( !bReadStatus ){
        if( GetLastError() == ERROR_IO_PENDING ){
            WaitForSingleObject( m_OverlappedRead.hEvent, 1000 );
            return( (int) dwBytesRead );
            }
        return( 0 );
        }

    return( (int) dwBytesRead );

}

bool CSerial::findAvailablePorts(int portNo){
    char szPort[15];
    char szComParams[50];
    DCB dcb;


    wsprintfA( szPort, "COM%d", portNo );
    m_hIDComDev = CreateFileA( szPort, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED, NULL );
    if( m_hIDComDev == INVALID_HANDLE_VALUE) return( FALSE );



    CloseHandle(m_hIDComDev);
    return TRUE;
}
