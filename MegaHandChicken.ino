#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  53  // 选择你的ESP32的SS引脚
#define RST_PIN 5  // 选择你的ESP32的RST引脚

MFRC522 mfrc522(SS_PIN, RST_PIN); // 创建MFRC522实例

unsigned long lastCommandTime = 0;
const int fivesec = 5000; // 5 秒超时

// 预定义的用户名和卡ID
struct User {
  const char* username;
  byte cardID[4];
};

User users[] = {
  {"212878", {0x23, 0x77, 0xF1, 0xFD}},
  {"2392B", {0x83, 0x26, 0xE5, 0xF7}},
  {"Charlie", {0x34, 0x56, 0x78, 0x9A}},
};

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600); // 初始化串口通信
  SPI.begin(); // 启动SPI总线
  mfrc522.PCD_Init(); // 初始化MFRC522
  for (int i = 22; i <= 25; i++) {
    pinMode(i, OUTPUT); // 将数字22到25设置为输出模式
    digitalWrite(i, HIGH); // 初始状态设置为高电平（锁定继电器）
  }
}

void loop() {
  if (Serial1.available() > 0) {
    String command = Serial1.readStringUntil('\n');  // 读取串口输入
    Serial.println(command);

    if (command == "readcard") {
      unsigned long currentMillis = millis();
      if (currentMillis - lastCommandTime >= fivesec) {
        lastCommandTime = currentMillis;
        Serial.println("Place your card on the reader...");

        // 等待卡片读取
        unsigned long startTime = millis();
        while (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
          if (millis() - startTime >= fivesec) {
            Serial.println("Timeout waiting for card.");
            return; // 超时后退出
          }
          // 持续等待直到读取到卡片
        }

        // 读取卡片ID
        byte readCardID[4];
        for (byte i = 0; i < 4; i++) {
          readCardID[i] = mfrc522.uid.uidByte[i];
        }

        // 查找用户名
        const char* username = "Unknown";
        for (User user : users) {
          if (memcmp(readCardID, user.cardID, 4) == 0) {
            username = user.username;
            break;
          }
        }

        // 打印用户名
        Serial.print("Username: ");
        Serial.println(username);
        Serial1.println(username);

        // 停止读取卡片
        mfrc522.PICC_HaltA();
        mfrc522.PCD_StopCrypto1();
      } else {
        Serial.println("Command executed recently. Please wait.");
      }
    } else {
      Serial.println("Unknown command. Please type 'readcard' to read a card.");
      jidianqi(command);
    }
  }
}

void jidianqi(String input) {
  if (input.length() >= 4) {
    int pin = (input.charAt(0) - '0') * 10 + (input.charAt(1) - '0'); // 解析前两位数字作为引脚号
    String command = input.substring(2); // 提取剩余部分作为指令

    if (pin >= 22 && pin <= 25) { // 检查引脚号是否在允许范围内
      if (command.equalsIgnoreCase("Unlock")) {
        digitalWrite(pin, LOW); // 设置引脚为低电平，解锁继电器
        Serial.println("Relay on pin " + String(pin) + " unlocked.");
        Serial1.println("Relay on pin " + String(pin) + " unlocked.");
      } else if (command.equalsIgnoreCase("Lock")) {
        digitalWrite(pin, HIGH); // 设置引脚为高电平，锁定继电器
        Serial.println("Relay on pin " + String(pin) + " locked.");
        Serial1.println("Relay on pin " + String(pin) + " locked.");
      } else {
        Serial.println("Invalid command.");
      }
    } else {
      Serial.println("Invalid pin number.");
    }
  } else {
    Serial.println("Invalid input format.");
  }
}
