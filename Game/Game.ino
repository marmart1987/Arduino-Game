#include <LiquidCrystal.h>
#include <EEPROM.h>
#include "pitches.h"

#define BUZZER 4
int topRow[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
int bottomRow[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
int speed = 1000;
const int X_pin = A0;  // analog pin connected to X output
const int Y_pin = A1;  // analog pin connected to Y output
LiquidCrystal lcd(7, 8, 9, 10, 11, 12);

byte player[8] = {
  B00100,
  B01010,
  B00100,
  B10101,
  B00100,
  B01010,
  B10001,
};
byte wall[8] = {
  B11111,
  B01110,
  B00100,
  B01110,
  B00100,
  B01110,
  B10101,
  B11111
};
byte coin[8] = {
  B00000,
  B00100,
  B01010,
  B10101,
  B01010,
  B00100,
  B00000,
  B01110
};
void (*resetFunc)(void) = 0;
void setup() {
  lcd.noBlink();
  lcd.noCursor();
  Serial.begin(115200);
  lcd.begin(16, 2);
  int seedBuffer;
  long sTime = millis();
  while(millis()-sTime <= 50){
    sTime += analogRead(1) + analogRead(2) + analogRead(3) + analogRead(4) + analogRead(5);
  }

  randomSeed(sTime);
  lcd.createChar(2, coin);
  lcd.createChar(1, wall);
  lcd.createChar(0, player);
  pinMode(BUZZER, OUTPUT);
}
int lastTop = 0;
int lastBottom = 0;
bool slot = false;
bool playerPos = false;
long score = 0;
bool gameOver() {
  
  lcd.clear();
  lcd.home();

  Serial.println("Game Over!");
  lcd.print("Game Over!");
  lcd.setCursor(0, 1);
  lcd.print("Score: ");
  lcd.print(score);
  tone(BUZZER,NOTE_C4,300);
  delay(200);
  tone(BUZZER,NOTE_F3,300);
  delay(3000);
  lcd.home();
  lcd.print("HS: ");
  long HS = 0;
  EEPROM.get(0, HS);

  if (score > HS) {
    EEPROM.put(0, score);
    lcd.print(score);
    lcd.setCursor(0, 1);
    delay(1500);
    lcd.print("New High Score!");
    delay(3000);
  } else {
    lcd.print(HS);
    delay(3000);
  }
  resetFunc();
}
void checkInput() {
  lcd.home();
  if (analogRead(Y_pin) < 550) {
    playerPos = false;
  } else {
    if (analogRead(Y_pin) > 690) {
      playerPos = true;
    }
  }
  return true;
}
void reward(){
  score += 600;
  tone(BUZZER, NOTE_C5,250);
}
void loop() {
  if (speed > 50) {
    speed = floor(350 - (pow(log((millis() / 1000) + 1), 3.50969)));
  }

  shiftRows();
  if (slot == true) {
    chooseTiles();
  } else {
    topRow[14] = 0;
    bottomRow[14] = 0;
  }
  drawRow(topRow, true);
  drawRow(bottomRow, false);
  checkInput();
  slot = !slot;
  score += 30;
  long t1 = millis() + 1;
  tone(BUZZER,(millis()/50)%1200,10);
  while ((millis() + 1 - t1) < speed) {
    checkInput();
    if (int(floor((millis() / 5))) % 5 == 1) {
      if (playerPos == false) {

        if (topRow[0] == 1) {
          gameOver();
        }
        if (topRow[0] == 2) {
          reward();      
          topRow[0] = 0;
        }
        lcd.setCursor(0, 0);
        lcd.write(byte(0));
        lcd.setCursor(0, 1);
        lcd.print(" ");
      } else {
        if (bottomRow[0] == 1) {
          gameOver();
        }
        if (bottomRow[0] == 2) {
          reward();
          bottomRow[0] = 0;
        }
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
        lcd.setCursor(0, 0);
        lcd.print(" ");
      }
    }
  }

  Serial.print(" Speed: ");
  Serial.print(speed);

  Serial.print(" Lag: ");
  Serial.print((millis() + 1 - t1) - speed);

  Serial.print(" Score: ");
  Serial.println(score);
}
bool chooseTiles() {
  int currentTop = generateTile();
  int currentBottom = 0;
  if (lastBottom == 1) {
    currentTop = 0;
  }

  if (currentTop == 1) {
    currentBottom = 0;
    if (random(1, 3) == 1) {
      currentBottom = 2;
    }
  } else {

    if (currentTop == 0) {
      currentBottom = generateTile();
    }
  }
  if (lastTop == 1) {
    currentBottom = 0;
  }
  lastTop = currentTop;
  lastBottom = currentBottom;
  topRow[14] = currentTop;
  bottomRow[14] = currentBottom;
  return true;
}
bool shiftRows() {
  memcpy(topRow, &topRow[1], sizeof(topRow) - sizeof(bool));
  memcpy(bottomRow, &bottomRow[1], sizeof(bottomRow) - sizeof(bool));
}
bool drawRow(int row[15], bool isTopSide) {
  int rowNo = 1;
  if (isTopSide == true) { rowNo = 0; }
  for (int tile = 0; tile < 15; tile++) {
    lcd.setCursor(tile, rowNo);
    if (row[tile] == 1) {
      lcd.write(byte(1));
    } else {
      if (row[tile] == 2) {
        lcd.write(byte(2));
      } else {
        lcd.print(" ");
      }
    }
  }
  lcd.print(";");
}
int generateTile() {
  if (random(1, 4) == 1) {
    return 1;  //wall
  } else if (random(1, 6) == 1) {
    return 2;  //coin
  } else {
    return 0;  //air
  }
}