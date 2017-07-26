
// log level, anything lower than this
// is printed, for setting verbosity
int loglevel = 6; 

// any needed startup
void initUser(){
  // serial, wifi and webserver all done in zr_lib
  Serial.println("\r InitUser");
}

// simple log helper
void log(int lvl, String msg){
  if(lvl <= loglevel){
  Serial.println("\r"+String(millis())+":"+msg);
  }
}

// user loop (superseeds wifi loop, be careful)
void loopUser(){
    wifiloop();
}
