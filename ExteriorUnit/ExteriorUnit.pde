//Copyright Royce Pipkins 2010
//May be used under the terms of the GPL V3 or higher. http://www.gnu.org/licenses/gpl.html
#include "string.h"
#include <LiquidCrystal.h>
#include <NewSoftSerial.h>
#include <ASCIIProtocol.h>

#define INT_ADDRESS 1

#define ROW1 13
#define ROW2 14
#define ROW3 15
#define ROW4 16
#define COL1 17
#define COL2 18
#define COL3 19

#define STX 2
#define ETX 3

#define ID_CHECK 1
#define ACCESS_REQ 2
#define NO_ACTIVITY 3
#define ACCESS_GRANTED 4
#define ACCESS_DENIED 5
#define SET_IDLE 6
#define ACKNOWLEDGE 7

#define IDLETIME 5000
                 //0123456789ABCDEF
#define idle_msg1 "   Welcome to"
#define idle_msg2 "   Bucketworks"

char keys[4][3] =
{
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};

struct RFID_fields
{
 char packet_end; 
 char stx;
 char id[10];
 char checksum[2];
 char crlf[2];
 char etx; 
};

union RFID_packet
{
 char packet_data[17];  
 RFID_fields fields; 
};

RFID_packet rfid_packet;

char keyBuffer[12];

char id_for_verification[11];


// initialize the library with the numbers of the interface pins

NewSoftSerial intUnitSerial(8, 7, false); 
ASCIIProtocol intCom(intUnitSerial);

class LCDMngr2x16 : public LiquidCrystal
{
  private:
    char idle1[17];
    char idle2[17];
    
    unsigned long idle_time;
    bool idle_timer_active;
    
  public:
    LCDMngr2x16(uint8_t rs, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)  : 
          LiquidCrystal(rs, enable, d0, d1, d2, d3),
          idle_timer_active(false) 
    {
        idle1[0] = 0;
        idle2[0] = 0;
    };
    
  
    void begin(uint8_t cols , uint8_t rows, uint8_t charsize = LCD_5x8DOTS)
    {
      LiquidCrystal::begin(16, 2);
    };
  
    void setIdleMsg(char* line1, char* line2)
    {
      strncpy(idle1, line1, 16);
      strncpy(idle2, line2, 16);
      idle1[16] = 0;
      idle2[16] = 0;
    };
    
    void startIdleTimer(unsigned long ms)
    {
      idle_time = millis() + ms;
      idle_timer_active = true;
      
    };
    
    void processLCD()
    {
      if (idle_timer_active)
      {
        if (millis() > idle_time) //expires early in rare wrap sitution. not a big deal.
        {
          idle_timer_active = false;
          LiquidCrystal::begin(16, 2);
          print(idle1);
          setCursor(0,1); 
          print(idle2);
        }  
      }
    };
      
  
};

LCDMngr2x16 lcd(12, 11, 5, 4, 3, 2);


void Buzz(bool state)
{
  if (state)
  {
    TCCR1B = _BV(CS11) | _BV(CS11);   //Set clock to div 64
  } 
  else
  {
     TCCR1B = 0;  //Clock off
  }
}

char readKeyPad()
{
  int idx;
  static bool keyDown = false;
  //set rows to input and activate pull up resistors
  pinMode(ROW1, INPUT);
  pinMode(ROW2, INPUT);
  pinMode(ROW3, INPUT);
  pinMode(ROW4, INPUT);
  digitalWrite(ROW1, HIGH);
  digitalWrite(ROW2, HIGH);
  digitalWrite(ROW3, HIGH);
  digitalWrite(ROW4, HIGH);
  
  
  //set columns to output and set output to active low
  pinMode(COL1, OUTPUT);
  pinMode(COL2, OUTPUT);
  pinMode(COL3, OUTPUT);
  digitalWrite(COL1, LOW);
  digitalWrite(COL2, LOW);
  digitalWrite(COL3, LOW);
  
  
  
  int col[3], row[4];
  
  //delay(1);
  //read the row states
  row[0] = digitalRead(ROW1);
  row[1] = digitalRead(ROW2);
  row[2] = digitalRead(ROW3);
  row[3] = digitalRead(ROW4);
  
  
  //set columns to output and set output to active low
  pinMode(COL1, INPUT);
  pinMode(COL2, INPUT);
  pinMode(COL3, INPUT);
  digitalWrite(COL1, HIGH);
  digitalWrite(COL2, HIGH);
  digitalWrite(COL3, HIGH);
  
  //set rows to input and activate pull up resistors
  pinMode(ROW1, OUTPUT);
  pinMode(ROW2, OUTPUT);
  pinMode(ROW3, OUTPUT);
  pinMode(ROW4, OUTPUT);
  digitalWrite(ROW1, LOW);
  digitalWrite(ROW2, LOW);
  digitalWrite(ROW3, LOW);
  digitalWrite(ROW4, LOW);
  
  //delay(1);
  col[0] = digitalRead(COL1);
  col[1] = digitalRead(COL2);
  col[2] = digitalRead(COL3);
  
  int row_idx = -1, col_idx = -1;
  for(idx = 0; idx < 4; idx++)
  {
     if (row[idx] == 0)
    {
       row_idx = idx;
       break;
    } 
  }
  
  for(idx = 0; idx < 3; idx++)
  {
    if (col[idx] == 0)
    {
       col_idx = idx;
       break;
    } 
  }
  
  
  if (row_idx >= 0 && col_idx >= 0)
  {
    if (!keyDown)
    {
      keyDown = true;
      Buzz(true);
      return keys[row_idx][col_idx];   
    }    
    else return 0;
  }
  
  Buzz(false);
  keyDown = false;
  return ' ';
}



bool is_good_rfid_packet()
{
  if (rfid_packet.fields.stx == STX && rfid_packet.fields.etx == ETX)
  {
    //TODO perform checksum on ID field.
    return (true);
  }  
  return (false);
}

void rfid_packet_recv(char data)
{
  char idx;
  //lcd.print(data);
  if (data == STX)
  {
    for(idx = 0; idx < sizeof(RFID_packet); idx++)
    {
     rfid_packet.packet_data[idx] = 0;
    }
    rfid_packet.fields.packet_end++;
    rfid_packet.fields.stx = STX;
    
  }
  else if (data == ETX)
  {
    rfid_packet.fields.packet_end++;
    rfid_packet.fields.etx = ETX;
  }
  else if (rfid_packet.fields.stx == STX && rfid_packet.fields.etx != ETX)
  {
    rfid_packet.fields.packet_end++;
    if (rfid_packet.fields.packet_end >=  sizeof(RFID_packet))
    {
     for(idx = 0; idx < sizeof(RFID_packet); idx++)
     {
      rfid_packet.packet_data[idx] = 0;
     } 
    }
    else
     rfid_packet.packet_data[rfid_packet.fields.packet_end] = data;  
  }
  else
  {
    //data is garbage
  }
}

void processRFID()
{
  char data;
  
  while (Serial.available() > 0)
  {
    rfid_packet_recv(Serial.read());
  }
  
  if (is_good_rfid_packet())
  {
   
   Buzz(true);
   delay(50);
   Buzz(false); 
   strncpy(id_for_verification, rfid_packet.fields.id, 10);
   id_for_verification[10] = 0;
   lcd.clear();
   lcd.print("Checking...");
   lcd.setCursor(0,1);
   lcd.startIdleTimer(IDLETIME); 
   
   rfid_packet.fields.stx = 0;
  }  
}

void processKeypad()
{
  static unsigned long start_time = 0;
  char data[2];
  data[0] = readKeyPad();
  data[1] = 0;
  
  
  if (data[0] != ' ') //' ' means no key press
  {
     if (data[0] != 0) // 0 means the previous keypress hasn't been released
     {
       if (start_time == 0) //clear the screen on the first press
       {
         keyBuffer[0] = 0;
         keyBuffer[11] = 0;
         lcd.clear();
         lcd.print("Press # at end.");
         lcd.setCursor(0,1);
         lcd.startIdleTimer(IDLETIME);         
       }
       start_time = millis(); //mark the time of the key press
       lcd.startIdleTimer(IDLETIME); 
       //lcd.print(data);  
       lcd.print('*');     //don't print the passcode itself
       if (strlen(keyBuffer) < 10)
         strcat(keyBuffer, data);
       
       if (data[0] == '#') //'#' means submit id code for access
       {
         //transfer buffer to await next poll
         strcpy(id_for_verification, keyBuffer);  
         keyBuffer[0] = 0;
         keyBuffer[11] = 0;
         lcd.clear();
         lcd.print("Checking...");
         lcd.setCursor(0,1);
         lcd.startIdleTimer(IDLETIME); 
       }
     } 
  }
  else
  {
    if (millis() - start_time > IDLETIME && start_time > 0)
    {
      start_time = 0;
      keyBuffer[0] = 0;
      keyBuffer[11] = 0;
    }    
  }  
}



void processInteriorUnit()
{
  char line1[17], line2[17];
  uint8_t data;
  while (intUnitSerial.available() > 0)
  {
    
    data = intUnitSerial.read();
    //lcd.print(data);  
    intCom.recv(data);
  }

  if (data == 3)
  {
    //lcd.clear();
    //lcd.print(intCom.getChecksum());  
  }

  if (intCom.isValidPacket())
  {
    switch(intCom.getType())
    {
    case ID_CHECK:
      
      if (strlen(id_for_verification))
      {
        intCom.send(INT_ADDRESS, ACCESS_REQ, (uint8_t*)id_for_verification);
        intCom.erasePkt();
      }
      else
      {
        intCom.send(INT_ADDRESS, NO_ACTIVITY, (const unsigned char*)"NO_ACTIVITY");
        intCom.erasePkt();
      }
      break;
    case ACCESS_GRANTED:
      strncpy(line1, (char*)(intCom.getBody()), 16);
      line1[16] = 0;
      strncpy(line2, (char*)(intCom.getBody() + 16), 16);
      line2[16] = 0;
      
      lcd.clear();
      lcd.print(line1);
      lcd.setCursor(0,1);
      lcd.print(line2);
      lcd.startIdleTimer(IDLETIME);
      id_for_verification[0] = 0;
      intCom.send(INT_ADDRESS, ACKNOWLEDGE, (const unsigned char*)"ACKNOWLEDGE");
    case ACCESS_DENIED:
      strncpy(line1, (char*)(intCom.getBody()), 16);
      line1[16] = 0;
      strncpy(line2, (char*)(intCom.getBody() + 16), 16);
      line2[16] = 0;
      
      lcd.clear();
      lcd.print(line1);
      lcd.setCursor(0,1);
      lcd.print(line2);
      lcd.startIdleTimer(4500);
      id_for_verification[0] = 0;
      intCom.send(INT_ADDRESS, ACKNOWLEDGE, (const unsigned char*)"ACKNOWLEDGE");
      break;
    case SET_IDLE:
      strncpy(line1, (char*)(intCom.getBody()), 16);
      line1[16] = 0;
      strncpy(line2, (char*)(intCom.getBody() + 16), 16);
      line2[16] = 0;
      lcd.setIdleMsg(line1, line2);
      lcd.startIdleTimer(0);
      break; 
    default:
      break;
    }
    intCom.erasePkt();
  }  
}

void setup() 
{  
  lcd.begin(16, 2);
  lcd.setIdleMsg(idle_msg1, idle_msg2);
  lcd.startIdleTimer(0);
  
  TCCR1A = _BV(COM1B1) | _BV(COM1B0) | _BV(WGM10); //Set on match, run in 8-bit PWM mode
  TCCR1B = 0; //Clock off for now
  OCR1B = 128; //50% duty cycle
  pinMode(10, OUTPUT); //Set OC1B to output
  
  Serial.begin(9600);    //connection to ID-12 RFID reader module
  intUnitSerial.begin(9600);  //connection to interior unit
  intUnitSerial.println("****************************************");  
  intUnitSerial.println("Milwaukee Makerspace Access Control v1.1");
  intUnitSerial.println("****************************************");
  
  int idx;
  for(idx = 0; idx < sizeof(RFID_packet); idx++)
  {
   rfid_packet.packet_data[idx] = 0;
  }
}

void loop() 
{
  processRFID();
  processKeypad();
  processInteriorUnit();
  lcd.processLCD();
}

