// Libraries included
#include <LiquidCrystal.h>
#include <Keypad.h>
#include <TimerOne.h>

// number of rows and number of columns
const byte ROWS = 4; 
const byte COLS = 4; 

// Joystick pins
const int JOYSTICK_X = A1;
const int JOYSTICK_Y = A2;
//const int JOYSTICK_BTN = 44; // Optional joystick button

// Threshold for joystick movement
const int JOYSTICK_THRESHOLD = 400; // Adjust as needed
const int CENTER = 512; // Midpoint for analog joystick (range: 0-1023)

// Variables to track joystick state
String joystickInput = "";

//Button pin
const int BTN = 18;

// LED pins
int LED_G1 = 4;
int LED_G2 = 3;
int LED_G3 = 2;
int LED_R1 = 5;
int LED_R2 = 6;
int LED_R3 = 7;
int LED_B1 = 36;
int LED_B2 = 37;

//Speaker on pin 42
const int SPEAKER = 42;

//Music key
#define NOTE_F 87

//Define note to SpeakerKey
volatile int SpeakerKey = NOTE_F;

// LED ARRAY
int LED_G[3] {LED_G1, LED_G2, LED_G3};
int LED_R[3] {LED_R1, LED_R2, LED_R3};
int LED_B[2] {LED_B1, LED_B2};

// Constants
int i = 0; // constant for check loop
int a = 0; // constant for membrane input
int j = 1; // current level
int k = 0; // constant for LED loop
int b = 0; // constant for LED loop
int c = 0;
int counter = 0; // counter to check if membrane input matches random output
char randOutput[12]; // max size of random output
char MembraneInput[sizeof(randOutput)]; // max size of membrane input, set at size of random output
long randChar; // define random output

// define what each keypad outputs
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// The random character array to include directions
char randomChar[18] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', '^', 'v', '<', '>'}; // U: Up, D: Down, L: Left, R: Right

// Define row and column pins for the keypad
byte rowPins[ROWS] = {53, 52, 51, 50}; // row pinouts of the keypad
byte colPins[COLS] = {49, 48, 47, 46}; // column pinouts of the keypad

// Function to register our inputs to the corresponding row and column
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// LCD module pins
LiquidCrystal lcd(8, 9, 10, 11, 12, 13);

// Non-blocking LED timing variables, being used to make the led blink with real time millis instead of the use of delay
unsigned long previousMillisG = 0;
unsigned long previousMillisR = 0;
unsigned long previousMillisB = 0;
unsigned long previousMillisSpeaker = 0;
const unsigned long intervalG = 100;
const unsigned long intervalR = 100;
unsigned long intervalB = 1000;
unsigned long intervalSpeaker = 1000;
bool ledStateG = LOW;
bool ledStateR = LOW;
bool ledStateB = LOW;

// 15-second timer variables to decide the length of each level
unsigned long levelStartTime = 0;
const unsigned long levelTimeLimit = 14000; // 15 seconds in milliseconds

// Function to clear specified row
void clearRow(byte rowToClear) {
  lcd.setCursor(0, rowToClear);
  lcd.print("                ");  
}

void Levels(byte level) {
  //Stop Speaker
  noTone(SPEAKER);
  //Only print level on lcd when you are on level 1-5
  if (level < 9) {
    clearRow(0);
    lcd.setCursor(5, 0);
    lcd.print("LEVEL ");
    lcd.print(level);
    delay(2000);
    clearRow(0);
  }

  // Generate random characters for the level
  randomSeed(analogRead(0));
  for (i = 0; i <= (3 + level - 1); i++) { // Adjusting for level difficulty
    randChar = random(0, 17); // Updated range for joystick inputs
    randOutput[i] = randomChar[randChar]; // assigning random char to our random output on the lcd screen
    if(level == 9){ // if statement to determine the max level and print victory and generate lightshow. Reset to level 1
      clearRow(0);
      lcd.setCursor(3, 0);
      lcd.print("VICTORY :)");
      unsigned long startTime = millis();
      while (millis() - startTime < 6800) {// Blink for 6.8 seconds
        blinkGreenLEDsNonBlocking();
        blinkRedLEDsNonBlocking();
      }
      clearRow(0);
      clearRow(1);
      j = 1;
       // Reset Blue time indicator;
      intervalB = 1000;
      ledStateB = LOW;

      //Reset Speaker
      intervalSpeaker = 1000;

      Levels(j);
      break;
    }
  }
  randOutput[i] = '\0'; // Null-terminate the string

  // Reset timer for level
  levelStartTime = millis();

  // Display the random output
  lcd.setCursor(0, 0);
  lcd.print(randOutput);
  Serial.println(randOutput);
}

void readJoystickInput() {
  int xValue = analogRead(JOYSTICK_X);
  int yValue = analogRead(JOYSTICK_Y);

  if (xValue > CENTER + JOYSTICK_THRESHOLD) {
    joystickInput = ">"; // Right
  } else if (xValue < CENTER - JOYSTICK_THRESHOLD) {
    joystickInput = "<"; // Left
  } else if (yValue > CENTER + JOYSTICK_THRESHOLD) {
    joystickInput = "v"; // Down
  } else if (yValue < CENTER - JOYSTICK_THRESHOLD) {
    joystickInput = "^"; // Up
  } else {
    joystickInput = "";
  }

  // Append joystick input to MembraneInput
  if (joystickInput != "" && a < sizeof(MembraneInput) - 1) {
    MembraneInput[a] = joystickInput[0];
    a++;
    MembraneInput[a] = '\0'; // Null-terminate
    lcd.setCursor(0, 1);
    lcd.print(MembraneInput);
    Serial.println(MembraneInput);
    delay(200); // Debounce joystick input
  }
}



// Function to blink LEDs non-blocking. This function is important, so the LED can blink without interfering with the other hardware in order to prevent 
// timing conflicts

// Blue LED Blink
void blinkBlueLEDsNonBlocking() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisB >= intervalB) {
    previousMillisB = currentMillis;
    ledStateB = !ledStateB;
    for (int k = 0; k < 2; k++) {
      digitalWrite(LED_B[k], ledStateB);
    }
    intervalB -= 35; // Reduce timer for faster blinking
  }
}


//Red LED blink
void blinkRedLEDsNonBlocking() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisR >= intervalR) {
    previousMillisR = currentMillis;
    ledStateR = !ledStateR;
    for (int k = 0; k < 3; k++) {
      digitalWrite(LED_R[k], ledStateR);
      delay(100);
    }
  }
}

//Green LED blink
void blinkGreenLEDsNonBlocking() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisG >= intervalG) {
    previousMillisG = currentMillis;
    ledStateG = !ledStateG;
    for (int k = 0; k < 3; k++) {
      digitalWrite(LED_G[k], ledStateG);
      delay(100);
    }
  }
}

// Function to check if the level is complete
void checkComplete(byte level) {
  counter = 0;
  for (i = 0; i <= (3 + level - 1); i++) {
    if (MembraneInput[i] == randOutput[i]) {
      counter++; //counter that increments everytime our input is equal the output
    }
  }

   //if statement to check if counter is equal to the corresponding random output, so on level 1 the counter is 4 and we have (3+level-1)+1 = 4
  if (counter == (3 + level - 1) + 1) {
    clearRow(0);
    clearRow(1);
    lcd.setCursor(0, 0);
    lcd.print("LEVEL COMPLETE");

    //Green LED blink, and starting the time for the duration of the blinks
    unsigned long startTime = millis();
    while (millis() - startTime < 1600) { //blink for 1.6 seconds
      blinkGreenLEDsNonBlocking();
    }
    // Reset input buffer and advance to next level
    memset(MembraneInput, 0, sizeof(MembraneInput));
    a = 0;
    j++; // Advance to next level

    // Reset Blue time indicator;
    intervalB = 1000;
    ledStateB = LOW;

    //Reset Speaker
    intervalSpeaker = 1000;
  
    Levels(j); // Initialize the next level
  } else {
    clearRow(0);
    clearRow(1);
    lcd.setCursor(0, 0);
    lcd.print("TRY AGAIN");

    //RED LED blink, and starting the time for the duration of the blinks
    unsigned long startTime = millis();
    while (millis() - startTime < 1600) {
      blinkRedLEDsNonBlocking();
    }
    clearRow(0);

    //Reset input buffer
    memset(MembraneInput, 0, sizeof(MembraneInput));
    a = 0;
    lcd.setCursor(0, 0);
    lcd.print(randOutput);
  }
}

//funtion to restart the game, and print Game Over
void GameOver() {
  //Speaker crazy mode
  tone(SPEAKER,SpeakerKey);

  clearRow(0);
  clearRow(1);
  lcd.setCursor(3, 0);
  lcd.print("GAME OVER!");

  //Red LED lightshow, when game is over
  unsigned long startTime = millis();
    while (millis() - startTime < 3600) { // Blink for 3.6 second
      blinkRedLEDsNonBlocking();
    }
  // Reset input buffer
  memset(MembraneInput, 0, sizeof(MembraneInput));
  a=0;
  // Reset Blue time indicator;
  intervalB = 1000;
  ledStateB = LOW;

  //Reset Speaker
  intervalSpeaker = 1000;

  j = 1; // Reset to level 1
  Levels(j);
}

//Function to speaker sound with faster buzzing with time passing
void Speaker() {
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillisSpeaker >= intervalSpeaker) {
    previousMillisSpeaker = currentMillis;
    tone(SPEAKER,SpeakerKey);
    delay(50);
    noTone(SPEAKER);
    intervalSpeaker -= 35; // Reduce timer for faster buzzing
  }
}

void Restart(){
  j = 1;

  // Reset Blue time indicator;
  intervalB = 1000;
  ledStateB = LOW;

  //Reset Speaker
  intervalSpeaker = 1000;

  // Reset input buffer
  memset(MembraneInput, 0, sizeof(MembraneInput));
  a=0;

  // Reset timer for level
  levelStartTime = millis();
  Levels(j);
}

void setup() {
  //manual intterupt button, game over
  attachInterrupt(digitalPinToInterrupt(BTN), Restart, RISING);
  pinMode(BTN, INPUT_PULLUP);
  // Setup LEDs
  pinMode(LED_G1, OUTPUT);
  pinMode(LED_G2, OUTPUT);
  pinMode(LED_G3, OUTPUT);
  pinMode(LED_R1, OUTPUT);
  pinMode(LED_R2, OUTPUT);
  pinMode(LED_R3, OUTPUT);
  pinMode(LED_B1, OUTPUT);
  pinMode(LED_B2, OUTPUT);

  //Setup Speaker
  pinMode(SPEAKER, OUTPUT);

  // Setup LCD and start the game
  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.setCursor(3, 0);
  lcd.print("WELCOME");
  delay(2000);
  Levels(j); // Initialize the first level
}

void loop() {
  // Check for keypad input
  char key = keypad.getKey();
  if (key != NO_KEY) {
    if (key == '*') { // Clear input buffer
      memset(MembraneInput, 0, sizeof(MembraneInput));
      a = 0;
      clearRow(1);
    } else if (key == '#') { // Check completion
      checkComplete(j);
    } else { // Store input
      MembraneInput[a] = key;
      a++;
      MembraneInput[a] = '\0'; // Null-terminate the string for safety
      lcd.setCursor(0, 1);
      lcd.print(MembraneInput);
      Serial.println(MembraneInput);
    }
  }

   // Read joystick input
  readJoystickInput();

  // Check if time limit is exceeded
  if (millis() - levelStartTime > levelTimeLimit) {
    GameOver();
  }

  //Blue LED time indicating
  blinkBlueLEDsNonBlocking();

  //Speaker function indicating time
  Speaker();

}

