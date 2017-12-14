#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <SHT1X.h>
#include <stdio.h>

/* Pin Numbers Definition */
int ldrPin = A0;
int servoPin = D0;
int shSCKPin = D1;
int shDATAPin = D2;
int ssegDotPin = D3;
int ssegAPin = D5;
int ssegBPin = D6;
int ssegCPin = D7;
int ssegDPin = D8;

/* Variables */
char str[8];
int degree;
int tempC = 0;
int humidity = 0;
int sevenSegmentValue = -1;
int light = 0;
const int numbers[11][4] = { {0, 0, 0, 0},
                             {0, 0, 0, 1},
                             {0, 0, 1, 0},
                             {0, 0, 1, 1},
                             {0, 1, 0, 0},
                             {0, 1, 0, 1},
                             {0, 1, 1, 0},
                             {0, 1, 1, 1},
                             {1, 0, 0, 0},
                             {1, 0, 0, 1},
                             {1, 1, 1, 1}};
Servo ourServo;

/* WiFi Information */
const char* SSID = "WiFi_SSID";
const char* password = "WiFi_Password";
ESP8266WebServer WebServer(80);
WiFiClient client;
MDNSResponder mdns;

void setup()
{
  //7-Segment
  pinMode(ssegAPin, OUTPUT);
  pinMode(ssegBPin, OUTPUT);
  pinMode(ssegCPin, OUTPUT);
  pinMode(ssegDPin, OUTPUT);
  pinMode(ssegDotPin, OUTPUT);

  //Turn 7-Segment off
  sevenSegmentValue = -1;
  setSevenSegment();
  digitalWrite(ssegDotPin, 1);
    
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, password);

  //Display c on 7seg(Connecting)
  digitalWrite(ssegAPin, 0);
  digitalWrite(ssegBPin, 1);
  digitalWrite(ssegCPin, 0);
  digitalWrite(ssegDPin, 1);
  
  while (WiFi.status() != WL_CONNECTED)
    delay(500);

  WebServer.begin();
  IPAddress myIP = WiFi.localIP();

  // Start MDNS
  mdns.begin("drivemonit", WiFi.localIP());
    
  //Turn 7seg off
  sevenSegmentValue = -1;
  setSevenSegment();

  //Display IP on 7-Segment
  displayIP(myIP);

  //Listen on /api/v1/status
  WebServer.on("/api/v1/status", handleStatusMessage);

  //Listen on NotFound
  WebServer.onNotFound(handleNotFound);

  //Listen on /api/v1/display
  WebServer.on("/api/v1/display", handleDisplayMessage);
}

void loop()
{
  WebServer.handleClient();
}

void handleStatusMessage() 
{
  char message[400];
  if(WebServer.method() == HTTP_GET)
  {
    light = analogRead(ldrPin);
    light = map(light, 0, 1023, 0, 100);

    SHT1x sht15(shDATAPin, shSCKPin); //Data, SCK
    tempC = sht15.readTemperatureC();
    humidity = sht15.readHumidity();
        
    sprintf(message, " {\"temperature\": %d, \"light\": %d, \"humidity\": %d, \"sevenSegment\": %d} ", tempC, light, humidity, sevenSegmentValue);
    WebServer.send(200, "application/jsonrequest", message);
  }
  else
    handleNotFound();
}

void handleDisplayMessage()
{
  if(WebServer.method() == HTTP_POST)
  {
    if(WebServer.arg(0) == "sevenSegment")
    {
      if(WebServer.arg(1).toInt() >= -1 && WebServer.arg(1).toInt() < 10)
      {
        sevenSegmentValue = WebServer.arg(1).toInt();
        setSevenSegment();
        WebServer.send(200, "text/plain", "");
      }
      else
        WebServer.send(422, "text/plain", "Out of Range Input");
    }
    else if(WebServer.arg(0) == "servo")
    {
      if(WebServer.arg(1).toInt() >= 0 && WebServer.arg(1).toInt() < 360)
      {
        degree = WebServer.arg(1).toInt();
        setServo();
        WebServer.send(200, "text/plain", "");        
      }
      else
        WebServer.send(422, "text/plain", "Out of Range Input");
    }
    else
      handleNotFound();
  }
  else
  {
    WebServer.sendHeader("Access-Control-Allow-Headers", "Content-Type, Content-Length, Authorization, Accept, X-Requested-With", false);
    WebServer.sendHeader("Access-Contrl-Allow-Methods", "POST, GET, OPTIONS", false);
    WebServer.sendHeader("Access-Control-Max-Age", "86400", false);
    WebServer.send(200, "text/plain", "");
  }
}

void handleNotFound() 
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += WebServer.uri();
  message += "\nMethod: ";
  message += (WebServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += WebServer.args();
  message += "\n";
  for (uint8_t i = 0; i < WebServer.args(); i++)
    message += " " + WebServer.argName(i) + ": " + WebServer.arg(i) + "\n";
  WebServer.send(404, "text/plain", message);
}

void setServo() // Set degree variable then call this to go to that degree
{
  ourServo.attach(servoPin);
  for(int pos = 0; pos <= degree ; pos++)
  {
    ourServo.write(pos);
    delay(15);
  }
  ourServo.detach();
}

void setSevenSegment() // Set sevenSegmentValue variable then call this to display that number
{
  if(sevenSegmentValue == -1)
  {
    digitalWrite(ssegAPin, 1);
    digitalWrite(ssegBPin, 1);
    digitalWrite(ssegCPin, 1);
    digitalWrite(ssegDPin, 1);  

  }
  else
  {
    digitalWrite(ssegAPin, numbers[sevenSegmentValue][3]);
    digitalWrite(ssegBPin, numbers[sevenSegmentValue][2]);
    digitalWrite(ssegCPin, numbers[sevenSegmentValue][1]);
    digitalWrite(ssegDPin, numbers[sevenSegmentValue][0]);    
  }
  
}

void displayIP(IPAddress myIP) // Display IP on 7-SEGMENT
{
  int digit1, digit2, digit3;
  
  for(int i = 0 ; i < 4 ; i++)
  {
    digit1 = myIP[i] / 100;
    digit2 = (myIP[i] - (digit1 * 100)) / 10;
    digit3 = myIP[i] - (digit1 * 100) - (digit2 * 10);

    if(digit1 != 0)
    {
      sevenSegmentValue = digit1;
      setSevenSegment();

      delay(500);      
    }
    
    sevenSegmentValue = -1;
    setSevenSegment();

    delay(100);
          
    if(digit2 != 0)
    {
      sevenSegmentValue = digit2;
      setSevenSegment();

      delay(500);      
    }

    sevenSegmentValue = -1;
    setSevenSegment();

    delay(100);

    sevenSegmentValue = digit3;
    setSevenSegment();
    
    delay(500);      

    sevenSegmentValue = -1;
    setSevenSegment();

    delay(100);

    if(i != 3)
    {
      digitalWrite(ssegDotPin, 0);
      delay(250);
      digitalWrite(ssegDotPin, 1);
    }
  }
}
