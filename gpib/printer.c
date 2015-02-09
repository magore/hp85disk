/**
 @file gpib/printer.c

 @brief HPGL printer capture code for HP85 disk emulator project for AVR8. 

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#include "hardware/hardware.h"

#include "defines.h"
#include "gpib_hal.h"
#include "gpib.h"
#include "gpib_task.h"
#include "printer.h"

///@brief Plotter file data structure definition used for saving plot data.
struct _plot
{
    uint32_t count;                               // total bytes
    int16_t ind;                                  // buffer cache index
    uint8_t error;                                // error status
    FILE *fp;
    char *buf;
};
///@brief Plotter file data structure used for saving plot data.
struct _plot plot;

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
        mysprintf(fname,"/plot-%02d%s%04d-%02d%02d%02d.plt",
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

    myprintf("Capturing plot to:%s\n", ptr);

    plot.fp = fopen(ptr,"w");
    if(plot.fp == NULL)
    {
        perror("open failed");
        myprintf("exiting...\n");
        return;
    }

    plot.buf = calloc(256+1,1);
    if(plot.buf == NULL)
    {
        printer_close();
    }
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

void printer_close()
{
    if( receive_plot_flush() )
        plot.error = 1;

    if(plot.error)
        myprintf("ERROR durring write\n");

    if(plot.fp)
    {
        fclose(plot.fp);
        myprintf("\nDONE: %08ld\n",plot.count);
    }

    plot.error = 0;
    plot.count = 0;
    plot.ind = 0;
    plot.fp = NULL;

    if(plot.buf)
        safefree(plot.buf);
    plot.buf = NULL;
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
    int fileno;

    if(plot.fp == NULL || plot.ind == 0)
        return(0);

    ret  = fwrite(plot.buf, 1, plot.ind , plot.fp);
    if(ret != plot.ind)
    {
        perror("receive_plot_flush");
        myprintf("write failed: wanted %d, got:%d\n", plot.ind, ret);
        return(-1);
    }

    fileno = stream_to_fileno( plot.fp );
    if(ret < 0)
        return(ret);
    ret = syncfs( fileno );
    return ( ret );
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

    if( ( plot.count & 255L ) == 0)
        myprintf("%08ld\r",plot.count);

    ch = val & 0xff;
    if(val & (0xff00 & ~REN_FLAG))
    {
        char str[40];
        if( receive_plot_flush() )
            plot.error = 1;
        gpib_decode_str(ch, str);
        puts(str);
        fprintf(plot.fp,"%s\n", str);
        plot.count += strlen(str);
    }
    else
    {
        ch  = val & 0xff;
        plot.buf[plot.ind++] = ch;
        plot.count++;

        if(plot.ind >= 256)
        {
            if( receive_plot_flush() )
                plot.error = 1;
            plot.ind  = 0;
        }
    }
}


/// @brief  Manually receive plot data - user called function.
///
/// - This function would be manually invoked prior to invoking a 
/// Send Plot command on a GPIB device.
/// - Exit on user key press.
///
/// @param[in] name: file name to save plot data in.
///
/// @return  void

void receive_plot( char *name )
{
    uint16_t ch;

    gpib_bus_init(1);
    gpib_state_init();

    printer_close();

    name = skipspaces(name);
    if(!*name)
    {
        myprintf("receive_plot: expected file name\m");
        return;
    }

    printer_open(name);

    while(1)                                      // Main loop, forever
    {
        if(uart_keyhit(0))
            break;

        ch = gpib_read_byte();

        if(( plot.count & 255L ) == 0)
            myprintf("%08ld\r",plot.count);

        if(ch & (0xff00 & ~REN_FLAG))
        {
            char str[40];
            if( receive_plot_flush() )
                plot.error = 1;
            gpib_decode_str(ch, str);
            puts(str);
            fprintf(plot.fp,"%s\n", str);
        }
        else
        {
            ch &= 0xff;
            plot.buf[plot.ind++] = ch;

            if(ch == '\n')
                myprintf("\n%ld EOL\n", plot.count);
            if(ch == '\r')
                myprintf("\n%ld CR\n", plot.count);

            if(plot.ind >= 256)
            {
                if( receive_plot_flush() )
                    plot.error = 1;
                plot.ind  = 0;
            }
        }
        plot.count++;
    }
    if( receive_plot_flush() )
        plot.error = 1;
    printer_close();
}


/// @brief  GPIB Secondary rCcommand Printer commands.
///
/// @todo  Fully emulated plotter response.
/// @return  0

int PRINTER_COMMANDS(uint8_t ch)
{

    if(listening == PRINTER_MLA)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[SC PRINTER Listen: %02x]\n",  0xff & ch );
#endif
        return(0);
    }

    if(listening == PRINTER_MTA)
    {
#if SDEBUG > 1
        if(debuglevel > 1)
            myprintf("[SC PRINTER Talk: %02x]\n",  0xff & ch );
#endif
        return(0);
    }
    return(0);
}


/// @brief  Instruct Intrument to send Plot data.
///
/// - Not finished or working yet - barely started work in progress,
/// @return  void

void plot_echo( int echo )
{
    int ch;
    long count;
    char *ptr;
    int len;
    uint16_t status;
    char Line[40+2];

    gpib_bus_init(1);
    gpib_state_init();

    printer_close();

    gpib_decode_header();

    if(echo)
        myprintf("echo: on\n");
    else
        myprintf("echo: off\n");

    count = 0;
    while(1)                                      // Main loop, forever
    {
        if(uart_keyhit(0))
        {
#if 1
            extern void uart_put(uint8_t c);
            extern void get_line (char *buff, int len);

            uart_put('>');
            get_line(Line,40);
            ptr = skipspaces(Line);
            if ( (len = token(ptr,"exit")) )
            {
                return;
            }
            status = ATN_FLAG;
            if(gpib_write_str(plot_str, sizeof(plot_str), &status) != sizeof(plot_str))
                myprintf("[write failed]\n");

            len = strlen(ptr);
            ptr[len++] = '\r';
            ptr[len++] = '\n';

            status = 0;
            if(gpib_write_str((uint8_t *) ptr, len, &status) != len)
                myprintf("[write failed]\n");
#else
            return;
#endif
        }
        ch = gpib_read_byte();

        if(( count & 255L ) == 0)
            myprintf("%08ld\r",count);
        if(echo)
        {
            if(ch & (0xff00 & ~REN_FLAG))
            {
                char str[40];
                gpib_decode_str(ch, str);
                puts(str);
            }
            else
            {
                putchar(ch & 0xff);
            }
        }
        ++count;
    }
    myprintf("\nLogged %ld\n",count);
}
