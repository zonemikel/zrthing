# ZRThing

### Author 
Michael McCarty

### Description
This is the general purpose code for the ZRThing IoT device. The device was created to be a multi use "widget" of sorts to capitalize on the awesome esp8266 wifi devices. These wifi devices are very powerful but out of the box they need all kinds of connections and power circuits just to get working. So at zonerobotics we created a easy to use module so you have everything you need from the start. The ZRThing module includes the power circuits required as well as hbridges for motor control. It also has easy to use micro-jst connectors so you can plug your modules directly into it, and it has a built in battery charger. 

### Hardware Features
- Hbridge for control of 2 motors
- Servo motor connector and control
- 3.3v LDO for powering module
- Lithium Ion battery charger (18650 recommended)
- Micro jst connectors for programming, motors, charger, power, sensors
- Program (or user) button and reset button (can restart in program mode)
- rgb LED

### Software features (this repo)
- Captive web portal 
- Web API/page
-- Set client AP un/pw
-- set time
-- set/read gpio 
-- set/read analog
-- servo control
-- D1W interaction
- Auto dweet.io integration based off unique mac
- Persistant setting storage in eeprom
- Generation of unique name based off MAC

### Current state
Not all of this is done or tested, I'm still largely working on this. 

### 3d enclosure
search onshape for zrthingenclosure

### Applications
Livestock/Pet door - TODO link
Educational 

## More info webpage
- note this is not up yet
zrthing.com or data.zonerobotics.com

