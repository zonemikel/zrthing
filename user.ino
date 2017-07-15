// defines
#define topsw     4   // topsw gpio 
#define bottomsw  5   // bottom sw gpio
#define USEM1     1   // use m1 not m2 
#define M1_IA     12  // gpio for motor
#define M1_IB     13  // gpio for motor
#define M2_IA     14  // gpio for motor
#define M2_IB     15  // gpio for motor

// light sensor is analog value
// if its below this (darker) then its considered night
#define NIGHTVAL 1000
// we are not using light sensor but time in this one
#define lightSensorMode false 

// logging stuff
#define logdebug 5
#define logsevere 0
#define loglevel logdebug

// time to get off switch
#define delayOffSw 2000

// globals
int travelTime = 0; // time to lift up door
int iterations = 0;
int doorState = 0; // 0 lowered, 1 lifted
int configMode = 0;

// Time to sleep (in seconds):
const int sleepTimeS = 60;

 
// any needed startup
void initUser(){
  pinMode(topsw, INPUT_PULLUP);
  pinMode(bottomsw, INPUT_PULLUP);
  pinMode(M1_IA, OUTPUT);
  pinMode(M1_IB, OUTPUT);
  pinMode(M2_IA, OUTPUT);
  pinMode(M2_IB, OUTPUT);
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  // set these high so rgb led is off
  // conserve power
  digitalWrite(0, HIGH);
  digitalWrite(2, HIGH);
  
  // startup serial
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Startup \r\n User Code");

  // todo later add a bit of data to eeprom
  // but only like once a day for 50k limit
  //EEPROM.begin(512);

  // test startup
  //motorUp();
  //delay(1000);
  //motorStop();

  if(!lightSensorMode){
    Serial.println("Light Sensor mode Disabled");
  }

  printVoltage();
  printSWState();
  // if sw1 & sw2 not hit go into config mode 
  if(!topSwitchClosed() && !bottomSwitchClosed()){
    Serial.println("Config Mode ");
    configMode = 1;
    wifi_setup();   
  }else{
    Serial.println("Run Mode ");
  }
  
}


// simple motor controls
void motorDown(){
  if(USEM1){
    digitalWrite(M1_IA, LOW);
    digitalWrite(M1_IB, HIGH); 
  }else{
    digitalWrite(M2_IA, LOW);
    digitalWrite(M2_IB, HIGH); 
  }
}
void motorUp(){
  // onmotor up ? spin auger for feed motor?
  if(USEM1){
    digitalWrite(M1_IA, HIGH);
    digitalWrite(M1_IB, LOW); 
  }else{
    digitalWrite(M2_IA, HIGH);
    digitalWrite(M2_IB, LOW);   
  }
  
}
void motorStop(){
  if(USEM1){
    digitalWrite(M1_IA, LOW);
    digitalWrite(M1_IB, LOW); 
  }else{
    digitalWrite(M2_IA, LOW);
    digitalWrite(M2_IB, LOW); 
  }
}

void log(int lvl, String msg){
  if(lvl <= loglevel){
  Serial.println("\r"+String(millis())+":"+msg);
  }
}

boolean switchHit(int timeoutms){
  int i=0;
  while(i<timeoutms){
    if(topSwitchClosed() || bottomSwitchClosed()){
      return true;
    }
    delay(1);
    i++;
  }
  return false;
}

boolean getOffSwitch(int timeoutms){
  while(timeoutms > 0){
    if(topSwitchClosed()){
      motorDown();
    }else if(bottomSwitchClosed()){
      motorUp();
    }else{
      break;
    }
    delay(1);
    timeoutms--;
  }
  motorStop();
  if(timeoutms > 0){
    return true;
  }
  return false;
}

boolean topSwitchClosed(){
  return digitalRead(topsw);
}

boolean bottomSwitchClosed(){
  return digitalRead(bottomsw);
}

boolean isLightOutside(){ 
  int anal = analogRead(A0);
  log(logdebug, " Analog: "+String(anal));
  return (anal > NIGHTVAL);
}

// dealing with door
void raiseDoor(){
  log(logdebug, "Raising Door");
  
  
  if(!getOffSwitch(delayOffSw)){
    log(logsevere, "Could not get off switch");
    return;
  }

  motorUp();
  
  if(switchHit(10000)){
    if(topSwitchClosed()){
      log(logdebug, "Door Raised");
    }else{
      log(logsevere, "Bottom Switch Hit while Raising");
    }
  }else{
    log(logsevere, "Could not raise door, neither switch hit");
  }
  motorStop();
}

void lowerDoor(){
  log(logdebug, "Lowering Door");
  
  if(!getOffSwitch(delayOffSw)){
    log(logsevere, "Could not get off switch");
    return;
  }

  motorDown();

  if(switchHit(10000)){
    if(bottomSwitchClosed()){
      log(logdebug, "Door Lowered");
    }else{
      log(logsevere, "Top Switch Hit while Raising");
    }
  }else{
    log(logsevere, "Could not raise door, neither switch hit");
  }
  motorStop();
}

void printVoltage(){
  int AnalogReads[5] = {0,0,0,0,0};
  int sum = 0;
  for(int i=0; i< 5; i++){
    AnalogReads[i] = analogRead(A0);
    Serial.println("\r\n Reads: "+String(AnalogReads[i]));
  }
  // sum up values
  for(int i=0; i< 5; i++){
    sum += AnalogReads[i];
    //Serial.println("\r\n sum: "+String(sum));
  }
  float adcAverage = sum/(5.0*1024.0);
  // calculate and print voltage
  float voltage = adcAverage*6;
  // 5.45 is from (r2/(r1+r2))*Vo
  // where r1=10 and r2=2.2 
  // comparing with vom 6 was closer to reality, so using 6
  
  log(logdebug, " Battery Voltage: "+String(voltage)+" Iterations: "+String(iterations));
  
}
void printSWState(void){
  log(logdebug, "Lowersw: "+String(digitalRead(bottomsw)) + " Uppersw:"+digitalRead(topsw) + " IsLightOutside:"+isLightOutside());
}

void loopUser(){
  // get the current analog reading
  iterations++;

  // print some more info every 10 iterations (50sec)
  //if(iterations % 1 == 0){
    
  //}

  if(lightSensorMode){
    
    printSWState();
    
    // if its light outside and door is not open
    if(isLightOutside() && !topSwitchClosed()){
      raiseDoor();
    }
    // if its not light outside and door open
    if(!isLightOutside() && !bottomSwitchClosed()){
      lowerDoor();
    }
  }

  if(configMode){
    wifiloop();
  }

  // go into deep sleep mode conserving batt 
  // Serial.println("ESP8266 in deep sleep mode");
  // ESP.deepSleep(sleepTimeS * 1000000);
  
}
