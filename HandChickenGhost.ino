/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID           "TMPL6uxgTRxrZ"
#define BLYNK_TEMPLATE_NAME         "Quickstart Template"
#define BLYNK_AUTH_TOKEN            "QeQtZYovnFcCeRgFHFzxLCEs5Sf4ZKbG"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
#define mySerial Serial2


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include "time.h"
#include <Adafruit_Fingerprint.h>
//#include <SPIFFS.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <HardwareSerial.h>

#define RX_PIN 18// 定义软件串行端口的 RX 和 TX 引脚
#define TX_PIN 19

HardwareSerial MegaSerial(1);

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "KAILANGLAPTOP";
char pass[] = "Junyi3329*";
//char ssid[] = "WiFi@FoonYew";
//char pass[] = "";

BlynkTimer timer;

// NTP server details
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 8 * 3600; // GMT+8 for MY Standard Time
const int daylightOffset_sec = 0; // No daylight saving time in MY
const int ntpdelaytime = 10 * 1000;
unsigned long long int lcdlast = 0;
int fingerErr = 0;
unsigned long long int timelast = 0;
const int fivesec = 5 * 1000;
const int onesec = 1000;
int cardlast = 0;
int nextAvailableLockerID = 22;


String date1 = "12/12/2024";
char date[20];
String time1 = "12:12:12";

// 定义Keypad的行和列数量
const byte ROWS = 4; // 四行
const byte COLS = 4; // 四列
// 定义Keypad的按键布局
char keys[ROWS][COLS] = {
  {'*','7','4','1'},
  {'0','8','5','2'},
  {'#','9','6','3'},
  {'D','C','B','A'}
};
// 定义连接Keypad的GPIO引脚
byte rowPins[ROWS] = {13, 12, 14, 27}; // 将这些引脚连接到Keypad的行引脚
byte colPins[COLS] = {26, 25, 33, 32}; // 将这些引脚连接到Keypad的列引脚

bool manualMode = false;
bool autoMode = true;
bool beforeSchool = true;
bool afterSchool = false;
bool autotem = false;
bool autotembeforeSchool = false;
bool autotemafterSchool = false;
bool manualtem = false;
bool manualtembeforeSchool = false;
bool manualtemafterSchool = false;
bool getfingerid = false;
bool SUMenu = true;

// 创建Keypad对象
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

LiquidCrystal_I2C lcd(0x27,20,4);

const int MAX_USERS = 5;
const int MAX_ID_LENGTH = 6;
const int MAX_PASSWORD_LENGTH = 6;

struct User {
  char studentID[MAX_ID_LENGTH + 1];
  int fingerprintID;
  char password[MAX_PASSWORD_LENGTH + 1];
  int lockerID; // 柜子编号
};

User users[MAX_USERS];
int userIndex = 0; // 当前用户索引

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  if (param.asInt() == 1) {
    manualMode = true;
    autoMode = false;
    if (autotem) {
      afterSchool = autotemafterSchool;
      beforeSchool = autotembeforeSchool;
      //autotem = false;
      manualtem = false;
    } else {
      if (manualtem) {
        afterSchool = manualtemafterSchool;
        beforeSchool = manualtembeforeSchool;
        //manualtem = false;
      }
    }
  } else {
    manualMode = false;
    autoMode = true;
    autoM();
  }
}

BLYNK_WRITE(V1)
{
  // any code you place here will execute when the virtual pin value changes
  Serial.println("Blynk.Cloud is writing something to V1");
  if(param.asInt() == 2 && manualMode)
  {
    // execute this code if the switch widget is now ON
    //digitalWrite(2,HIGH);  // Set digital pin 2 HIGH
    afterSchool = true;
    beforeSchool = false;
    manualtemafterSchool = true;
    manualtembeforeSchool = false;
    manualtem = true;
    autotem = false;
  }
  else
  {
    if (manualMode && param.asInt() == 0) {
      // execute this code if the switch widget is now OFF
      //digitalWrite(2,LOW);  // Set digital pin 2 LOW
      afterSchool = false;
      beforeSchool = true; 
      manualtemafterSchool = false;
      manualtembeforeSchool = true;
      manualtem = true;
      autotem = false;
    } else {
      if (param.asInt() == 1 && manualMode) {
        afterSchool = false;
        beforeSchool = false; 
        manualtemafterSchool = false;
        manualtembeforeSchool = false;
        manualtem = true;
        autotem = false;
      } else {
        if (autoMode && param.asInt() == 1) {
          autotemafterSchool = false;
          autotembeforeSchool =  false;
          autotem = true;
        } else {
          if (autoMode && param.asInt() == 0) {
            autotemafterSchool = false;
            autotembeforeSchool =  true;
            autotem = true;
          } else {
            if (autoMode && param.asInt() == 2) {
              autotemafterSchool = true;
              autotembeforeSchool =  false;
              autotem = true;
            }
          }
        }
      }
    }
  }
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V0);
  Blynk.syncVirtual(V1);
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent() {}

void setup() {
  // Debug console
  Serial.begin(9600);
  MegaSerial.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Starting");
  lcd.setCursor(0,3);
  lcd.print("---Initialization---");
  //初始化spiffs
  //if (!SPIFFS.begin(true)) {
   // Serial.println("SPIFFS initialization failed!");
  //  return;
  //}
  
  Serial.println("Blynk Connecting...");
  lcd.setCursor(8,0);
  lcd.print(".");
  lcd.setCursor(0,1);
  lcd.print("Connecting to WiFi:");
  lcd.setCursor(0,2);
  lcd.print(String(ssid));
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
  lcd.setCursor(9,0);
  lcd.print(".");
  // You can also specify server:
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Setup a function to be called every second
  timer.setInterval(1000L, myTimerEvent);
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  lcd.print(".");
  //fingerprint
  Serial.println("\n\nAdafruit finger initialization");
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  lcd.print(".");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Starting.....");
  lcd.setCursor(0,3);
  lcd.print("---Initialization---");
  lcd.setCursor(0,1);
  lcd.print("Init. Fingerprint");
  lcd.setCursor(12,0);
  lcd.print(".");
  // set the data rate for the sensor serial port
  finger.begin(57600);
  delay(5);
  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
    lcd.print(".");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Fingerprint Error!");
    lcd.setCursor(0,1);
    lcd.print("Restarting...");
    lcd.setCursor(0,3);
    lcd.print("---Initialization---");
    delay(3000);
    ESP.restart();
    while (1) { delay(1); }
  }

  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  lcd.print(".");

  finger.getTemplateCount();

  if (finger.templateCount == 0) {
    Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
  }
  else {
    Serial.println("Waiting for valid finger...");
      Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  }
  lcd.print(".");
  
  
  pinMode(5, INPUT_PULLUP);
  for (int i = 1; i < 6; i++) {
    deleteFingerprint(i);
  }
}

void loop() {
  
  Blynk.run();
  timer.run();
  getTime();
  //if (!digitalRead(2)) {
    //int x = zhiwenid();
    //Serial.println(String(x));
    //Serial.println(getKeypadString());
 // }
  //Serial.println(getChar());
  if (SUMenu) {
    startUpMenu();
  }
  
  
  char key = keypad.getKey(); // 获取按键值
  if (key != NO_KEY) { // 检查是否有按键被按下
    if (key == '*') { // 检查是否是 '*' 键
      Serial.println("Detected '*' key press");
      if (beforeSchool && !afterSchool) {
        SUMenu = false;
        login();
      } else if (!beforeSchool && afterSchool) {
        SUMenu = false;
        takeout();
      }
      // 在这里添加 '*' 键按下后的处理代码
    }
  }
  if (!digitalRead(5)) {
    MegaSerial.print("readcard\n");
    while (!MegaSerial.available()) {    }
    String reply = MegaSerial.readStringUntil('\n');
    Serial.println(reply);
  }
}  



void getTime() {
  if (timelast == 0 || millis() - timelast >= onesec) {
    timelast = millis();
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {
      Serial.println("Failed to obtain time");
      return;
    }
    strftime(date, sizeof(date), "%d/%m/%Y %H:%M:%S", &timeinfo);
    //Serial.print("Current time: ");
    //Serial.println(date);

    // Check if time is after 3:40 PM
    if (autoMode) {
      if (timeinfo.tm_hour > 15 || (timeinfo.tm_hour == 15 && timeinfo.tm_min >= 40)) {
        //Serial.println("Current time is after 3:40 PM");
        afterSchool = true;
      } else {
        //Serial.println("Current time is before 3:40 PM");
        afterSchool = false;
      }

      // Check if time is before 7:30 AM
      if (timeinfo.tm_hour < 7 || (timeinfo.tm_hour == 7 && timeinfo.tm_min < 30)) {
        //Serial.println("Current time is before 7:30 AM");
        beforeSchool = true;
      } else {
        //Serial.println("Current time is after 7:30 AM");
        beforeSchool = false;
      }
    }
  }
}

void login() {
  unsigned long long int starttime = millis();
  char studentID[MAX_ID_LENGTH + 1] = {0};
  int length = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("---Login Program----");
  lcd.setCursor(0, 1);
  lcd.print("Stu.ID: (Selection)");
  lcd.setCursor(0,2);
  lcd.print("       1|Card 2|Type");
  char key = keypad.getKey(); // 获取按键值
  while (key == NO_KEY) { // 检查是否有按键被按下
    key = keypad.getKey();
    if (key == '1') { // 检查是否是 '1' 键
      Serial.println("Detected '1' key press");
      lcd.setCursor(0,1);
      lcd.print("                    ");
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,1);
      lcd.print("Stu.ID: ");
      MegaSerial.print("readcard\n");
      MegaSerial.print("readcard\n");
      MegaSerial.print("readcard\n");
      MegaSerial.print("readcard\n");
      MegaSerial.print("readcard\n");
      while (!MegaSerial.available()) {
        if (millis() - starttime >= millis()) {
          return;
        }
      }
      String reply = MegaSerial.readStringUntil('\n');
      reply.trim();
      Serial.println(reply);
      lcd.print(reply);
      // 将 reply 保存到用户的学生ID中
      if (userIndex < MAX_USERS) {
        reply.toCharArray(users[userIndex].studentID, MAX_ID_LENGTH + 1);
      } else {
        Serial.println("User limit reached.");
        return;
      }
      delay(1000);
      waitForFingerprintID();
      waitForPassword();
      assignLockerID();
      userIndex++;
    } else if (key == '2') {
      //keypad输入
      lcd.setCursor(0,1);
      lcd.print("                    ");
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,1);
      lcd.print("Stu.ID: ");
      waitForStudentID();
      waitForFingerprintID(); // 获取指纹ID
      waitForPassword(); // 获取密码
      assignLockerID();
      userIndex++;
    }
  }
}

void waitForStudentID() {
  char studentID[MAX_ID_LENGTH + 1] = {0};
  int length = 0;

  while (true) {
    char key = keypad.getKey();

    if (key) { // 如果有按键被按下
      if (key == '#' && length > 0) { // 如果按下了退格键
        length--;
        studentID[length] = '\0'; // 确保字符串以null结尾
      } else if (key != '#' && key != '*' && length < MAX_ID_LENGTH) { // 忽略无效字符
        studentID[length] = key; // 将字符添加到学号
        length++;
        studentID[length] = '\0'; // 确保字符串以null结尾
      } else if (key == '*' && length >= 5) { // 输入完成标志，假设学号长度为5到6位
        strncpy(users[userIndex].studentID, studentID, MAX_ID_LENGTH);
        users[userIndex].studentID[MAX_ID_LENGTH] = '\0'; // 确保字符串以null结尾
        Serial.print("Saved Student ID: ");
        Serial.println(users[userIndex].studentID);
         // 增加用户索引
        break; // 退出循环
      }
      updateIDDisplay(studentID, length);
    }
  }
}

void waitForFingerprintID() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("---Login Program----");
  lcd.setCursor(0, 1);
  lcd.print("Fingerprint Enroll:");
  while (!getFingerprintEnroll());
  users[userIndex].fingerprintID = userIndex; // 保存指纹ID
}

void waitForPassword() {
  char password[MAX_PASSWORD_LENGTH + 1] = {0};
  int length = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("---Login Program----");
  lcd.setCursor(0, 1);
  lcd.print("Set Password");
  lcd.setCursor(0,2);
  lcd.print("Password: ");
  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#' && length > 0) { // 退格键
        length--;
        password[length] = '\0'; // 确保字符串以null结尾
      } else if (key != '#' && key != '*' && length < MAX_PASSWORD_LENGTH) { // 输入密码
        password[length] = key;
        length++;
        password[length] = '\0';
      } else if (key == '*' && length == MAX_PASSWORD_LENGTH) { // 确认输入
        strncpy(users[userIndex].password, password, MAX_PASSWORD_LENGTH);
        users[userIndex].password[MAX_PASSWORD_LENGTH] = '\0'; // 确保字符串以null结尾
        break;
      }
      updatePasswordDisplay(password, length);
    }
  }
  SUMenu = true;
}

void updatePasswordDisplay(const char* password, int length) {
  lcd.setCursor(0, 2);
  lcd.print("Password: ");
  // 隐藏所有之前输入的字符，用 '*' 填充
  for (int i = 0; i < length - 1; i++) {
    lcd.print('*');
  }
  // 显示最新输入的字符
  if (length > 0) {
    lcd.print(password[length - 1]); // 最新输入的字符
  }
  // 确保密码显示区域长度一致
  for (int i = length; i < MAX_PASSWORD_LENGTH; i++) {
    lcd.print(' ');
  }
  lcd.setCursor(length + 5, 1);
}

void updateIDDisplay(char* studentID, int length) {
  lcd.setCursor(8, 1); // 将光标设置到第二行的起始位置
  lcd.print(studentID); // 显示输入的学号
  // 清除剩余位置
  for (int i = length; i < 12; i++) {
    lcd.print(' ');
  }
}

void assignLockerID() {
  if (nextAvailableLockerID > 26) {
    Serial.println("No available locker IDs.");
    return;
  }
  users[userIndex].lockerID = nextAvailableLockerID;
  nextAvailableLockerID++;
}

void takeout() {
  unsigned long long int starttime = millis();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("--Retrieve Program--");
  lcd.setCursor(0, 1);
  lcd.print("Stu.ID: (Selection)");
  lcd.setCursor(0,2);
  lcd.print("       1|Card 2|Type");
  char key = keypad.getKey(); // 获取按键值
  while (key == NO_KEY) { // 检查是否有按键被按下
    key = keypad.getKey();
    if (key == '1') { // 检查是否是 '1' 键
      Serial.println("Detected '1' key press");
      lcd.setCursor(0,1);
      lcd.print("                    ");
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,1);
      lcd.print("Stu.ID: ");
      MegaSerial.print("readcard\n");
      MegaSerial.print("readcard\n");
      MegaSerial.print("readcard\n");
      MegaSerial.print("readcard\n");
      MegaSerial.print("readcard\n");
      while (!MegaSerial.available()) {
        if (millis() - starttime >= fivesec) {
          return;
        }
      }
      String reply = MegaSerial.readStringUntil('\n');
      reply.trim();
      Serial.println(reply);
      lcd.print(reply);
      delay(1000);
      String id = reply;
      option(id);
    } else if (key == '2') {
      //keypad输入
      lcd.setCursor(0,1);
      lcd.print("                    ");
      lcd.setCursor(0,2);
      lcd.print("                    ");
      lcd.setCursor(0,1);
      lcd.print("Stu.ID: ");
      ASwaitForStudentID();
      
    }
  }
}

void ASwaitForStudentID() {
  char studentID[MAX_ID_LENGTH + 1] = {0};
  int length = 0;

  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#' && length > 0) { // 退格键
        length--;
        studentID[length] = '\0'; // 确保字符串以null结尾
      } else if (key != '#' && key != '*' && length < MAX_ID_LENGTH) { // 输入学号
        studentID[length] = key;
        length++;
        studentID[length] = '\0';
      } else if (key == '*' && length >= 5) { // 确认输入
        int ASuserIndex = findUserIndex(studentID);
        Serial.println(String(ASuserIndex));
        if (ASuserIndex != -1) {          
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("--Retrieve Program--");
          lcd.setCursor(0, 1);
          lcd.print("Verify identidy:");
          lcd.setCursor(0,2);
          lcd.print("1|Fingerprint");
          lcd.setCursor(0, 3);
          lcd.print("2|Password");
          
          while (true) {
            char option = keypad.getKey();
            if (option == '1') {
              verifyFingerprint(ASuserIndex);
              break;
            } else if (option == '2') {
              verifyPassword(ASuserIndex);
              break;
            }
          }
        } else {
          lcd.setCursor(0, 1);
          lcd.print("User not found");
          delay(2000);
        }
        break;
      }
      updateIDDisplay(studentID, length);
    }
  }
}

void option(String studentID) {
  // 将整数 studentID 转换为字符串
  String studentIDStr = String(studentID);
  int ASuserIndex = findUserIndex(studentIDStr.c_str()); // 使用 c_str() 将 String 转换为 const char*
  
  if (ASuserIndex != -1) {          
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("--Retrieve Program--");
    lcd.setCursor(0, 1);
    lcd.print("Verify identity:");
    lcd.setCursor(0, 2);
    lcd.print("1|Fingerprint");
    lcd.setCursor(0, 3);
    lcd.print("2|Password");

    while (true) {
      char option = keypad.getKey();
      if (option == '1') {
        verifyFingerprint(ASuserIndex);
        break;
      } else if (option == '2') {
        verifyPassword(ASuserIndex);
        break;
      }
    }
  } else {
    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print("User not found");
    delay(2000);
  }
}


void verifyFingerprint(int ASuserIndex) {
  int id = ASuserIndex + 1;
  Serial.println(id);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("--Retrieve Program--");
  lcd.setCursor(0,1);
  lcd.print("FGP. Verification:");
  while (true) {
    int zhiwen = zhiwenid();
    if (zhiwen == 0 || zhiwen != id) {
      lcd.setCursor(0,2);
      lcd.print("Failed Pls try again");
    } else if (zhiwen == id) {
      lcd.setCursor(0,2);
      lcd.print("Success!");
      unlock(ASuserIndex);
      break;
    }
  }
}

void verifyPassword(int ASuserIndex) {
  char enteredPassword[MAX_PASSWORD_LENGTH + 1] = {0};
  int length = 0;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("--Retrieve Program--");
  lcd.setCursor(0,1);
  lcd.print("Pwd. Verification");
  lcd.setCursor(0,2);
  lcd.print("Password: ");
  
  while (true) {
    char key = keypad.getKey();
    if (key) {
      if (key == '#' && length > 0) { // 退格键
        length--;
        enteredPassword[length] = '\0'; // 确保字符串以null结尾
      } else if (key != '#' && key != '*' && length < MAX_PASSWORD_LENGTH) { // 输入密码
        enteredPassword[length] = key;
        length++;
        enteredPassword[length] = '\0';
      } else if (key == '*' && length == MAX_PASSWORD_LENGTH) { // 确认输入
        if (strcmp(enteredPassword, users[ASuserIndex].password) == 0) {
          lcd.setCursor(0, 3);
          lcd.print("Password OK");
          unlock(ASuserIndex);
          delay(2000);
          break;
        } else {
          lcd.setCursor(0, 3);
          lcd.print("Password FAIL");
        }
      }
      updatePasswordDisplay(enteredPassword, length);
    }
  }
}

void unlock(int ASuserIndex) {
  int pin = users[ASuserIndex].lockerID;
  
  // 使用 String 类构造函数进行字符串拼接
  String message = String(pin) + "Unlock";
  MegaSerial.print(message + "\n");
  Serial.print(message + "\n");
  
  // 等待串口数据可用，并添加超时处理
  unsigned long startTime = millis();
  const unsigned long timeout = 5000; // 5 秒超时
  while (!MegaSerial.available()) {
    if (millis() - startTime > timeout) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Timeout");
      SUMenu = true;
      return;
    }
  }
  
  String reply = MegaSerial.readStringUntil('\n');
  reply.trim(); // 去掉前后空格和换行符
  Serial.println(reply);

  // 比较字符串
  String expectedReply = "Relay on pin " + String(pin) + " unlocked.";
  if (reply == expectedReply) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unlocked");
    delay(2000);
    SUMenu = true;
  } else {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unlock Failed");
    delay(2000);
    SUMenu = true;
    return;
  }
}

int findUserIndex(const char* studentID) {
  for (int i = 0; i < userIndex; i++) {
    if (strcmp(users[i].studentID, studentID) == 0) {
      return i;
    }
  }
  return -1; // 用户未找到
}

void displayAllUsers() {
  lcd.clear();
  for (int i = 0; i < userIndex; i++) {
    Serial.print("User ");
    Serial.print(i + 1);
    Serial.println(":");
    Serial.print("Student ID: ");
    Serial.println(users[i].studentID);
    Serial.print("Fingerprint ID: ");
    Serial.println(users[i].fingerprintID);
    Serial.print("Password: ");
    Serial.println(users[i].password);
    Serial.println("-------------------");
  }
  lcd.setCursor(0, 0);
  lcd.print("Check Serial Monitor");
}

void startUpMenu() {
  if (lcdlast == 0 || millis() - lcdlast >= onesec) {
    lcdlast = millis();
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("------Welcome-------");
    lcd.setCursor(0, 1);
    lcd.print(date);
    lcd.setCursor(0, 3);
    lcd.print(" Press '*' to start");
  }
  
}

String getKeypadString() {
  String inputString = "";
  char key;
  while (true) {
    key = keypad.getKey();
    if (key != NO_KEY) {
      if (key == '#') {  // '#' 作为输入结束的标志
        break;
      } else {
        inputString += key;
      }
    }
  }
  return inputString;
}

char getChar() {
  char key;
  while (true) {
    key = keypad.getKey();
    if (key != NO_KEY) {
      return key;
    }
  }
}


uint8_t zhiwenid() {
  getfingerid = true;
  while (getfingerid) {
    uint8_t id = getFingerprintID();
    if (fingerErr == 1) {
      //Serial.println("未检测到指纹");
    } else if (fingerErr == 6) {
      Serial.println("未找到匹配的指纹");
    } else if (fingerErr == 2) {
      Serial.println("通信错误");
    } else if (fingerErr == 5) {
      Serial.println("指纹图像采集失败");
    } else if (fingerErr == 4) {
      Serial.println("未知错误");
    } else if (fingerErr != 0) {
      Serial.println("出错了，请重试");
    } else if(fingerErr == 0) {
      Serial.print("检测到指纹，ID：");
      Serial.println(id);
      return id;
      getfingerid = false;
    } else {
      Serial.println("未知错误");
      return 0;
    }
  }
}

void autoM() {
  // Get current time
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }

  // Print current time
  Serial.println(&timeinfo, "Current time: %Y-%m-%d %H:%M:%S");

  // Check if time is after 3:40 PM
  if (timeinfo.tm_hour > 15 || (timeinfo.tm_hour == 15 && timeinfo.tm_min >= 40)) {
    Serial.println("Current time is after 3:40 PM");
    afterSchool = true;
  } else {
    Serial.println("Current time is before 3:40 PM");
    afterSchool = false;
  }

  // Check if time is before 7:30 AM
  if (timeinfo.tm_hour < 7 || (timeinfo.tm_hour == 7 && timeinfo.tm_min < 30)) {
    Serial.println("Current time is before 7:30 AM");
    beforeSchool = true;
  } else {
    Serial.println("Current time is after 7:30 AM");
    beforeSchool = false;
  }
}

uint8_t getFingerprintID() {
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println("No finger detected");
      fingerErr = 1;
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      fingerErr = 2;
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      fingerErr = 3;
      return p;
    default:
      Serial.println("Unknown error");
      fingerErr = 4;
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      fingerErr = 1;
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      fingerErr = 2;
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      fingerErr = 5;
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      fingerErr = 5;
      return p;
    default:
      Serial.println("Unknown error");
      fingerErr = 4;
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    fingerErr = 2;
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    fingerErr = 6;
    return p;
  } else {
    Serial.println("Unknown error");
    fingerErr = 4;
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  fingerErr = 0;

  return finger.fingerID;
}

uint8_t getFingerprintEnroll() {
  int id = userIndex + 1;
  int p = -1;
  Serial.print("Waiting for valid finger to enroll as #"); Serial.println(id);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  Serial.println("Remove finger");
  lcd.setCursor(0,2);
  lcd.print("   Remove Finger");
  delay(2000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  Serial.print("ID "); Serial.println(id);
  p = -1;
  Serial.println("Place same finger again");
  lcd.setCursor(0,2);
  lcd.print(" Place finger again");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!
  lcd.setCursor(0,2);
  lcd.print("                    ");
  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  Serial.print("Creating model for #");  Serial.println(id);

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    lcd.setCursor(0,2);
    lcd.print("Success");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

uint8_t deleteFingerprint(uint8_t id) {
  uint8_t p = -1;

  p = finger.deleteModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Deleted!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not delete in that location");
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
  } else {
    Serial.print("Unknown error: 0x"); Serial.println(p, HEX);
  }

  return p;
}
