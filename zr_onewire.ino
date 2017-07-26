#define ONE_WIRE_BUS 5  // gpio5 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
dht11 DHT11;
String respOneWire = "{\"Success\":\"d1w\"}";
float lastReadF;

int readDHT11(){
    int chk = DHT11.read(ONE_WIRE_BUS);
    
    //Serial.print("Read sensor DHT11: ");
    switch (chk)
    {
      case DHTLIB_OK: 
      break;
      
      case DHTLIB_ERROR_CHECKSUM: 
      Serial.println("Checksum error"); 
      break;
      
      case DHTLIB_ERROR_TIMEOUT: 
      Serial.println("Time out error"); 
      break;
      
      default: 
      Serial.println("Unknown error"); 
      break;
    }
    return chk;
}

//Celsius to Fahrenheit conversion
double Fahrenheit(double celsius)
{
  return 1.8 * celsius + 32;
}

//Celsius to Kelvin conversion
double Kelvin(double celsius)
{
  return celsius + 273.15;
}

// dewPoint function NOAA
// reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
// reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
//
double dewPoint(double celsius, double humidity)
{
  // (1) Saturation Vapor Pressure = ESGG(T)
  double RATIO = 373.15 / (273.15 + celsius);
  double RHS = -7.90298 * (RATIO - 1);
  RHS += 5.02808 * log10(RATIO);
  RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
  RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
  RHS += log10(1013.246);

        // factor -3 is to adjust units - Vapor Pressure SVP * humidity
  double VP = pow(10, RHS - 3) * humidity;

        // (2) DEWPOINT = F(Vapor Pressure)
  double T = log(VP/0.61078);   // temp var
  return (241.88 * T) / (17.558 - T);
}

// delta max = 0.6544 wrt dewPoint()
// 6.9 x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
  double a = 17.271;
  double b = 237.7;
  double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
  double Td = (b * temp) / (a - temp);
  return Td;
}

String zr_onewire_getResponse(){
  return respOneWire;
}

int zr_onewire_OneWireRead(String driver){
  int respCode = 200;
  float tempf = 0;
  int tries = 5;
  if(driver.equals("DS18B20")){
    do {
      DS18B20.requestTemperatures(); 
      tempf = DS18B20.getTempFByIndex(0);
      Serial.print("TemperatureF: ");
      Serial.println(tempf);
      lastReadF = tempf;
      tries--;
    } while (tempf == 85.0 || tempf == (-196.0) && tries > 0);
    respOneWire = "{\"TempF\":\""+String(tempf)+"\"}";
  
  }// temp humidity sensor DHT11
  else if(driver.equals("DHT11")){
    int chk = DHT11.read(ONE_WIRE_BUS);
    switch (chk)
    {
      case DHTLIB_OK: 
      respOneWire = "{\"TempF\":\""+String(Fahrenheit(DHT11.temperature))+"\",\"Humidity\":\""+String(DHT11.humidity)+"\"}";
      break;
      
      case DHTLIB_ERROR_CHECKSUM: 
      respCode = 501;
      respOneWire = "{\"Error\":\"Checksum error\"}";
      break;
      
      case DHTLIB_ERROR_TIMEOUT: 
      respCode = 408;
      respOneWire = "{\"Error\":\"Time out error\"}";
      break;
      
      default: 
      respCode = 501;
      respOneWire = "{\"Error\":\"Unknown error\"}";
      break;
    }
  }else{
    respCode = 501;
    respOneWire = "{\"Error\":\"Unknown D1W Driver\"}";
  }
  return respCode;

  
}

void zrOneWireLoop(){
  float tempF = 0;
  //timer.update();
  readDHT11();
  tempF = Fahrenheit(DHT11.temperature);
  WiFiClient client = server.client();
  int sa = analogRead(A0);

  if(tempF > 100.5){
    light = 0;
    digitalWrite(lightgpio, LOW); //LIGHT OFF
  }
  if(tempF < 99.5){
    light = 1;
    digitalWrite(lightgpio, HIGH); //LIGHT ON
  }

  String getReq = (String("GET /dweet/for/") + String(MyName) + String("?temperature=") + String(tempF) + "&humidity=" + String(DHT11.humidity) + "&light=" + String(light) + "&adc=" + sa + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  Serial.print(getReq);

  if(client.connect(host,80)){
  
    // This will send the request to the server
    client.print(getReq);
    delay(10);
    
    // Read all the lines of the reply from server and print them to Serial
    while(client.available()){
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  }else{
    Serial.println("connection to host failed");
  }

  delay(2000);
}

