//Copyright Royce Pipkins 2010
//May be used under the terms of the GPL V3 or higher. http://www.gnu.org/licenses/gpl.html
#include <avr/wdt.h>
#include <NewSoftSerial.h>
#include <ASCIIProtocol.h>




const uint16_t LOCK_TIME = 14000; //the lock is open for seven secs follow admission
const uint8_t LOCK_PIN = 7;
const uint8_t DEBUG_PIN = 8;

const uint8_t SERVER_ADDRESS = 0;
uint8_t ADDRESS = 0;
const uint8_t BROADCAST_ADDRESS = 255;
const uint8_t EXT_ADDRESS = 1;

const uint8_t SSRX = 18;
const uint8_t SSTX = 19;
const uint8_t EXT_TIMEOUT = 100;

const uint8_t svrFlowPin = 2;


const uint8_t ID_CHECK = 1;
const uint8_t ACCESS_REQ = 2;
const uint8_t NO_ACTIVITY = 3;
const uint8_t ACCESS_GRANTED = 4;
const uint8_t ACCESS_DENIED = 5;
const uint8_t SET_IDLE = 6;
const uint8_t ACKNOWLEDGE = 7;



NewSoftSerial nss(4, 3, false);  //RX, TX
ASCIIProtocol svrCom(Serial);
ASCIIProtocol extCom(nss);



void setup()
{

  pinMode(LOCK_PIN, OUTPUT);    //set the door strike pin to output mode
  digitalWrite(LOCK_PIN, LOW);  //turn it off (locked)
  
  pinMode(DEBUG_PIN, OUTPUT);    
  digitalWrite(DEBUG_PIN, LOW);  
  
  pinMode(svrFlowPin, OUTPUT);
  digitalWrite(svrFlowPin, LOW); 
  
  
  nss.begin(9600);
  Serial.begin(9600);

  //The pins are still inputs, so this turns on the pull-up resistors
  digitalWrite(14, HIGH);
  digitalWrite(15, HIGH);
  digitalWrite(16, HIGH);
  digitalWrite(17, HIGH);
  digitalWrite(18, HIGH);
  digitalWrite(19, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  
  //read the dip switch values
  ADDRESS |= digitalRead(14);
  ADDRESS |= (digitalRead(15) << 1);  
  ADDRESS |= (digitalRead(16) << 2);    
  ADDRESS |= (digitalRead(17) << 3);  
  ADDRESS |= (digitalRead(18) << 4);  
  ADDRESS |= (digitalRead(19) << 5);  
  ADDRESS |= (digitalRead(5) << 6);  
  ADDRESS |= (digitalRead(6) << 7);  
  ADDRESS = ~ADDRESS;
   
  Serial.println("****************************************");  
  Serial.println("Milwaukee Makerspace Access Control v1.1");
  Serial.println("****************************************");
  Serial.print("Device Address = 0x");
  Serial.println(ADDRESS, HEX);
  
  wdt_enable(WDTO_2S);
}

bool waitForExtReply()
{
  uint8_t data;
  unsigned long timeout = millis() + EXT_TIMEOUT; 
  
  extCom.erasePkt();
  while(!extCom.isValidPacket())
  {
     while (nss.available()) 
     {
       data = nss.read();
       extCom.recv(data);       
     }
     
     if (millis() > timeout) break;
  }
  
  return (extCom.isValidPacket());
}

//encapsulated the process of setting the flow direction pin
void xmitToSvr(uint8_t type, uint8_t* body)
{
  digitalWrite(svrFlowPin, HIGH);
  svrCom.send(SERVER_ADDRESS, type, extCom.getBody());  
  delay(5); //add a couple milliseconds for the last char to shift out
  digitalWrite(svrFlowPin, LOW);
}

void respondToPoll()
{
  int idx = 0;
  uint8_t data;

  //query the external unit for a possible ID
  extCom.send(EXT_ADDRESS, ID_CHECK, (const unsigned char*)"ID_CHECK");
  
  waitForExtReply();

  if (extCom.isValidPacket())
  {
    delay(5);
    if (extCom.getType() == ACCESS_REQ)
    { 
      xmitToSvr(ACCESS_REQ, extCom.getBody());
    }
    else
    {
      xmitToSvr(NO_ACTIVITY, (uint8_t*)"NO_ACTIVITY");
    }
  }
}

void DoLockDelay()
{
  int lt = LOCK_TIME;
  while(lt > 0)
  {
    delay(500);
    lt -= 500;
    wdt_reset();
  }  
}

//expecting 33 bytes, 2x16 lines with null terminator
void grantAccess(unsigned char* data)
{
  //pass the message on to the exterior unit.
  extCom.send(EXT_ADDRESS, ACCESS_GRANTED, data);
  
  if (waitForExtReply())
  {
    if (extCom.getType() == ACKNOWLEDGE)
    {
      delay(5);
      acknowledge();
      //open the lock for a period of time
      digitalWrite(LOCK_PIN, HIGH);
      DoLockDelay();
      //delay(LOCK_TIME);
      digitalWrite(LOCK_PIN, LOW);
    }
  }
}

//expecting 33 bytes, 2x16 lines with null terminator
void denyAccess(unsigned char* data)
{
  //pass the message on to the exterior unit.
  extCom.send(EXT_ADDRESS, ACCESS_DENIED, data);
  
  if (waitForExtReply())
  {
    if (extCom.getType() == ACKNOWLEDGE)
    {
      delay(5);
      acknowledge();
    }
  }
}

//expecting 33 bytes, 2x16 lines with null terminator
void setIdleMsg(unsigned char* data)
{
  //pass the message on to the exterior unit.
  extCom.send(EXT_ADDRESS, SET_IDLE, data);
  
}

void acknowledge()
{
  xmitToSvr(ACKNOWLEDGE, (uint8_t*)"ACKNOWLEDGE");
}

//debuggin loop
/*void loop()
{
 //TODO send a stream of ID_CHECKS
 
 respondToPoll();
 //delay(5000);
 

}*/

void loop()
{
  //TODO wait for server poll, upon reciept:
  //issue poll to exterior unit and wait for response
  //pass repsonse on to server.
  //wait for server response
  //possibly run door strike
  //pass messages on to exterior unit (possibly)
  uint8_t addr;
  
  while(Serial.available() > 0)
  { 
    svrCom.recv(Serial.read());
  }
  
  //did we get a complete packet?
  if (svrCom.isValidPacket())
  {
    addr = svrCom.getAddress();
    //was that packet addressed to us or was it a global broadcast?
    if (ADDRESS == addr || BROADCAST_ADDRESS == addr)
    {
      //switch which action is taken based on the type of packet recieved
      switch(svrCom.getType())
      {
       case ID_CHECK:
         respondToPoll();
         break;         
       case ACCESS_GRANTED:
         grantAccess(svrCom.getBody());
         break;
       case ACCESS_DENIED:
         //TODO set the msg on the screen
         denyAccess(svrCom.getBody());
         break;
       case SET_IDLE:
         //TODO send the msg to the screen
         setIdleMsg(svrCom.getBody());
         break;
      }
    }
    svrCom.erasePkt();
  }
  
  wdt_reset();
}
