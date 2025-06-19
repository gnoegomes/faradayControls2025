#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg;
MCP2515 mcp2515(10); // CS pin connected to D10

char receivedMsg[65]; // Buffer to store the received message
uint8_t receivedBytes = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); // Set CAN speed to 500kbps
  mcp2515.setNormalMode();

  Serial.println("MCP2515 Initialized Successfully!");
}

void loop() {
  if (mcp2515.readMessage(&canMsg) == MCP2515::ERROR_OK) {
    memcpy(receivedMsg + receivedBytes, canMsg.data, canMsg.can_dlc);
    receivedBytes += canMsg.can_dlc;

    if (receivedBytes >= 64) {
      receivedMsg[64] = '\0'; // Null-terminate the string

      Serial.print("Received message: ");
      Serial.println(receivedMsg);

      receivedBytes = 0; // Reset for the next message
    }
  }
}
