#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include <Servo.h> 
#include <DallasTemperature.h>
#include "TimeLib.h"
#include "OneWire.h"
#include "webStrings.h"


 
ESP8266WebServer server(80);
#define DEBUG_ESP_HTTP_SERVER

Servo espservo;

// TODO: convert all responses to json, its human readable and better
// TODO: check input

const char* passphrase = "zonerobotics";
String st;
String content;
char MyName[43];
int statusCode;
DNSServer dnsServer;

void wifiloop(){
  // all in the webserver 
  server.handleClient();
}

void wifi_setup() {
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(10);
  Serial.println();
  Serial.println();
  getUniqueName();
  Serial.println("Startup \r\n Unique Name: "+String(MyName));

  /* We dont do client mode, we will add this later 
  // todo if certain gpio is set clear the eprom (factory reset)
  // todo move to own function in zrlib, we may have more reset stuff
  pinMode(4, INPUT);
  if(digitalRead(4)){
    Serial.println("clearing eeprom");
    for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
    EEPROM.commit();
  }
  
  // read eeprom for ssid and pass
  Serial.println("Reading EEPROM ssid");
  // if the first byte is not a ascii char
  // assume the whole thing is empty
  if(EEPROM.read(0) != 0){
    String esid;
    for (int i = 0; i < 32; ++i)
    {
      esid += char(EEPROM.read(i));
    }
    Serial.print("SSID("+String(esid.length())+"): ");
    Serial.println(esid);
    Serial.println("Reading EEPROM pass");
    String epass = "";
    for (int i = 32; i < 96; ++i)
      {
        epass += char(EEPROM.read(i));
      }
    Serial.print("PASS: ");
    Serial.println(epass); 
    // using len here does not work
    // todo remove 
    if ( esid.length() > 1 ) {
        WiFi.begin(esid.c_str(), epass.c_str());
        if (testWifi()) {
          launchWeb(0);
          return;
        } 
    }
  }else{ */
    Serial.print("No Stored Network");
    setupAP();
  // }
}

bool testWifi(void) {
  int c = 0;
  Serial.println("Waiting for Wifi to connect");  
  while ( c < 20 ) {
    if (WiFi.status() == WL_CONNECTED) { return true; } 
    delay(500);
    Serial.print(WiFi.status());    
    c++;
  }
  Serial.println("");
  Serial.println("Connect timed out, opening AP");
  return false;
} 

void launchWeb(int webtype) {
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Local IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("SoftAP IP: ");
  Serial.println(WiFi.softAPIP());
  createWebServer(webtype);
  // Start the server
  server.begin();
  Serial.println("Server started"); 
}

void setupAP(void) {

  /* no need to scan till we allow client mode
  WiFi.mode(WIFI_STA);
  //WiFi.disconnect();
  Serial.println("Debug 1");
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
     {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
     }
  }
  Serial.println(""); 
  st = "<select name='SSID'>";
  for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      st += "<option value="+WiFi.SSID(i)+">"+WiFi.SSID(i)+" ("+WiFi.RSSI(i)+")"+"</option>";
      //st += WiFi.SSID(i);
      //st += " (";
      //st += WiFi.RSSI(i);
      //st += ")";
      //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*";
      //st += "</li>";
    }
  st += "</option>";
  delay(100);
  

  WiFi.softAP(MyName, passphrase, 6);
  */
  // captive portal 
  IPAddress ip(192, 168, 4, 1);
  IPAddress gateway(192, 168, 1, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(192, 168, 1, 1);
  WiFi.softAP(MyName, NULL);
  WiFi.config(ip, gateway, subnet, dns);
  dnsServer.start(53, "*", ip);
  Serial.println("softap: "+ String(MyName));
  launchWeb(1);
  Serial.println("over");
}

void getUniqueName(){
  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 4], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 3], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "ZR" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(MyName, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    MyName[i] = AP_NameString.charAt(i);
}



void createWebServer(int webtype)
{
  // these urls are always available
  server.on("/gpio", []() {
    int pin = server.arg("pin").toInt();
    String cmd = server.arg("cmd");
    
    int respCode = zr_gpio(pin, cmd);
    
    server.send(respCode, "application/json", zr_getResponse());  
  });

  server.on("/analog", []() {
    int pin = server.arg("pin").toInt();
    String cmd = server.arg("cmd");
    int writeValue = server.arg("val").toInt();

    // call zrlib
    int respCode = zr_analog(pin, cmd, writeValue);

    // send resp
    server.send(respCode, "application/json", zr_getResponse());  
  });// end analog

  server.on("/servo", []() {
      int pin = server.arg("pin").toInt();
      int writeValue = server.arg("val").toInt();

      // call zrlib
      int respCode = zr_servo(pin, writeValue);

      // send resp
      server.send(respCode, "application/json", zr_getResponse());  
  });// end analog

  server.on("/d1w", []() {
      int pin = server.arg("pin").toInt();
      String cmd = server.arg("cmd");
      String val = server.arg("val");

      // call zrlib
      int respCode = zr_d1w(pin, cmd, val);
 
      // send resp
      server.send(respCode, "application/json", zr_getResponse());  
  });// end d1w
  
  server.on("/zrlib", []() {
      int pin = server.arg("pin").toInt();
      String val = server.arg("val");
      String cmd = server.arg("cmd");
      float tempf;
      int tries = 5;
      
      //TODO: validate pin and value, return errors if needed
      // remember 400 is bad request
      content = "{\"Success\":\"zrlib\"}";
      int respCode = 200;
      
      server.send(respCode, "application/json", content);  
  });// end d1w

  server.on("/getver", []() {
      content = "{\"Success\":\""+ZRVER+"\"}";
      int respCode = 200;
      
      server.send(respCode, "application/json", content);  
  });// end getver
  
  // need to see who i'm connected to 
  server.on("/id", []() {

      content = "{\"Success\":\"zrlib\"}";
      zr_gpio(5, "0");
      delay(1000);
      zr_gpio(5, "1");
      int respCode = 200;
      server.send(respCode, "application/json", content);  
  });// end id

  server.on("/settingpetdoor", []() {
    String opentime = server.arg("opentime");
    String closetime = server.arg("closetime");
    String upsec = server.arg("upsec");
    String downsec = server.arg("downsec");
    statusCode = 200;
    content = "{\"Success\":\"Saved\"}";
    if (opentime.length() > 0 && opentime.length() > 0) {
      // todo call zr_lib function to store in eeprom
    }
    server.send(statusCode, "application/json", content);
  });

  server.on("/settime", []() {
    String newtime = server.arg("newtime");
    statusCode = 200;
    content = "{\"Success\":\""+newtime+"\"}";
    if (newtime.length() > 0 && newtime.length() > 0) {
      String year = newtime.substring(0,4);
      String month = newtime.substring(5,7);
      String day = newtime.substring(8,10);
      String hour = newtime.substring(11,13);
      String minute = newtime.substring(14,16);
      String testTime = String(year) + '.' + String(month) + '.' + String(day) + '.' + String(hour) + '.' + String(minute);
      Serial.println("Test Time: "+ testTime);
      // todo call zr_lib function to store in eeprom
      setTime(hour.toInt(), minute.toInt(), 0, day.toInt(), month.toInt(), year.toInt());
    }
    server.send(statusCode, "application/json", content);
  });

  // if they need to connect to wifi 
  if ( webtype == 1 ) { // cant connect to user wifi, default, or no settings stored in eeprom
    server.on("/", []() {
        IPAddress ip = WiFi.softAPIP();
        String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
        content = "<!DOCTYPE HTML>\r\n<html>";
        // general infos 
        content += "<h3>Your ZRThing</h3>";
        content += "ZRThing ID: "+String(MyName)+ " (Write that down!)<br> ";
        content += "Current Ip: "+ ipStr;
        content += "<br>Time: " + String(hour()) +":"+ String(minute()) +":"+ String(second()) +" "+ String(day()) +"/"+ String(month()) +"/"+ String(year());
        // connect to exising wifi 
        content += "<h3>Connect to your wifi</h3><form method='get' action='setting'>";
        content += st; // drop down option of wifi's found
        // todo add gateway and ip so user can specify
        content += "<br><label>PASS: </label><input name='pass' length=64><input type='submit'><br></form>";
        // local functions 
        content += "<h3>ID your ZRThing</h3>";
        content += "<form method='get' action='id'><input type='submit' value='ID'><br></form>";
        // in webstrings.h
        content += setTimeJS;
        content += setTimeHTML;
        // in webstrings.h
        content += petDoorHTML;
        content += "</html>";
        server.send(200, "text/html", content);  
    });
    //server.on("/req", []() {
    //    content = "<!DOCTYPE HTML>\r\n<html>RESP ";
    //    content += getResponse();
    //    content += "</html>";
    //    server.send(200, "text/html", content);  
    //});    
    server.on("/setting", []() {
        String qsid = server.arg("ssid");
        String qpass = server.arg("pass");
        if (qsid.length() > 0 && qpass.length() > 0) {
          Serial.println("clearing eeprom");
          for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
          Serial.println(qsid);
          Serial.println("");
          Serial.println(qpass);
          Serial.println("");
            
          Serial.println("writing eeprom ssid:");
          for (int i = 0; i < qsid.length(); ++i)
            {
              EEPROM.write(i, qsid[i]);
              Serial.print("Wrote: ");
              Serial.println(qsid[i]); 
            }
          Serial.println("writing eeprom pass:"); 
          for (int i = 0; i < qpass.length(); ++i)
            {
              EEPROM.write(32+i, qpass[i]);
              Serial.print("Wrote: ");
              Serial.println(qpass[i]); 
            }    
          EEPROM.commit();
          content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
          statusCode = 200;
        } else {
          content = "{\"Error\":\"404 not found\"}";
          statusCode = 404;
          Serial.println("Sending 404");
        }
        server.send(statusCode, "application/json", content);
    });
  } else if (webtype == 0) { // connected to user router
    server.on("/", []() {
      IPAddress ip = WiFi.localIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "Connected to your wifi ! Your ip (on your network) is " + ipStr;
      content += "<br>Click below to reset to default<form method='get' action='cleareeprom'><input type='submit' value='reset'></form>";
      server.send(200, "text/html", content);
    });
    server.on("/cleareeprom", []() {
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>Clearing the EEPROM</p></html>";
      server.send(200, "text/html", content);
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
      EEPROM.commit();
    });
  }
}

