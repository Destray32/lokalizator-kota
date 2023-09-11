#include <SoftwareSerial.h>
#include <TinyGPS++.h>

#define RST 4

SoftwareSerial gpsSerial(9,8);
// Create a TinyGPS++ object
TinyGPSPlus gps;

//Create software serial object to communicate with SIM800L
SoftwareSerial simSerial(3, 2); //SIM800L Tx & Rx is connected to Arduino #3 & #2

// Ciąg znaków do porównania z treścią wiadomości SMS
String command = "pozycja";

bool connected = false;
bool findCords = false;


void setup() {
  //Begin serial communication with Arduino and Arduino IDE (Serial Monitor)
  Serial.begin(9600);
  

  // Begin serial communication with gps module
  gpsSerial.begin(9600);


  //Begin serial communication with Arduino and SIM800L
  simSerial.begin(9600);




  // Set the reset pin as an output and initialize it to HIGH
  pinMode(RST, OUTPUT);
  digitalWrite(RST, HIGH);

  Serial.println("Initializing...");
  delay(1000);

  simSerial.println("AT+CMGF=1"); // Configuring TEXT mode
  updateSerial();
  simSerial.println("AT+CNMI=1,2,0,0,0"); // Decides how newly arrived SMS messages should be handled
  updateSerial();

}

void loop() {
  if(!findCords)
  {
    simSerial.listen();
    updateSerial();
  }
  else
  {
    gpsSerial.listen();
    //delay(5000);
    while (gpsSerial.available() > 0)
      if (gps.encode(gpsSerial.read()))
        {
          displayInfo();
          break;
        }
    //findCords = false;
    //delay(2000);

  }
}

void updateSerial()
{
  delay(500);
  while (Serial.available()) 
  {
    simSerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(simSerial.available()) 
  {
    String data = simSerial.readString();
    
    if (data.indexOf("+CMT:") >= 0)
    {

      // Wyodrębnij treść wiadomości SMS z danych
      int start_index = data.indexOf('/', data.indexOf('/', data.indexOf('/') + 1 ) + 1) + 1;
      int end_index = data.lastIndexOf("/");
      String message = data.substring(start_index, end_index);
      // Porównaj treść wiadomości SMS z zdefiniowanym ciągiem znaków
      if(message == command)
      {
        // Wyświetl informację o tym, że wiadomość zawiera komendę "/pozycja"
        Serial.println("Wiadomość zawiera komendę /pozycja");
        findCords = true;
      }
      else 
      {
      // Wyświetl treść wiadomości SMS
        Serial.println(message);
        findCords = false;
      } 
    }
  }
}

void sendSMS(String text)
{
  Serial.print("Wysylam SMS: ");
  Serial.println(text);
  simSerial.println("AT+CMGS=\"+48516574688\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
  updateSerial();
  simSerial.print(text); //text content
  updateSerial();
  simSerial.write(26);
}


void displayInfo()
{
  Serial.println("Jestesmy w funkcji displayInfo");
  String output;
  float lat = 0.000000, lng = 0.000000;
  if (gps.location.isValid())
  {
    lat = gps.location.lat();
    lng = gps.location.lng();
  }
  else
  {
    output = "error";
  }
  simSerial.listen();
  
  delay(1000);

  String outLat(lat, 6);
  String outLng(lng, 6);
  output = ("https://www.google.com/maps/place/" + outLat + "," + outLng);

  
  if (output != "error")
  {
    findCords = false;
    simSerial.listen();

    delay(2000);
    sendSMS(output);
  }

  gpsSerial.listen();

}
