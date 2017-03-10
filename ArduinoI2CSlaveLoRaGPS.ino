#include <Wire.h>

unsigned char ResultLength, Result[1];
char *ptr = 0;
char NMEA[80];
char LoRa[256];
char Other[80];
int SendingNow, NMEAActive, LoRaActive;
char Command;
unsigned long Timeout=0L;
int counter;

void setup()
{
  int i;

  counter = 0;
  NMEAActive = 0;
  LoRaActive = 0;
  SendingNow = 0;
  // SendingMode = 0;
  
  Serial.begin(9600);           // start serial for GPS
  Serial.println("Arduino PaperChase i2c Slave");

  SetupGPS();

  SetupLoRa();
  
  Wire.begin(8);                // join i2c bus with address #8
  Wire.onReceive(receiveEvent); // register event
  Wire.onRequest(sendEvent);   // register event
}

void loop()
{
  CheckGPS();

  CheckLoRa();
  
  if (SendingNow == 0)
  {
    // Priority to LoRa packets
    if (LoRaActive)
    {
      LoRaActive = 0;
      ptr = LoRa;
    }
    else if (NMEAActive)
    {
      NMEAActive = 0;
      *NMEA = '!';
      ptr = NMEA;
    }
    else if (millis() >= Timeout)
    {
      sprintf(Other, "^%d\n", LoRaRSSI());
      ptr = Other;
      Timeout = millis() + 500L;
    }
  }
  
  if (Command)
  {
    Serial.print("Command = ");
    Serial.println(Command);
    Command = 0;
  }
}

void receiveEvent(int bytecount)
{
  while (Wire.available()) 
  {
    unsigned char c, Channel, Pin;
    unsigned int Value;
   
    c = Wire.read(); // receive byte as a character
    Command = c;
  } 
}

void sendEvent(void)
{
  if (ptr)
  {
    Result[0] = *ptr;
    ResultLength = 1;

    ptr++;

    if (*ptr == '\0')
    {
      ptr = 0;
    }

    Wire.write(Result, ResultLength);

  }
  SendingNow = ptr != 0;
}
