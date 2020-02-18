## AVR ATMEGA1284P pin assignments for HP85 Disk
  * @see Documents/HP85Disk.pdf for a hand drawn diagram
  * GPIB:  Each GPIB pin (8 data and 8 control lines ) attach to CPU via 120 ohm current limit resistor .
    * Each GPIB connector pin (8 data and 8 control lines) have a 10K pull-up resistor to VCC.
  * ISP header: MOSI,MISO,SCK,/Reset connects directly to ISP header
  * Micro SD Interface: MOSI,MISO,SCK attach to CPU function via a 1k series resistor.
    * Micro SD interface has level shifters and internal 5V to 3.3V regulator
    * PB3 /CS must have a 10K pull-up up to VCC to prevent access during ISP programming
    * PB4 should have a 10K pull up help assure the SPI bus does not go into slave mode.
  * RS232 TTL: connect to FTDI232 USB  board which also provides 5V VCC power to all circuits..
  * I2C: SCL,SDA connect to optional DS1307 RTC board with each line having a 2k2 pull-up
<pre>

                       ATMEGA1284P (and ATMEGA644P) 
                       +---V---+ 
     5 EOI INT0  PB0  1|       |40  PA0      D1  1 
     6 DAV INT1  PB1  2|       |39  PA1      D2  2 
       PP  INT2  PB2  3|       |38  PA2      D3  3 
    SD /CS  PWM  PB3  4|       |37  PA3      D4  4 
       NC   PWM  PB4  5|       |36  PA4      D5 13 
    SD     MOSI  PB5  6|       |35  PA5      D6 14 
    SD     MISO  PB6  7|       |34  PA6      D7 15 
    SD      SCK  PB7  8|       |33  PA7      D8 16 
    10K pull-up  /RST  9|       |32  AREF     0.1uf 
       +5        VCC 10|       |31  GND      GND   
       GND       GND 11|       |30  AVCC     +5    
    20MHZ      XTAL2 12|       |29  PC7      NC    
    20MHZ      XTAL1 13|       |28  PC6      NC    
       RX   RX0  PD0 14|       |27  PC5  TDI JTAG 
       TX   TX0  PD1 15|       |26  PC4  TDO JTAG 
     7 NRFD RX1  PD2 16|       |25  PC3  TMS JTAG 
     8 NDAC TX1  PD3 17|       |24  PC2  TCK JTAG 
     9 IFC  PWM  PD4 18|       |23  PC1  SDA I2C   
    10 SRQ  PWM  PD5 19|       |22  PC0  SCL I2C  
    11 ATN  PWM  PD6 20|       |21  PD7  PWM REN 17 
                       +-------+ 
</pre>




___ 

## Parallel Poll Response circuit
  * Uses: Three chips 74HC05, 74HC32, 74HC595
  * Parallel Poll Response must be less than 2 Microseconds therefore we use hardware to do it!
  * @see Documents/HP85Disk.pdf for a hand drawn diagram


<pre>
    ATMEGA               HC595             HC05 
                      +----V----+          +-V-+  
    3 PB2 -------- 12 |RCLK   Q0| 15 -x- 1 |   | 2 --- GPIB D8 
    6 MOSI ------- 14 |SER    Q1| 1  -x- 3 |   | 4 --- GPIB D7 
    8 SCK -------- 11 |SRCLK  Q2| 2  -x- 5 |   | 6 --- GPIB D6 
    9 IFC -------- 10 |SRCLR  Q3| 3  -x- 9 |   | 8 --- GPIB D5 
           HC32       |         |     |    |   | 7 GND 
          +-V-+       |         |     |    |   |14 VCC 
     EOI 2|   |       |         |     |    +---+ 
     ATN 1|   |       |         |     \--- each line has its own 
          |   | 3--13 |/OE      |          10K resistor to GND 
    VCC 14|   |       |         | 16 VCC 
    GND  7|   |       |         |  8 GND 
          +---+       +---------+ 
</pre>

Notes: When both EOI and ATN are low the HC32 enables HC595 outputs
  * If any HC595 output is high the GPIB bus bit will be pulled low
  * IFC low resets the HC595 outputs low - so the HC05 outputs will float.

___ 

## Pictures
  * [HP85A with connected h85disk V1 board - wire wrapped](Photos/HP85-with-hp85disk.png)
  * [hp85disk V1 board closeup - wire wrapped](Photos/hp85disk-V1-board.png)

___ 
