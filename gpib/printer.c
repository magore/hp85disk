/**
 @file gpib/printer.c

 @brief HPGL printer capture code for HP85 disk emulator project for AVR. 

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

*/

#include "user_config.h"

#include "defines.h"

#include "drives.h"

#include "gpib_hal.h"
#include "gpib.h"
#include "gpib_task.h"
#include "printer.h"
#include "posix.h"

///@brief Plotter file data structure used for saving plot data.
PRINTERStateType plot = { 0 };

///@brief Plotter GPIB test vector.
uint8_t plot_str[] = { 0x3f, 0x5f, 0x47, 0x21 };

/// @brief  Open a file to receive plot data using POSIX functions.
///
/// - Uses POSIX FatFs wrappers.
///
/// @see posix.c
/// @see posix.h
/// @return  void

void printer_open(char *name)
{

    char *ptr;

    if(!name)
    {
        time_t seconds;
        tm_t *tc;
        ts_t ts;
        char fname[64];

        clock_gettime(0, (ts_t *) &ts);
        seconds = ts.tv_sec;
        tc = gmtime(&seconds);
        sprintf(fname,"/plot-%02d%s%04d-%02d%02d%02d.plt",
            tc->tm_mday,
            tm_mon_to_ascii(tc->tm_mon),
            tc->tm_year + 1900,
            tc->tm_hour,
            tc->tm_min,
            tc->tm_sec);
        ptr = fname;
    }
    else
    {
        ptr = name;
    }

    if(debuglevel & 32)
        printf("Capturing plot to:%s\n", ptr);

    plot.fp = fopen(ptr,"w");
    if(plot.fp == NULL)
    {
        if(debuglevel & (1+32))
        {
            perror("open failed");
            printf("exiting...\n");
        }
        return;
    }

    plot.buf = calloc(512+1,1);
    if(plot.buf == NULL)
        printer_close();
    plot.size = 512;
}


/// @brief  Initialize plotter structures and state.
///
/// - Only called once after power on by main().
/// @return  void

void printer_init()
{
    plot.error = 0;
    plot.count = 0;
    plot.ind = 0;
    plot.fp = NULL;
    plot.buf = NULL;
}


/// @brief  Close current plot file and reset states.
///
/// - Verify that polt file is open before attempting close.
/// - Uses POSIX FatFs wrappers.
///
/// @see posix.c
/// @see posix.h
/// @return  void
/// FYI: for the HP54645D plots end with: pd;pu;pu;sp0;
/// This gets called
void printer_close()
{
    if( receive_plot_flush() < 0 )
        plot.error = 1;

    if(debuglevel & (1+32))
    {
        if(plot.error)
            printf("ERROR durring write\n");
    }

    if(plot.fp)
    {
        fclose(plot.fp);
        if(debuglevel & 32)
            printf("\nDONE: %08ld\n",plot.count);
    }

    if(plot.buf)
        safefree(plot.buf);
    printer_init();
}


/// @brief Write Plotter data and flush.
///
/// - Uses POSIX FatFs wrappers.
///
/// @see posix.c
/// @see posix.h
/// @see printer_buffer()
/// @return  file handle

int receive_plot_flush()
{
    int ret;
    int fno;

    if(plot.fp == NULL || plot.ind == 0)
        return(0);

    ret  = fwrite(plot.buf, 1, plot.ind , plot.fp);
    if(ret != plot.ind)
    {
        if(debuglevel & (1+32))
        {
            perror("receive_plot_flush");
            printf("write failed: wanted %d, got:%d\n", plot.ind, ret);
        }
        return(-1);
    }

    fno = fileno( plot.fp );
    if(fno < 0)
        return(-1);
    ///@brief sync filesystem after every write 
    syncfs( fno );
    return (ret);
}


/// @brief  Buffer Plotter data and flush when buffer is full
///
/// - Uses POSIX FatFs wrappers.
///
/// @see posix.c
/// @see posix.h
/// @return  void
void printer_buffer( uint16_t val )
{

    uint16_t ch;

    if(debuglevel & (1+32))
    {
        if( ( plot.count & 255L ) == 0)
            printf("%08ld\r",plot.count);
    }

    ch = val & 0xff;
    if(val & (0xff00 & ~REN_FLAG))
    {
        if( receive_plot_flush() )
            plot.error = 1;

        //fprintf(plot.fp,"%s\n", ptr);
        //plot.count += strlen(ptr);
    }
    else
    {
        ch  = val & 0xff;
        plot.buf[plot.ind++] = ch;
        plot.count++;

        if(plot.ind >= plot.size)
        {
            if( receive_plot_flush() < 0 )
                plot.error = 1;
            plot.ind  = 0;
        }
    }
}


/// @brief  GPIB Secondary Command Printer commands.
///
/// @todo  Fully emulated plotter response.
/// @return  0

int PRINTER_COMMANDS(uint8_t ch)
{

    // We could, for example, use secondaries to set file names, etc
    // We don not use them yet
    if(PRINTER_is_MLA(listening))
    {
#if SDEBUG
        if(debuglevel & 32)
            printf("[SC PRINTER Listen: %02XH]\n",  0xff & ch );
#endif
        return(0);
    }

    if(PRINTER_is_MTA(talking))
    {
#if SDEBUG
        if(debuglevel & 32)
            printf("[SC PRINTER Talk: %02XH]\n",  0xff & ch );
#endif
        return(0);
    }
    return(0);
}

int gpib_ascii_request(char *str, uint16_t *status)
{
    int len = strlen(str);
    return(gpib_write_str( (uint8_t *)str,len,status) );
}
    


/// @brief  Instruct Instrument to send Plot data.
/// - Not finished or working yet - barely started work in progress,
/// @return  void
void plot_echo( int gpib_address)
{
    int len;
    long count = 0;
    int ind;
    uint16_t status,ch;
    char *ptr,line[128];

    uint8_t bus,buslast;
    
    printer_close();

    ind = find_type(PRINTER_TYPE);

    if(ind == -1)
    {
        printf("printer not defined\n");
        return;
    }
// DEBUGGING
    gpib_decode_header(stdout);

    //gpib_assert_ifc();
    //printf("sent: ifc\n");
    //GPIB_IO_LOW(REN);
    //GPIB_IO_LOW(ATN);

    buslast = gpib_control_read();
    buslast |= gpib_handshake_read();
    gpib_trace_display(buslast,TRACE_BUS);

    while(GPIB_PIN_TST(NDAC) && GPIB_PIN_TST(NRFD) )
    {
        bus = gpib_control_read();
        bus |= gpib_handshake_read();
        if(bus != buslast)
        {
            gpib_trace_display(buslast,TRACE_BUS);
            buslast = bus;
        }
        if(uart_keyhit(0))
            return;
    }
    printf("ready to write\n");

    gpib_write_byte(DCL | ATN_FLAG);

    gpib_write_byte(0x5f | ATN_FLAG);   // untalk
    printf("sent: untalk\n");
    gpib_write_byte(0x3f | ATN_FLAG);   // unlisten
    printf("sent: unlisten\n");

    gpib_write_byte(0x20 | 8 | ATN_FLAG);   // SCOPE listen
    printf("sent: SCOPE listen\n");
    gpib_write_byte(0x40 | 5 | ATN_FLAG);   // PRINTER talk
    printf("sent: PRINTER talk\n");

    status = 0;
    len = gpib_ascii_request(":HARDcopy:DEVice?",&status);
    printf("sent: %d bytes\n",len);

    gpib_write_byte(0x5f | ATN_FLAG);   // untalk
    gpib_write_byte(0x3f | ATN_FLAG);   // unlisten

    gpib_write_byte(0x40 | 8 | ATN_FLAG);   // SCOPE talk
    gpib_write_byte(0x20 | 5 | ATN_FLAG);   // PRINTER listen

    //gpib_trace_task("plot.txt");
    //return;

    while(1)                                      // Main loop, forever
    {
        if(uart_keyhit(0))
        {
            extern int uart_put(int c);
            extern int get_line (char *buff, int len);

            uart_put('>');
            get_line(line,40);
            ptr = skipspaces(line);
            if ( (len = token(ptr,"exit")) )
            {
                return;
            }
            status = ATN_FLAG;
            if(gpib_write_str(plot_str, sizeof(plot_str), &status) != sizeof(plot_str))
                printf("[write failed]\n");

            len = strlen(ptr);
            ptr[len++] = '\r';
            ptr[len++] = '\n';

            status = 0;
            if(gpib_write_str((uint8_t *) ptr, len, &status) != len)
                printf("[write failed]\n");
        }
        ch = gpib_read_byte(NO_TRACE);

        if(( count & 255L ) == 0)
            printf("%8ld\r",count);
        if(ch & (0xff00 & ~REN_FLAG))
        {
            gpib_decode(ch);
        }
        else
        {
        putchar(ch & 0xff);
        }
        ++count;
    }
    printf("\nLogged %ld\n",count);
}
