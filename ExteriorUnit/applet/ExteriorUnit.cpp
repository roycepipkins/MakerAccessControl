// include the library code:
#include <LiquidCrystal.h>
#include <NewSoftSerial.h>
#include "LCDMngr.h"


#define ROW1 13
#define ROW2 14
#define ROW3 15
#define ROW4 16
#define COL1 17
#define COL2 18
#define COL3 19

#define STX 2
#define ETX 3

#define idle_msg1 "  Welcome to"
#define idle_msg2 "  Bucketworks!"

#include "WProgram.h"
void Buzz(bool state);
char readKeyPad();
bool is_good_rfid_packet();
void rfid_packet_recv(char data);
void processRFID();
void processKeypad();
void iu_packet_recv(char data);
bool is_valid_iu_packet();
void reset_iu_packet();
void processInteriorUnit();
void setup();
void loop();
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

NewSoftSerial rfid(8, 7, false); 

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
          clear();
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
  
  while (rfid.available() > 0)
  {
    rfid_packet_recv(rfid.read());
  }
  
  if (is_good_rfid_packet())
  {
   
   Buzz(true);
   delay(50);
   Buzz(false); 
   strncpy(id_for_verification, rfid_packet.fields.id, 10);
   id_for_verification[10] = 0;
   lcd.clear();
   lcd.startIdleTimer(10000);
   lcd.print(id_for_verification);
   
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
         lcd.startIdleTimer(10000);         
       }
       start_time = millis(); //mark the time of the key press
       lcd.startIdleTimer(10000); 
       lcd.print(data); 
       if (strlen(keyBuffer) < sizeof(11))
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
         lcd.startIdleTimer(10000); 
       }
     } 
  }
  else
  {
    if (millis() - start_time > 10000 && start_time > 0)
    {
      start_time = 0;
      keyBuffer[0] = 0;
      keyBuffer[11] = 0;
    }    
  }
  
  
}

struct IU_Packet_Fields
{
   char stx;
   char type;
   char line1[17];
   char line2[17];
   char etx; 
};

union IU_Packet
{
  IU_Packet_Fields fields;
  char data[37];  
};

IU_Packet iu_packet;
char iu_packet_end = 0;

void iu_packet_recv(char data)
{
  
  if (iu_packet_end == 0)
  {
    //an STX is required to start the packet
    if (data == STX)
    {
      iu_packet.fields.stx = data;
      iu_packet.fields.etx = 0;
      iu_packet_end++;
    }  
  }
  else
  {
    
    if (iu_packet_end < 37)
    {
      iu_packet.data[iu_packet_end] = data;
      iu_packet_end++;
    }
    else
      iu_packet_end = 0;    
  }
}

bool is_valid_iu_packet()
{
  return (iu_packet.fields.stx == STX && iu_packet.fields.etx == ETX);  
}

void reset_iu_packet()
{
  iu_packet.fields.stx = 0;
  iu_packet_end = 0;
}

#define ID_CHECK 1
#define ACCESS_REQ 2
#define NO_ACTIVITY 3
#define ACCESS_GRANTED 4
#define ACCESS_DENIED 5
#define SET_IDLE_MSG 6

void processInteriorUnit()
{
  while (Serial.available() > 0)
  {
    iu_packet_recv(Serial.read());
  }

  if (is_valid_iu_packet())
  {
    switch(iu_packet.fields.type)
    {
    case ID_CHECK:
      //the lines are not populated for an id check
      Serial.print(STX);
      if (strlen(id_for_verification))
      {
        Serial.print(ACCESS_REQ);
        Serial.print(id_for_verification);
        Serial.print(0);
        reset_iu_packet();
      }
      else
      {
        Serial.print(NO_ACTIVITY);
      }
      Serial.print(ETX);
      break;
    case ACCESS_GRANTED:
    case ACCESS_DENIED:
      iu_packet.fields.line1[16] = 0;
      iu_packet.fields.line2[16] = 0;
      lcd.clear();
      lcd.print(iu_packet.fields.line1);
      lcd.setCursor(0,1);
      lcd.print(iu_packet.fields.line2);
      lcd.startIdleTimer(10000);
      break;
    case SET_IDLE_MSG:
      lcd.setIdleMsg(iu_packet.fields.line1, iu_packet.fields.line2);
      lcd.startIdleTimer(0);
      break; 
    default:
      reset_iu_packet();
      break;
    }
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
  
  rfid.begin(9600);    //connection to ID-12 RFID reader module
  Serial.begin(9600);  //connection to interior unit
  
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


int main(void)
{
	init();

	setup();
    
	for (;;)
		loop();
        
	return 0;
}

