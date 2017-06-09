gpib_hal.c Contains all hardware level GPIB interfacing
gpib.c Contains all low level GPIB BUS IO and parallel poll code for bytes and blocks
gpib_task.c has MAIN GPIB loop called gpib_task tracks command states, talking and listening for devices, secondary commands, etc
            Processes all requests and passes them on to devices
gpib_tests.c gpib user tests
ss80.c SS80 drive emulation state machine
amigo.c AMIGO drive emulation state machine
printer.c PRINTER capture
