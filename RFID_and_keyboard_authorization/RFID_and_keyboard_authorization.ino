#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <SPI.h>
#include <MFRC522.h>

#define ROWS 4
#define COLS 4
#define PASSWORD_LENGTH 4
#define I2C_ADDRESS 0x27
#define R_PIN A2
#define G_PIN A1
#define B_PIN A0
#define BUZZER_PIN 12
#define RST_PIN 10
#define SS_PIN 11 
#define KEYBOARD_COL_PINS {6, 7, 8, 9}
#define KEYBOARD_ROW_PINS {2, 3, 4, 5}

char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

MFRC522 mfrc522(SS_PIN, RST_PIN); 
Keypad keypad4x4 = Keypad(makeKeymap(keys), KEYBOARD_ROW_PINS, KEYBOARD_COL_PINS, ROWS, COLS); 
LiquidCrystal_I2C lcd(I2C_ADDRESS, 16, 2);
  
const byte uid[4] = {0x03, 0x55, 0xD4, 0x1E};
const char* password = "1852";
char keyboardNumberBuffer[PASSWORD_LENGTH];
int currentKeyboardInputPosition = -1;
bool isRFIDInputValidated = false;
bool isKeyboardInputValidated = false;

void setup() {
  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  turnLed(0);
  lcd.backlight();
  lcd.init();
  Serial.begin(9600);
  SPI.begin();
	mfrc522.PCD_Init();	
  mfrc522.PCD_DumpVersionToSerial();
}

void loop() {
  if(!isRFIDInputValidated){
    if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()){
      return;
    }
    printRFIDInputUID();
    if(isReadUIDValid()){
      confirmRFIDInput();
    }
    else{
      denyRFIDInput();
      return;
    }
  }

  char key = keypad4x4.getKey();

  if(key == 'C'){
    restartKeyboardInput();
  }
  else if(key == 'A'){
    restartSystem();
  }
  
  if(isKeyboardInputValidated){
    return;
  }
  else if(isNumber(key)){   
    if(currentKeyboardInputPosition < (PASSWORD_LENGTH - 1)){
      currentKeyboardInputPosition++;
      keyboardNumberBuffer[currentKeyboardInputPosition] = key;
    }
    printInputPassword();
  }
  else if(key == 'B'){
    if(currentKeyboardInputPosition > -1){
      keyboardNumberBuffer[currentKeyboardInputPosition] = 0;
      currentKeyboardInputPosition--;
    }
    printInputPassword();
  }
  else if(key == 'D'){
    if(isInputPasswordValid()){
      grantAccess();
    }
    else{
      denyAccess();
    }
    isKeyboardInputValidated = true;
  }
}

bool isNumber(char key){
  if(key == '1' || key == '2' || key == '3' || 
    key == '4' || key == '5' || key == '6' || 
    key == '7' || key == '8' || key == '9' ||
    key == '0'){
    return true;
  }
  return false;
}

void printRFIDInputUID(){
  Serial.println("Input UID:");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();
}

bool isReadUIDValid(){
  for(byte i = 0; i < mfrc522.uid.size; i++){
    if(mfrc522.uid.uidByte[i] != uid[i]){
      return false;
    }
  }
  return true;
}

void blinkLed(){
  for(byte i = 0; i < 3; i++){
    turnLed('r');
    delay(300);
    turnLed(0);
    delay(300);
  }
}

void notifyKeyboardInput(){
  turnLed('b');
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.printstr("Enter password:");
}

void confirmRFIDInput(){
  isRFIDInputValidated = true;
  notifyKeyboardInput();
  mfrc522.PICC_HaltA();
}

void denyRFIDInput(){
  blinkLed();
  denyAccess();
  mfrc522.PICC_HaltA();
}

void printInputPassword(){
  lcd.clear();
  for(byte i = 0; i <= currentKeyboardInputPosition; i++){
    lcd.setCursor(i, 0); 
    lcd.print(keyboardNumberBuffer[i]);
  }
}

bool isInputPasswordValid(){
  for(byte i = 0; i < PASSWORD_LENGTH; i++){
    if(keyboardNumberBuffer[i] != password[i]){
      return false;
    }
  }
  return true;
}

void grantAccess(){
  turnLed('g');
  soundBuzzer(100);
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.printstr("Access granted");
}

void denyAccess(){
  turnLed('r');
  lcd.clear();
  lcd.setCursor(0, 0); 
  lcd.printstr("Access denied");
}

void emptyBuffer(){
  for(byte i = 0; i < PASSWORD_LENGTH; i++){
    keyboardNumberBuffer[i] = 0;
  }
}

void restartKeyboardInput(){
  isKeyboardInputValidated = false;
  turnLed('b');
  emptyBuffer(); 
  lcd.clear();
  lcd.setCursor(0, 0);
  currentKeyboardInputPosition = -1;
}

void restartSystem(){
  restartKeyboardInput();
  turnLed(0);
  isRFIDInputValidated = false;
}

void soundBuzzer(int ms){
  analogWrite(BUZZER_PIN, 150);
  delay(ms);
  analogWrite(BUZZER_PIN, 0);
  delay(ms);
}

void turnLed(char led){
  if(led == 'r'){
    digitalWrite(R_PIN, HIGH);
    digitalWrite(G_PIN, LOW);
    digitalWrite(B_PIN, LOW);
  }
  else if(led == 'g'){
    digitalWrite(R_PIN, LOW);
    digitalWrite(G_PIN, HIGH);
    digitalWrite(B_PIN, LOW);
  }
  else if(led == 'b'){
    digitalWrite(R_PIN, LOW);
    digitalWrite(G_PIN, LOW);
    digitalWrite(B_PIN, HIGH);
  }
  else {
    digitalWrite(R_PIN, LOW);
    digitalWrite(G_PIN, LOW);
    digitalWrite(B_PIN, LOW);
  }
}
