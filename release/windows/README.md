## Minimal Windows 6-wire ISP based flashing
  * **You MUST have an external In System Programmer (ISP) to use these steps**

### ISP options
  * See **Listing ISPs that avrdude supports** below after installing software in next step
  * **avrispv2** is a very low cost USB based *In System programer* **ISP** about $10
    * Low cost Pololu USB AVR Programmer V2.1
    * https://www.robotshop.com/ca/en/pololu-usb-avr-programmer-v21.html
  * **avrisp** is also a very low cost arduino based *In System programer* **ISP** you can make yourself
    * Details are at the end of this README

### Windows Software Installation instructions - very MINIMAL
  **Make sure your ISP and hp85disk are NOT attached yet**
  * Hint: Make sure you are network connected so Windows update can automatuically install any drivers
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
   
### Listing ISPs that avrdude supports
  * Note: the avrdude.exe binary and support DLL were obtained from an Arduino installation hppt://arduino.cc
  * Open a Command Prompt (search cmd.exe and open)
    * Unless you have one open from the previous steps
  * avrdude.exe -c list
  
### Detecting serial ports for the hp85disk and ISP
  **Make sure your ISP and hp85disk are NOT attached yet**
  * Hint: Make sure you are network connected so Windows update can automatuically install any drivers
  * Open a Command Prompt (search cmd.exe and open)
    * Unless you have one open from the previous steps
  * Run the following command
    * **python3 listports.py**
  * Attach the hp85disk to your computer and attach power addapter for the hp85disk board
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
    * With the hp85disk GPIB connector facing your left, pin 1 is the 1 in the diagram below
<pre>
GPIB connector is on your left 
ISP header top View
        O O O
        1 O O
PCB edge facing you
</pre>
   * Some ISP cable connectors have a small arrow symbol on PIN 1
   * Some have a read or black mark on the PIN 1 side of the cable

## flashing the hp85disk and optiboot bootloader
  * NOTE: You can always get updated HEX files from the release/build folder
  * Make sure your hp85disk is power on
  * Open a Command Prompt (search cmd.exe and open)
    * Unless you have one open from the previous steps
  * replace the COM port ISP name to your own
  * **flash_release.bat avrispv2 COM8 115200**
   * If you get errors retry a few times 

### Troubleshooting
  * Make sure your ISP header is connected the correct way
  * Make sure you have the correct baud rate
  * Make sure you have the correct come port - rerun python3 listports.py

### avrisp home made ISP details
  * You need the **Arduino Platform** http://arduino.cc
  * See: https://www.arduino.cc/en/tutorial/arduinoISP
    * This uses a low cost arduino atmega328P and has source code provided with the Arduino platform itself
    * Search for **arduino nano 3.0** cost about $10 and has small formfactor and has boot loader
  * Configureing
    * **FILE -> Examples -> ArduinoISP**
      * This project is often referred to as **Arduino as ISP** 
    * **NOTE: the default BAUD rate is 19200 you have to update the BAUD to this value when programming**
  * Connections to hp85disk ISP header:
<pre>
       Arduino Pin  hp85disk ISP header Pin
       D12          1
       5V           NC = Not Connected! see note
       D13          3
       D11          4
       D10          5
       GND          6
Note 5V connection - DO NOT CONNECT
     Given the hp85disk and arduino are both powered with their own supplies it would be bad to connect ther 5V connections together
</pre>
