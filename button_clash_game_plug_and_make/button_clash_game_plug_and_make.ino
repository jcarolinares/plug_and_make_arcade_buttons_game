/*
Button Clash Game - Arduino Plug & Make Kit

Button Clash is an exciting and fun two-player game where the person who pushes the button faster wins! Built it with the Arduino Plug and Make kit.

Created by Julián Caro Linares for Arduino INC

CC-BY-SA
*/

#include "Modulino.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"


// Global definitions
#define BUTTON_A 2
#define BUTTON_B 3
#define LIGHT_BUTTON_A 7
#define LIGHT_BUTTON_B 6

// Modulino objects 
ModulinoButtons buttons; 
ModulinoBuzzer buzzer;
// ModulinoBuzzer buzzer_b(0x0A);


// Global Variables
int skip = 0;
int pitch = 0;
int counter_a = -1; // We start at -1 to simplify matrix control. A +1 is need it to show final result 
int counter_b = -1;

bool push_a = false;
bool push_b = false;

int countdown_first_note = 440;
int countdown_second_note = 880;

int state = 3; // State machine control
char winner = 'z';

// Debounce buttons variables
uint32_t button_a_currentMillis = millis();
uint32_t button_b_currentMillis = millis();
uint32_t button_a_previousMillis = button_a_currentMillis;
uint32_t button_b_previousMillis = button_b_currentMillis;
uint32_t bounce_delay_a = 50; // 50 as default
uint32_t bounce_delay_b = 50; // 50 as default


ArduinoLEDMatrix matrix;

// Matrix Frames definition
uint8_t frame[8][12] = {
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 }
};

uint8_t winner_a[8][12] = {
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 },
  { 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 }
};

uint8_t winner_b[8][12] = {
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 },
  { 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 }
};


const uint32_t happy[] = {
    0x19819,
    0x80000001,
    0x81f8000
};
const uint32_t heart[] = {
    0x3184a444,
    0x44042081,
    0x100a0040
};

const uint32_t arrows_start[] = {
		0x0,
		0x90909a,
		0x5c03f0f,
		66
};


void setup() {
  Serial.begin(115200);
  matrix.begin();

  // Modulinos setup
  Modulino.begin();
  buzzer.begin();
  // buzzer_b.begin();
  buttons.begin();
  buttons.setLeds(true, true, true);

  
  // Arcade Buttons initialization pins
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(LIGHT_BUTTON_A, OUTPUT);
  pinMode(LIGHT_BUTTON_B, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(BUTTON_A), counterA_callback_rising, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_B), counterB_callback_rising, RISING);

  // Buttons Light OFF (Low bright, 5V output)
  digitalWrite(LIGHT_BUTTON_A, LOW);
  digitalWrite(LIGHT_BUTTON_B, LOW);

}

void loop() {

    // State machine
    switch (state)
    {
      case 0: // Game in progress
        buttons.update(); // Modulino buttons reading
        if(push_a == true)
        {
          counter_a++;
          buzzer.tone(440 + pitch, 50);
          digitalWrite(LIGHT_BUTTON_A, HIGH);

          //Matrix mouth count
          button_right_a();
          delay(50);

          push_a = false;
          button_a_previousMillis =  millis();
        }

        if(push_b == true)
        {
          counter_b++;
          buzzer.tone(1240 + pitch, 25);
          digitalWrite(LIGHT_BUTTON_B, HIGH);

          // Matrix mouth count
          button_right_b();
          delay(50);

          push_b = false;
          button_b_previousMillis =  millis();
        }

        digitalWrite(LIGHT_BUTTON_A, LOW);
        digitalWrite(LIGHT_BUTTON_B, LOW);
        delay(10);

        // Interrupt the current game pressing any modulino button
        if (buttons.isPressed(0) || buttons.isPressed(1) || buttons.isPressed(2))
        {
          winner = 'x'; // Game aborted status
          state = 1;
        }
        break;
    
      case 1: // Game End        
        win_animation();
        push_a = false;
        push_b = false;
        state = 3;
        break;

      case 2: // Start Game
        start_countdown();
        state = 0;
        break;

      case 3: // Idle Animation
        buttons.update(); // Modulino buttons reading

        matrix.loadFrame(arrows_start);

        digitalWrite(LIGHT_BUTTON_A, HIGH);
        digitalWrite(LIGHT_BUTTON_B, HIGH);

        while(state == 3)
        {

          buttons.update(); // Modulino buttons reading
          if ((digitalRead(BUTTON_A) == false && digitalRead(BUTTON_B) == false) || (buttons.isPressed(0) || buttons.isPressed(1) || buttons.isPressed(2)))
          {
            push_a = false;
            push_b = false;

            // Matrix clear
            for (int j=0; j<=11;j++){
              for (int i=0; i<=7;i++){
                  frame[i][j] = 0;  
              }
            }
            matrix.renderBitmap(frame, 8, 12);
            delay(100);
            counter_a = -2;
            counter_b = -2;
            state = 2;
          }
          else if(push_a == true)
          {
            buzzer.tone(440 + pitch, 50);
            digitalWrite(LIGHT_BUTTON_A, LOW);
            push_a = false;
            delay(50);
          }
          else if(push_b == true)
          {
            buzzer.tone(1240 + pitch, 25);
            digitalWrite(LIGHT_BUTTON_B, LOW);
            push_b = false; 
            delay(50);
          }

          digitalWrite(LIGHT_BUTTON_A, HIGH);
          digitalWrite(LIGHT_BUTTON_B, HIGH);
          delay(10);
          break;
    }
  }
}

void start_countdown()
{
  buzzer.tone(countdown_first_note + pitch, 500);
  digitalWrite(LIGHT_BUTTON_A, HIGH);
  digitalWrite(LIGHT_BUTTON_B, HIGH);
  delay(500);
  
  digitalWrite(LIGHT_BUTTON_A, LOW);
  digitalWrite(LIGHT_BUTTON_B, LOW);
  delay(500);

  buzzer.tone(countdown_first_note + pitch, 500);
  digitalWrite(LIGHT_BUTTON_A, HIGH);
  digitalWrite(LIGHT_BUTTON_B, HIGH);
  delay(500);
  
  digitalWrite(LIGHT_BUTTON_A, LOW);
  digitalWrite(LIGHT_BUTTON_B, LOW);
  delay(500);

  buzzer.tone(countdown_first_note + pitch, 500);
  digitalWrite(LIGHT_BUTTON_A, HIGH);
  digitalWrite(LIGHT_BUTTON_B, HIGH);
  delay(500);
  
  digitalWrite(LIGHT_BUTTON_A, LOW);
  digitalWrite(LIGHT_BUTTON_B, LOW);
  delay(500);

  buzzer.tone(countdown_second_note + pitch, 1000);
  digitalWrite(LIGHT_BUTTON_A, HIGH);
  digitalWrite(LIGHT_BUTTON_B, HIGH);
  delay(1000);

  digitalWrite(LIGHT_BUTTON_A, LOW);
  digitalWrite(LIGHT_BUTTON_B, LOW);
}

void win_animation()
{
  if(winner == 'a')
  {
    for (int i=0; i<10; i++)
    { 
      digitalWrite(LIGHT_BUTTON_A, HIGH);
      digitalWrite(LIGHT_BUTTON_B, LOW);
      buzzer.tone(440 + pitch, 200);
      delay(200);

      digitalWrite(LIGHT_BUTTON_A, LOW);
      digitalWrite(LIGHT_BUTTON_B, LOW);
      delay(200);
    }
  }
  else if(winner == 'b')
  {
    for (int i=0; i<10; i++)
    {
      digitalWrite(LIGHT_BUTTON_A, LOW);
      digitalWrite(LIGHT_BUTTON_B, HIGH);
      buzzer.tone(1240 + pitch, 200);
      delay(200);

      digitalWrite(LIGHT_BUTTON_A, LOW);
      digitalWrite(LIGHT_BUTTON_B, LOW);
      delay(200);
    }
  }
 else if(winner == 'x') // Game deuce or aborted
  {
    for (int i=0; i<10; i++)
    {
      digitalWrite(LIGHT_BUTTON_A, HIGH);
      digitalWrite(LIGHT_BUTTON_B, HIGH);
      buzzer.tone(440 + pitch, 200);
      delay(200);

      digitalWrite(LIGHT_BUTTON_A, LOW);
      digitalWrite(LIGHT_BUTTON_B, LOW);
      delay(200);
    }
  }
  
}

void counterA_callback_rising()
{
  button_a_currentMillis = millis();
  if (button_a_currentMillis - button_a_previousMillis > bounce_delay_a)
  {
    push_a = true;
  }
}

void counterB_callback_rising()
{
  button_b_currentMillis = millis();
  if (button_b_currentMillis - button_b_previousMillis > bounce_delay_b)
  {
    push_b = true;
  }
}


void button_right_a()
{
  // 96 pixels in total so 48 led or presses for each player, 24 in total with the double press feature
  int aux_counter_a = 0;
  
  if (counter_a >= 48)
  {
    winner = 'a';
    state = 1;
  } 
  else if (counter_a <= 95)
  {
    aux_counter_a = counter_a;
  }

  int aux_col = (aux_counter_a)/8.0;
  int col = abs(aux_col);
  col = (col != 0) ? col-- : 0;
  int aux_row = aux_counter_a%8;
  int row = aux_row;

  frame[row][col] = 1;
  matrix.renderBitmap(frame, 8, 12);
}

void button_right_b()
{
  // 96 pixels in total so 48 led or presses for each player, 24 in total with the double press feature
  int aux_counter_b = 0;

  if (counter_b >= 48)
  {
    winner = 'b';
    state = 1;
  }
  else if (counter_b <= 95)
  {
    aux_counter_b = counter_b;
  }

  int aux_col = (aux_counter_b)/8.0;
  int col = abs(aux_col);
  col = (col != 0) ? col-- : 0;
  int aux_row = aux_counter_b%8;
  int row = aux_row;

  frame[7-row][11-col] = 1;
  matrix.renderBitmap(frame, 8, 12);
}