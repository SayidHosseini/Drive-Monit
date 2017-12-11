#include <string.h>
#include <SHT1X.h>
#include <ESP8266WiFi.h>
#include <Servo.h>

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
float tempC = 0;
float humidity = 0;
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
const char* SSID = "V10_8081";
const char* password = "1a2s3d4f";
WiFiServer WebServer(80);
WiFiClient client;

void setup()
{
  Serial.begin(115200);
  Serial.println();

  //7-Segment
  pinMode(ssegAPin, OUTPUT);
  pinMode(ssegBPin, OUTPUT);
  pinMode(ssegCPin, OUTPUT);
  pinMode(ssegDPin, OUTPUT);
  pinMode(ssegDotPin, OUTPUT);

  //Turn 7-Segment off
  digitalWrite(ssegAPin, 1);
  digitalWrite(ssegBPin, 1);
  digitalWrite(ssegCPin, 1);
  digitalWrite(ssegDPin, 1);
  digitalWrite(ssegDotPin, 1);
    
  WiFi.disconnect();
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, password);

  //Display c on 7seg(Connecting)
  digitalWrite(ssegAPin, 0);
  digitalWrite(ssegBPin, 1);
  digitalWrite(ssegCPin, 0);
  digitalWrite(ssegDPin, 1);

  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  WebServer.begin();
  IPAddress myIP = WiFi.localIP();
  
  Serial.print("Server is UP and RUNNING: ");
  Serial.println(myIP);

  //Turn 7seg off
  digitalWrite(ssegAPin, 1);
  digitalWrite(ssegBPin, 1);
  digitalWrite(ssegCPin, 1);
  digitalWrite(ssegDPin, 1);

  //Display IP on 7-Segment
  displayIP(myIP);
}

void loop()
{
  // Check if a user has connected
  client = WebServer.available();
  if (!client)
    return;

  // Wait until the user sends some data
  while (!client.available())
    delay(1);

  // Read the first line of the request
  String request = client.readStringUntil('\r\n');
  client.flush();
  
  for(int i = 0 ; i < 11 ; i++)
  {
    sprintf(str, "/7seg=%d", i);
    if(request.indexOf(str) != -1)
    {
      digitalWrite(ssegAPin, numbers[i][3]);
      digitalWrite(ssegBPin, numbers[i][2]);
      digitalWrite(ssegCPin, numbers[i][1]);
      digitalWrite(ssegDPin, numbers[i][0]);
    }
  }

  if(request[4] == '/' && request[5] == 's' && request[6] == 'e' && request[7] == 'r' && request[8] == 'v' && request[9] == 'o' && request[10] == '=')
  {
    degree = (request[11] - 48) * 100 + (request[12] - 48) * 10 + (request[13] - 48);
    ourServo.attach(servoPin);
    for(int pos = 0; pos <= degree ; pos++)
    {
      ourServo.write(pos);
      delay(15);
    }
    ourServo.detach();
  }

  SHT1x sht15(shDATAPin, shSCKPin); //Data, SCK

  tempC = sht15.readTemperatureC();
  humidity = sht15.readHumidity();

  int light = analogRead(ldrPin);
  light = map(light, 0, 1023, 0, 100);
  
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html; charset=UTF-8");
  client.println("");
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
  client.println("<head>");
  client.println("<title>Exercise 1</title>");
  client.println("</head>");
  client.println("<body>");
  client.println("<center><a href=\"/\">UPDATE</a></center>");
  client.print("<center>Temperature : ");
  client.println(tempC);
  client.print(" 'C</br>Humidity: ");
  client.print(humidity);
  client.print(" %</br>Light: ");
  client.print(light, DEC);
  client.println(" %</center>");
  client.println("</br></br>7-SEGMENT: ");
  client.println("<a href = \"/7seg=0\">0</a>");
  client.println("<a href = \"/7seg=1\">1</a>");
  client.println("<a href = \"/7seg=2\">2</a>");
  client.println("<a href = \"/7seg=3\">3</a>");
  client.println("<a href = \"/7seg=4\">4</a>");
  client.println("<a href = \"/7seg=5\">5</a>");
  client.println("<a href = \"/7seg=6\">6</a>");
  client.println("<a href = \"/7seg=7\">7</a>");
  client.println("<a href = \"/7seg=8\">8</a>");
  client.println("<a href = \"/7seg=9\">9</a>");
  client.println("<a href = \"/7seg=10\">Off</a>");
  client.println("</br></br>Servo: ");
  client.println("<a href = \"/servo=000\">0</a>");
  client.println("<a href = \"/servo=030\">30</a>");
  client.println("<a href = \"/servo=045\">45</a>");
  client.println("<a href = \"/servo=060\">60</a>");
  client.println("<a href = \"/servo=090\">90</a>");
  client.println("<a href = \"/servo=099\">99</a>");
  client.println("<a href = \"/servo=179\">179</a>");
  client.println("</body>");
  client.println("</html>");
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
      digitalWrite(ssegAPin, numbers[digit1][3]);
      digitalWrite(ssegBPin, numbers[digit1][2]);
      digitalWrite(ssegCPin, numbers[digit1][1]);
      digitalWrite(ssegDPin, numbers[digit1][0]);

      delay(500);      
    }
    
    digitalWrite(ssegAPin, 1);
    digitalWrite(ssegBPin, 1);
    digitalWrite(ssegCPin, 1);
    digitalWrite(ssegDPin, 1);

    delay(100);
          
    if(digit2 != 0)
    {
      digitalWrite(ssegAPin, numbers[digit2][3]);
      digitalWrite(ssegBPin, numbers[digit2][2]);
      digitalWrite(ssegCPin, numbers[digit2][1]);
      digitalWrite(ssegDPin, numbers[digit2][0]);

      delay(500);      
    }

    digitalWrite(ssegAPin, 1);
    digitalWrite(ssegBPin, 1);
    digitalWrite(ssegCPin, 1);
    digitalWrite(ssegDPin, 1);

    delay(100);

    digitalWrite(ssegAPin, numbers[digit3][3]);
    digitalWrite(ssegBPin, numbers[digit3][2]);
    digitalWrite(ssegCPin, numbers[digit3][1]);
    digitalWrite(ssegDPin, numbers[digit3][0]);

    delay(500);      

    digitalWrite(ssegAPin, 1);
    digitalWrite(ssegBPin, 1);
    digitalWrite(ssegCPin, 1);
    digitalWrite(ssegDPin, 1);

    delay(100);

    if(i != 3)
    {
      digitalWrite(ssegDotPin, 0);
      delay(250);
      digitalWrite(ssegDotPin, 1);
    }
  }
}
