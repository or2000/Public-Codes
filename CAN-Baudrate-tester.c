/* 
Code to check the bus baudrate. The following baudrates are tested: 5KBPS, 10KBPS, 20KBPS, 40KBPS, 50KBPS, 80KBPS, 100KBPS, 125KBPS, 200KBPS, 250KBPS, 500KBPS, 1000KBPS.
The following code is tested on ESP32 connected to an MCP2515
*/

#include <mcp_can.h>
#include <SPI.h>

long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];

#define DELAY 1000
#define CAN0_INT 21
MCP_CAN CAN0(5);

INT8U tryReading() {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);
    if ((rxId & 0x80000000) == 0x800000000)
        sprintf(msgString, "Extended ID: 0x%.8lX DLC: %1d Data:", (rxId & 0x1FFFFFFF),len);
    else    
        sprintf(msgString, "Standard ID: 0x%.3lX DLC: %1d Data:",rxId,len);

    Serial.println (msgString);

    if((rxId & 0x40000000) == 0x40000000){    // Determine if message is a remote request frame.
      sprintf(msgString, " REMOTE REQUEST FRAME");
      Serial.print(msgString);
      return CAN_NOMSG;
    } else {
      Serial.print("Non RRF message");
      for(byte i = 0; i<len; i++){
        sprintf(msgString, " 0x%.2X", rxBuf[i]);
        Serial.print(msgString);
        return CAN_OK;
      }
    }
        
    Serial.println();
    
}

void setup()
{
    Serial.begin(115200);

    delay(2000);
    Serial.println("CAN Receiver\n");
    INT8U baudRates[12] = {CAN_5KBPS, CAN_10KBPS, CAN_20KBPS, CAN_40KBPS, CAN_50KBPS, CAN_80KBPS, CAN_100KBPS, CAN_125KBPS, CAN_200KBPS, CAN_250KBPS, CAN_500KBPS, CAN_1000KBPS};

    for (;;) {
      for (int i = 0;i<12;i++) {
          unsigned long startTime = millis();
          //Initializing MCP2515 running at 16 MHz with a baudrate of 500kb/s. Masks and filters are disabled.
          sprintf(msgString,"Trying to intialize CAN on %u.",baudRates[i]);
          Serial.println(msgString);
          if(CAN0.begin(MCP_ANY, baudRates[i], MCP_16MHZ) == CAN_OK)
            Serial.println("MCP2515 Initialized Successfully!");
          else
            Serial.println("Error Initializing MCP2515...");

          CAN0.setMode(MCP_NORMAL); //Set operation mode to normal so MCP2515 sends acks to received data
          pinMode(CAN0_INT, INPUT); //Configuring pin for /INT input
          Serial.println("Starting listening");
          for (;;) {
              if (!digitalRead(CAN0_INT)) {
                  Serial.println("Received a message"); 
                  if ((tryReading()) == CAN_OK) return;
                  if ((millis() - startTime) > DELAY) {sprintf(msgString,"Did not receive any data message\n");  Serial.println(msgString); break; }
              } else {    
                  if ((millis() - startTime) > DELAY) {sprintf(msgString,"Did not receive any data message\n");  Serial.println(msgString); break; }
              }
          }
      }
    }

}

void loop()
{
    if (!digitalRead(CAN0_INT)) tryReading();
}

