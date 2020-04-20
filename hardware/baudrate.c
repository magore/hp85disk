#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/// @brief  ATMEGA1284p UART baud rate caluculation tests
// Usage: baudrate -f F_CPU -b baud
//    F_CPU = External CPU clock
//    Displays actual baud rate and register values
int main(int argc, char *argv[])
{
    int i;
    long u2x, actual,fosc,baud;
    int foscf = 0;
    int baudf = 0;
    double div;
    long ubr_regi;
    char *ptr;

    for(i=1;i< argc;++i)
    {

        ptr = argv[i];
        if(*ptr == '-')
        {
            ++ptr;
            if(*ptr == 'f')
            {
                ++ptr;
                if(*ptr || (ptr = argv[++i]) )
                    fosc = atol(ptr);
                foscf = 1;
            }
            if(*ptr == 'b')
            {
                ++ptr;
                if(*ptr || (ptr = argv[++i]) )
                    baud = atol(ptr);
                baudf = 1;
            }
        }
    }
    if(!foscf || !baudf)
    {
        printf("Usage: %s -f F_CPU -b baud\n", argv[0]);
        printf("   F_CPU = External CPU clock\n");
        printf("   Displays actual baud rate and register values\n");
        exit(1);
    }

// Calculating Baud Rate
// (U2X = 0)
// BAUD = Fosc/(16*(UBRn+1))
// UBRn = Fosc/(16*Baud) -1
// (U2X = 1)
// BAUD = Fosc/(8*(UBRn+1))
// UBRn = Fosc/(8*Baud) -1

///@brief Use 8 prescale as a default
    u2x = 1;
    div = 8;
    ubr_regi = round( ((double)fosc/(div*(double)baud)) - 1.0 );

// For lower baud rates use 16 divider if the UBRR register overflows
// URBRR register is only a 12 bit register!
    if(ubr_regi > 4095)
    {
///@brief Use 16 prescale f we have a low baud rate
        u2x = 0;
        div = 16.0;
        ubr_regi = round( ((double)fosc/(div*(double)baud)) - 1.0 );
    }
//overflow, baud rate was too low - so we clip to maximum allowed
    if(ubr_regi > 4095)
        ubr_regi = 4095;

    actual = ((double)fosc/(div*((double)(ubr_regi+1))));

    printf("fosc = %ld, baud = %ld, actual = %ld, URBRR = %ld, U2X bit = %ld\n",
        fosc, baud, actual, ubr_regi, u2x);

    return(0);
}
