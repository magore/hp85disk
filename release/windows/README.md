## Minimal Windows 6-wire ISP based flashing
  * **You MUST have an external In System Programmer (ISP) to use these steps**
  * **I expect very few users will need these instructions unless their firmware becomes corrupted in some yet unforeseen way**

### ISP options
  * See **Listing ISPs that avrdude supports** below after installing software in next step
  * **avrispv2** is a very low cost USB based *In System programmer* **ISP** about $10
    * Low cost Pololu USB AVR Programmer V2.1
    * https://www.robotshop.com/ca/en/pololu-usb-avr-programmer-v21.html
  * **avrisp** is also a very low cost arduino based *In System programmer* **ISP** you can make yourself
    * Details are at the end of this README

### Windows Software Installation instructions - very MINIMAL
  **Make sure your ISP and hp85disk are NOT attached yet**
  * Hint: Make sure you are network connected so Windows update can automatically install any drivers
  * **Make sure you start these steps without the ISP or hp85disk attached**
  * Install **Windows 10 Python 3.7 App* from the Microsoft App Store
  * Download [windows.zip](https://raw.githubusercontent.com/magore/hp85disk/master/release/windows.zip) 
    * Extract extract it to a folder called windows on your Desktop
  * Open a Command Prompt (search cmd.exe and open)
  * **pip3 install pySerial**
    * Ignore any pip upgrade request/warning 
  * Change into the windows folder on your Desktop 
    * cd Desktop\windows
  * Keep you command prompt open for the following steps
   
### Detecting serial ports for the hp85disk and ISP
  **Make sure your ISP and hp85disk are NOT attached yet**
  * Hint: Make sure you are network connected so Windows update can automatically install any drivers
  * Open a Command Prompt (search cmd.exe and open)
    * Unless you have one open from the previous steps
  * Run the following command
    * **python3 listports.py**
  * Attach the hp85disk to your computer and attach power adapter for the hp85disk board
    * Wait until windows finishes installing the FTDI serial drivers this can take a minute
  * Re-Run 
    * **python3 listports.py**
    * Make a note of the new COM port
  * If you do **NOT** see a new COM port yet wait a minute and try again 
  * Attach your ISP to your computer and rerun the listports.py script
    * Wait until windows finishes installing the FTDI serial drivers
    * Make a note of the COM port
  * Keep you command prompt open for the following steps

### ISP connection to the hp85disk
  * Attach the 6-wire ISP connector to the hp85disk board
    * **Make sure you have PIN 1 connected to PIN on the hp85disk ISP header**
  * **ISP header on the hp85disk PCB**
    * Orient the hp85disk PCB with GPIB connector facing your left and power connector on your right
  * **hp85disk ISP header TOP View**
    <pre>
    hp85disk PCB top view of ISP header
    <== GPIB                                    ==> Power
    | connector                                connector |
    |                2  4  6                             |
    |            Pin 1  3  5                             |
    ================= hp85disk PCB card EDGE =============
    </pre>

  * **TOP X-RAY view of ISP programmer connector that is plugged into hp85disk**
    * Most ISP programmer cable connectors have a small arrow symbol on PIN 1
    * Many ISP programmer cables have a KEY sticking out on the side of the connector
    * Additionally there may be a RED or BLACK mark on the PIN 1 wire of the ISP progremmer header
    <pre>
    TOP X-RAY view of ISP programmer connector that is plugged into hp85disk
      We are looking through the connector from the cable side plugged into the hp85disk
    Note the arrow V  sympbol on the ISP plug
    Note some ISP connectors have a KEY sticking out near pin 3
    ===========
    |  2  4  6 |
    |  1  3  5 |
    =V==|  |====
        ====
    </pre>

    
## flashing the hp85disk and optiboot bootloader
  * NOTE: You can always get updated HEX files from the release/build folder
  * Make sure your hp85disk is power on
  * Open a Command Prompt (search cmd.exe and open)
    * Unless you have one open from the previous steps
  * Discover ISP COM port
    * Attach the ISP programmer USB port to your computer 
    * Use listports.py as documented and above to find the port
  * Example programming with **Arduino as ISP**
    * **flash_release.bat avrisp COM3 115200**
  * Example programming with **Pololu USB AVR Programmer V2.1**
    * **flash_release.bat avrispv2 COM3 115200**
  * **Note: If you get errors retry a few times**

### Troubleshooting
  * Make sure your ISP header is connected the correct way
  * Make sure you have the correct baud rate
  * Make sure you have the correct come port - rerun python3 listports.py
  * It might take some time for Windows to detect and install a new COM port
    * Try waiting a few minutes

### avrisp home made ISP details
  * You need the **Arduino Platform** http://arduino.cc
  * See: https://www.arduino.cc/en/tutorial/arduinoISP
    * This uses a low cost arduino atmega328P and has source code provided with the Arduino platform itself
    * Search for **arduino nano 3.0** cost about $10 and has small form factor and has an arduino boot loader
  * Instructions: https://www.arduino.cc/en/Tutorial/ArduinoISP
  * Notes:
    * Open Arduino Platform
    * **FILE -> Examples -> ArduinoISP**
      * This project is often referred to as **Arduino as ISP** 
      * Find word **BAUDRATE** using search
        * You will see a line that says **#define BAUDRATE	19200** around line 140 in the file
        * **Change this to 115200**
        * This will speed up programming of the hP85disk project
    * Pick the Arduino Board you have - example **Arduino Nano 3.0**
      * **Tools -> Board ->Arduino Nano**
    * Continue with setup and programming Instructions: https://www.arduino.cc/en/Tutorial/ArduinoISP

  * Arduino as ISP header pin-out
    * When connecting the pin numbers must match
    * **Hint** if you are using a 6 wire connector instead of jumpers wires they typically have an arrow marking pin 1
<pre>
       Arduino Pin  hp85disk ISP header Pin
       D12          1
       5V           NC = MUST NOT BE CONNECTED! see note for details
       D13          3
       D11          4
       D10          5
       GND          6
Note 5V connection - DO NOT CONNECT
     The hp85disk and arduino are both powered with their own supplies 
     If one supply is just slightly different in voltage power could flow back though one of the devices
     This could even cause damage to one of the devices - so please leave the 5V connections disconnected
</pre>

### Listing supported In System Programmers (ISP)s that avrdude supports
  * Note: the avrdude.exe binary and support DLL were obtained from an Arduino installation
    * See: https://www.arduino.cc/en/Main/Software 
  * Open a Command Prompt (search cmd.exe and open)
    * Unless you have one open from the previous steps
  * avrdude.exe -c list
  

