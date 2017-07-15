

////////////////////////////////////////////////////////
// Author: Michael McCarty, ZoneRobotics
// Version: see define
// Details: See zrthing.com, or data.zonerobotics for more info
// License: Totally free, totally open use however you want
//   I use other free packages which may have different licenses
////////////////////////////////////////////////////////
#include "ExtLibs.h"

String ZRVER="0.0.1e_door";

void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Startup \r\n zr_main"+ZRVER);
  initUser();
}


void loop() 
{
  loopUser();
}

