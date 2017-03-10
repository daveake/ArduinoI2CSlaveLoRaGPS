/* ========================================================================== */
/*   gps.ino                                                                  */
/*                                                                            */
/*   Serial code for ublox on AVR                                             */
/*                                                                            */
/*                                                                            */
/*                                                                            */
/* ========================================================================== */

// Globals
byte RequiredFlightMode=0;
byte GlonassMode=0;
byte RequiredPowerMode=-1;
byte LastCommand1=0;
byte LastCommand2=0;
byte HaveHadALock=0;

char Hex(char Character)
{
  char HexTable[] = "0123456789ABCDEF";
  
  return HexTable[Character];
}

void FixUBXChecksum(unsigned char *Message, int Length)
{ 
  int i;
  unsigned char CK_A, CK_B;
  
  CK_A = 0;
  CK_B = 0;

  for (i=2; i<(Length-2); i++)
  {
    CK_A = CK_A + Message[i];
    CK_B = CK_B + CK_A;
  }
  
  Message[Length-2] = CK_A;
  Message[Length-1] = CK_B;
}

void SendUBX(unsigned char *Message, int Length)
{
  LastCommand1 = Message[2];
  LastCommand2 = Message[3];
  
  int i;
  
  for (i=0; i<Length; i++)
  {
    Serial.write(Message[i]);
  }
}


void DisableNMEAProtocol(unsigned char Protocol)
{
  unsigned char Disable[] = { 0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
  
  Disable[7] = Protocol;
  
  FixUBXChecksum(Disable, sizeof(Disable));
  
  SendUBX(Disable, sizeof(Disable));
  
  // Serial.print("Disable NMEA "); Serial.println(Protocol);
}

void ProcessNMEA(char *Buffer, int Count)
{ 
    if (strncmp(Buffer+3, "GGA", 3) == 0)
    {
      Serial.print((char *)Buffer+1);
      strcpy(NMEA, Buffer);
      NMEAActive = 1;
      // DisableNMEAProtocol(0);      
    }
    else if (strncmp((char *)Buffer+3, "RMC", 3) == 0)
    {
      DisableNMEAProtocol(4);
    }
    else if (strncmp((char *)Buffer+3, "GSV", 3) == 0)
    {
      DisableNMEAProtocol(3);
    }
    else if (strncmp((char *)Buffer+3, "GLL", 3) == 0)
    {
      DisableNMEAProtocol(1);
    }
    else if (strncmp((char *)Buffer+3, "GSA", 3) == 0)
    {
      DisableNMEAProtocol(2);
    }
    else if (strncmp((char *)Buffer+3, "VTG", 3) == 0)
    {
      DisableNMEAProtocol(5);
    }
}


void SetupGPS(void)
{
}

int GPSAvailable(void)
{
  return Serial.available();
}


char ReadGPS(void)
{
  return Serial.read();
}

void PollGPSTime(void)
{
  uint8_t request[] = {0xB5, 0x62, 0x01, 0x21, 0x00, 0x00, 0x22, 0x67};
  SendUBX(request, sizeof(request));
}

void PollGPSLock(void)
{
  uint8_t request[] = {0xB5, 0x62, 0x01, 0x06, 0x00, 0x00, 0x07, 0x16};
  SendUBX(request, sizeof(request));
}

void PollGPSPosition(void)
{
  uint8_t request[] = {0xB5, 0x62, 0x01, 0x02, 0x00, 0x00, 0x03, 0x0A};
  SendUBX(request, sizeof(request));
}
  
void CheckGPS(void)
{
  static char Line[80];
  static int Length=0;
  unsigned char Character, Bytes, i;
  
  do
  {
    Bytes = GPSAvailable();
  
    for (i=0; i<Bytes; i++)
    { 
      Character = ReadGPS();
    
      if (Character == '$')
      {
        Line[0] = Character;
        Length = 1;
      }
      else if (Length >= (sizeof(Line)-2))
      {
        Length = 0;
      }
      else if (Length > 0)
      {
        if (Character != '\r')
        {
          Line[Length++] = Character;
          if (Character == '\n')
          {
            Line[Length] = '\0';
            ProcessNMEA(Line, Length);
            Length = 0;
          }
        }
      }
    }
  } while (Bytes > 0);
}


