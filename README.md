\section README
 @par HP85 Disk Emulator Copyright &copy; 2014 Mike Gore
 See [COPYRIGHT.md](@ref COPYRIGHT.md) for a full copywrite notice for the project

 This project emulates three GPIB devices
  * SS80 HP9134L disk at 700 for my HP85A (with 85A roms)
  * Amigo 9121D disk  at 710 for my HP85B (with 85B roms)
  * Printer capture   at 720 for my HP54645D scope

___

# Credits

<b>I really owe the very existence of this project to the original work done by Anders Gustafsson and his great "HP Disk Emulator" </b>
 * You can visit his project at this site:
   * <http://www.dalton.ax/hpdisk>
   * <http://www.elektor-labs.com/project/hpdisk-an-sd-based-disk-emulator-for-gpib-instruments-and-computers.13693.html>

 <b>Anders Gustafsson was extremely helpful in providing me his current 
 code and details of his project - which I am very thankful for.</b>

 As mainly a personal excercise in fully understanding the code I 
 ended up rewriting much of the hpdisk project. I did this one part at a 
 time as I learned the protocols and specifications - NOT because of any 
 problems with his original work. 

 I have retained Anders HP HP9121D and HP9134L data structures found
 in gpib/defines without modification. Although mostly rewritten I have 
 maintained the concept of state machines for GPIB read and write functions 
 as well as for SS80 execute state tracking. 

___
# Abbreviations
Within this project I have attempted to provide detailed referces 
to to manuals, listed below.  I have included short quotes and 
section and page# reference to these works.
 * <b>SS80</b>
 * <b>CS80</b>
 * <b>A or Amigo</b>
 * <b>HP-IP</b>
 * <b>HP-IP Tutorial</b>

## Documentation References and related sources of information
 * Web Resources
   * <http://www.hp9845.net>
   * <http://www.hpmuseum.net>
   * <http://www.hpmusuem.org>
   * <http://bitsavers.trailing-edge.com>
   * <http://en.wikipedia.org/wiki/IEEE-488>

## SS80 References: ("SS80" is the short form used in the project)
   * "Subset 80 from Fixed and flexible disc drives"
   * Printed November, 1985
   * HP Part# 5958-4129 

## CS80 References: ("CS80" is the short form used in the project)
   * "CS/80 Instruction Set Programming Manual"
   * Printed: APR 1983
   * HP Part# 5955-3442

## Amigo References: ("A" or "Amigo" is the short form used in the project)
   * "Appendix A of 9895A Flexible Disc Memory Service Manual"
   * HP Part# 09895-90030

## HP-IB
   * ("HP-IB" is the short form used in the project)
   * "Condensed Description of the Hewlett Packard Interface Bus"
   * Printed March 1975
   * HP Part# 59401-90030

## Tutorial Description of The Hewllet Packard Interface Bus
   * ("HP-IB Tutorial" is the short form used in the project)
   * <http://www.hpmemory.org/an/pdf/hp-ib_tutorial_1980.pdf>
   * Printed January 1983
   * <http://www.ko4bb.com/Manuals/HP_Agilent/HPIB_tutorial_HP.pdf>
   * Printed 1987

## GPIB / IEEE 488 Tutorial by Ian Poole
    * <http://www.radio-electronics.com/info/t_and_m/gpib/ieee488-basics-tutorial.php>

## HP 9133/9134 D/H/L References
   * "HP 9133/9134 D/H/L Service Manual"
   * HP Part# 5957-6560
   * Printed: APRIL 1985, Edition 2

## LIF Filesystem Format
   * <http://www.hp9845.net/9845/projects/hpdir/#lif_filesystem>

## Useful Utilities
   * HP Drive  (HP Drive Emulators for Windows Platform)
     * <http://www.hp9845.net/9845/projects/hpdrive/>
   * HP Dir    (HP Drive - Disk Image Manipulation)
     * <http://www.hp9845.net/9845/projects/hpdir/>

___

## GPIB Connector pinout by Anders Gustafsson in his hpdisk project
  * http://www.dalton.ax/hpdisk/


      Pin Name 	Signal Description       Pin    Name 	Signal Description 
      1   DIO1 	Data Input/Output Bit 1  13 	DIO5 	Data Input/Output Bit 5 
      2   DIO2 	Data Input/Output Bit 2  14 	DIO6 	Data Input/Output Bit 6 
      3   DIO3 	Data Input/Output Bit 3  15 	DIO7 	Data Input/Output Bit 7 
      4   DIO4 	Data Input/Output Bit 4  16 	DIO8 	Data Input/Output Bit 8 
      5   EIO 	End-Or-Identify          17 	REN 	Remote Enable 
      6   DAV 	Data Valid               18 	Shield 	Ground (DAV) 
      7   NRFD 	Not Ready For Data       19 	Shield 	Ground (NRFD) 
      8   NDAC 	Not Data Accepted        20 	Shield 	Ground (NDAC) 
      9   IFC 	Interface Clear          21 	Shield 	Ground (IFC) 
      10  SRQ 	Service Request          22 	Shield 	Ground (SRQ) 
      11  ATN 	Attention                23 	Shield 	Ground (ATN) 
      12  Shield  Chassis Ground           24     Single GND Single Ground

___

## AVR ATMEGA1284P pin assignments for HP85 Disk
  * GPIB:  Each GPIB pin (8 data and 8 control lines ) attach to CPU with a 120 ohm current limit resistor .
    * Each GPIB pin (8 data and 8 control lines ) have a 10K pull-up resistor to VCC.
  * ISP header: MOSI,MISO,SCK,/Reset connects directly to ISP header
  * Micro SD Interface: MOSI,MISO,SCK attach to CPU function via a 1k series resistor.
    * Micro SD interface has level shifters and internal 5V to 3.3V regulator
  * RS232 TTL: connect to FTDI232 USB  board which also provides 5V VCC power to all circuits..
  * I2C: SCL,SDA connect to optional DS1307 RTC board with each line having a 2k2 pull-up


                         ATMEGA1284P (and ATMEGA644P) 
                         +---\/---+ 
       5 EOI INT0  PB0  1|        |40  PA0      D1  1 
       6 DAV INT1  PB1  2|        |39  PA1      D2  2 
         PP  INT2  PB2  3|        |38  PA2      D3  3 
      SD /CS  PWM  PB3  4|        |37  PA3      D4  4 
         NC   PWM  PB4  5|        |36  PA4      D5 13 
      SD     MOSI  PB5  6|        |35  PA5      D6 14 
      SD     MISO  PB6  7|        |34  PA6      D7 15 
      SD      SCK  PB7  8|        |33  PA7      D8 16 
      10K pullup  /RST  9|        |32  AREF     0.1uf 
         +5        VCC 10|        |31  GND      GND   
         GND       GND 11|        |30  AVCC     +5    
      20MHZ      XTAL2 12|        |29  PC7      NC    
      20MHZ      XTAL1 13|        |28  PC6      NC    
         RX   RX0  PD0 14|        |27  PC5  TDI JTAG 
         TX   TX0  PD1 15|        |26  PC4  TDO JTAG 
       7 NRFD RX1  PD2 16|        |25  PC3  TMS JTAG 
       8 NDAC TX1  PD3 17|        |24  PC2  TCK JTAG 
       9 IFC  PWM  PD4 18|        |23  PC1  SDA I2C   
      10 SRQ  PWM  PD5 19|        |22  PC0  SCL I2C  
      11 ATN  PWM  PD6 20|        |21  PD7  PWM REN 17 
                         +--------+ 

___ 

## Parallel Poll Response circuit
  * Uses: Three chips 74HC05, 74HC32, 74HC595
  * Parallel Poll Response must be less then 2 Microseconds therefore we use hardware to do it!


       ATMEGA	           HC595            HC05 
                         +---\/---+          +-\/-+  
       3 PB3 -------- 12 |RCLK  Q0| 15 -x- 1 |    | 2 --- GPIB D8 
       6 MOSI ------- 14 |SER   Q1| 1  -x- 3 |    | 4 --- GPIB D7 
       8 SCK -------- 11 |SRCLK Q2| 2  -x- 5 |    | 6 --- GPIB D6 
       9 IFC -------- 10 |SRCLR Q3| 3  -x- 9 |    | 8 --- GPIB D5 
             HC32        |        |     |    |    | 7 GND 
             +-\/-+      |        |     |    |    |14 VCC 
        EOI 2|    |      |        |     |    +----+ 
        ATN 1|    |      |        |     \--- each line has its own 
             |    | 1-13 |/OE     |          10K resistor to GND 
       VCC 14|    |      |        | 16 VCC 
       GND  7|    |      |        |  8 GND 
             +----+      +--------+ 


Notes: When both EOI and ATN are low the HC32 enables HC595 outputs
  * If any HC595 output is high the GPIB bus bit will be pulled low
  * IFC low resets the outputs low
