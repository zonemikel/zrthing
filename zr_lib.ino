/////////////////////
// Pin Definitions //
/////////////////////
const int LED_GRN_PIN = 0;  // rgb green pin, low means on
const int LED_BLU_PIN = 4;  // rgb blue pin, low means on
const int ANALOG_PIN = A0;  // Analog pin, wired to vdiv on bat voltage
const int D1WIRE_PIN = 2;   // Dallas 1 wire pin, for 1 wire port
const int SERVO_PIN  = 5;   // pin header for servo
const int M1_A      = 12;  // motor1 A // below hooked up to l9110s dc motor control
const int M1_B      = 13;  // motor1 B
const int M2_A      = 14;  // motor2 A
const int M2_B      = 15;  // motor2 B

#include "Timer.h"
// GPIO controls heat light
// remember servo is 4 and blue is 5 (transposed!)
#define lightgpio 4
#define mtr1a 12
#define mtr1b 13
int light = 0;
char gpiostats[15]; // holds a char representing current gpio state
String respJsonContent = "{\"Success\":\"gpio\"}";

void checkSetPin(int pin, char pinmode){
    if(gpiostats[pin] == pinmode){
      return;
    }else{
      if(pinmode == 'O'){     // output
        pinMode(pin, OUTPUT);
        gpiostats[pin] = 'O';       
      }// intput
      else if(pinmode == 'I'){ // input
        pinMode(pin, INPUT);
      }
      else if(pinmode == 'P'){
        pinmode = 'I'; // just set as normal input
        pinMode(pin, INPUT_PULLUP);
      }
      else if(pinmode == 'S'){ // servo
        espservo.attach(pin);
        espservo.write(90);
      }
    }
    gpiostats[pin] = pinmode;
  
}

String getTimeString(){
  return String(hour()) +":"+ String(minute()) +":"+ String(second()) +"_"+ String(day()) +"/"+ String(month()) +"/"+ String(year());
}

// any needed startup
void initZRLib(){
  Serial.begin(115200);
  Serial.print("initZRLib\r\n");
  
  pinMode(A0, INPUT);

  pinMode(M1_A, OUTPUT);
  pinMode(M1_B, OUTPUT);
  pinMode(M2_A, OUTPUT);
  pinMode(M2_B, OUTPUT);

  zr_onewire_OneWireRead("DS18B20");
  Serial.print(zr_onewire_getResponse());
  wifi_setup(); // setup the wifi see webServer
  initUser();
}

// the main loop
void loopZRLib(){
  wifiloop(); // handle all wifi stuff see webServer
  loopUser(); // handle what the user code wants
}

String zr_getResponse(){
  return respJsonContent;
}

int zr_gpio(int pin, String cmd){
  int respCode = 200;
  respJsonContent = "{\"Success\":\"gpio\"}";
  if(cmd.equals("1")){
          checkSetPin(pin,'O');
          digitalWrite(pin, HIGH);
          }   
    else if(cmd.equals("0")){
          checkSetPin(pin,'O');
          digitalWrite(pin, LOW);
          }     
    else if(cmd.equals("P")){ // unused, todo use or remove
          checkSetPin(pin,'P');
          }       
    else if(cmd.equals("read")){
          checkSetPin(pin,'I');
    }
    else if(cmd.equals("setbyte")){
          // todo iterate over mask in "pin" 
          // write 1 to all that are in mask and set to "1"
    }
    else if(cmd.equals("clearbyte")){
          // todo iterate over mask in "pin" 
          // write 0 to all that are in mask and set to "O"
    }
    else if(cmd.equals("readbyte")){
          // todo iterate over mask in "pin"
          // if is in mask and set to "I" read into and return
          respJsonContent = "{\"readbyte\":\"0xFF\"}";
    }else{
      respCode = 400;
    }
    return respCode;
}

int zr_analog(int pin, String cmd, int writeValue){
  int analogVal = 0; 
  respJsonContent = "{\"Success\":\"Analog\"}";
  int respCode = 200;
  if(cmd.equals("read")){
    analogVal = analogRead(pin);
    respJsonContent = "{\"Pin\":\""+String(pin)+"\",\"ReadValue\":\""+String(analogVal)+"\"}";
  }
  else if(cmd.equals("write")){
    checkSetPin(pin,'O');
    if(writeValue > 1023){ writeValue = 1023; }
    analogWrite(pin, writeValue);
  }
  else{
    // done
    respJsonContent = "{\"Error\":\"Unknown Command\"}";
    respCode = 400;
  }
  // done
}

int zr_servo(int pin, int writeValue){
  int respCode = 200;
  // dont let them send crazy value
  if(writeValue > 180){ writeValue = 180; }
  // make sure that pin is setup for servo
  checkSetPin(pin,'S');
  // write to servo
  espservo.write(writeValue);
  // done
  return respCode;
}

int zr_d1w(int pin, String cmd, String val){
  float tempf;
  int tries = 5;
  
  //TODO: validate pin and value, return errors if needed
  // remember 400 is bad request
  respJsonContent = "{\"Success\":\"Dallas One Wire\"}";
  int respCode = 200;
  
  if(cmd.equals("read")){
      // Temp Sensor DS18B20
      zr_onewire_OneWireRead(val);
      respJsonContent = zr_onewire_getResponse();
  }
  else if(cmd.equals("write")){
    respCode = 501;
    respJsonContent = "{\"Error\":\"Unknown Command\"}";
  }
  else{
    // done
    respJsonContent = "{\"Error\":\"Unknown Command\"}";
    respCode = 400;
  }
}




