/// @brief  perform tests on delay functions
///
/// This included measurement of avr-libc delays
/// @return  void
void delay_tests()
{
    printf("System delays\n");

    clock_elapsed_begin();
    clock_elapsed_end("elapsed timer overhead");

    clock_elapsed_begin();
    _delay_us(100);
    clock_elapsed_end("_delay_us(100)");

    clock_elapsed_begin();
    _delay_us(500);
    clock_elapsed_end("_delay_us(500)");

    printf("My delays\n");

    clock_elapsed_begin();
    delayus(100U);
    clock_elapsed_end("delayus(100)");

    clock_elapsed_begin();
    delayus(500U);
    clock_elapsed_end("delayus(500)");

    clock_elapsed_begin();
    delayus(1100);
    clock_elapsed_end("delayus(1100)");

    clock_elapsed_begin();
    delayms(1000);
    clock_elapsed_end("delayms(1100)");
}
