#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg;
MCP2515 mcp2515(10); // CS pin connected to D10

void setup() {
  Serial.begin(115200);
  while (!Serial);

  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ); // Set CAN speed to 500kbps
  mcp2515.setNormalMode();

  Serial.println("MCP2515 Initialized Successfully!");
}

void loop() {
  const char* msg = "This is a 64-byte message that will be split into eight 8-byte CAN frames.";
  uint8_t msgLength = 64;
  uint8_t frames = (msgLength + 7) / 8; // Calculate number of frames needed

  for (uint8_t i = 0; i < frames; i++) {
    canMsg.can_id  = 0x036; // CAN ID
    canMsg.can_dlc = min(8, msgLength - i * 8); // Data length for this frame
    memcpy(canMsg.data, msg + i * 8, canMsg.can_dlc);

    mcp2515.sendMessage(&canMsg); // Send the message

    Serial.print("Sent frame ");
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.write(canMsg.data, canMsg.can_dlc);
    Serial.println();

    delay(100); // Short delay between frames
  }

  delay(1000); // Wait before sending the next message
}
