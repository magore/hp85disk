
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{

#if 0
#define PINA _SFR_IO8(0x00)
#define DDRA _SFR_IO8(0x01)
#define PORTA _SFR_IO8(0x02)
#endif

    int p,pin;
    for(p='A';p<'H';++p)
    {
        for(pin=0;pin<8;++pin)
        {
            printf("#define GPIO_%c%d %d\n", p,pin, (p-'A')*8 + pin );
        }
        printf("\n");
    }
    printf("\n");

}
