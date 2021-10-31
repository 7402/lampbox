// LampBox
// Copyright (C) 2021 Michael Budiansky. All rights reserved.

const int redPin = 3;
const int greenPin = 5;
const int bluePin = 6;

const int buttonPin = 2;

int buttonDown = false;

unsigned long bounceTime = 0;
#define MIN_BOUNCE_MILLIS 10

int debug = 0;
char debug_previous_command = ' ';
int debug_previous_next_index = 0;
int debug_previous_value = 0;


void setup() {
  Serial.begin(115200);
 
  pinMode(redPin, OUTPUT);
  analogWrite(redPin, 255);
 
  pinMode(greenPin, OUTPUT);
  analogWrite(greenPin, 255);
 
  pinMode(bluePin, OUTPUT);
  analogWrite(bluePin, 255);

  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
 
  digitalWrite(A0, LOW); 
  digitalWrite(A1, LOW); 
  digitalWrite(A2, LOW); 
  digitalWrite(A3, LOW); 
  digitalWrite(A4, LOW); 
  digitalWrite(A5, LOW); 
  
  pinMode(buttonPin, INPUT_PULLUP);

  randomSeed(analogRead(7));
}

unsigned long previous_millis = 0;
unsigned long previous_loop_count = 0;
unsigned long loop_count = 0;
long loops_per_milli = 0;

const int MAX_COMMANDS = 256;
char commands[MAX_COMMANDS];
int values[MAX_COMMANDS];
int next_index = 0;
int end_index = 0;

bool playing = false;
bool recording = false;
int cycle_start_index = 0;
int cycle_count = 0;
int last_cycle = 0;

bool delaying = false;
unsigned long delay_start = 0;
unsigned long delay_length = 0;

int red_value = 0;
int green_value = 0;
int blue_value = 0;

int start_red = 0;
int start_green = 0;
int start_blue = 0;

int end_red = 0;
int end_green = 0;
int end_blue = 0;

bool storing_fade_values = false;
bool fading = false;
unsigned long start_fade = 0;
unsigned long fade_length = 0;

void loop() {
  if (Serial.available()) {
    char command = Serial.read();
    
    while (Serial.available() == 0) ;
    int value = Serial.parseInt();
    
    while (Serial.available() == 0) ;
    char term = Serial.read();

    if (command == 'X' || command == 'x') {
      if (value == 0) {
        playing = false;
        recording = false;
        delaying = false;
        next_index = 0;
        end_index = 0;
        debug_previous_command = ' ';
        debug_previous_next_index = 0;
        debug_previous_value = 0;

        analogWrite(redPin, 255);
        analogWrite(greenPin, 255);
        analogWrite(bluePin, 255);
        
        digitalWrite(A0, LOW); 
        digitalWrite(A1, LOW); 
        digitalWrite(A2, LOW); 
        digitalWrite(A3, LOW); 
        digitalWrite(A4, LOW); 
        digitalWrite(A5, LOW); 
        
      } else {
        last_cycle = 1; 
      }
      
    }

    pushCommand(command, value);

    if (command == 'P' || command == 'p') {
      next_index = end_index - 1;
    }
  }

  if (millis() >= bounceTime + MIN_BOUNCE_MILLIS) {
    if (!digitalRead(buttonPin)) {
      if (!buttonDown) {
        buttonDown = true;
        bounceTime = millis();
        Serial.println("D");
      }
        
    } else {
        if (buttonDown) {
          buttonDown = false;
          bounceTime = millis();
          Serial.println("U");
        }
    }
  }

  if (recording && commands[next_index] != 'P' && commands[next_index] != 'p') {
    // no action
    
  } else if (end_index == 0) {
    // command queue is empty
    
  } else if (next_index < end_index) {
    bool finished = doCommand(commands[next_index], values[next_index]);

    if (finished) next_index++;
    
  } else {
    next_index = 0;
    end_index = 0;
    debug_previous_command = ' ';
    debug_previous_next_index = 0;
    debug_previous_value = 0;
  }

  loop_count++;
  unsigned long this_millis = millis();
  if (this_millis != previous_millis) {
    if (this_millis == previous_millis + 1) {
      loops_per_milli = loop_count - previous_loop_count;
    } else {
      loops_per_milli = -1;  
    }
    
    previous_millis = this_millis;
    previous_loop_count = loop_count;
  }
}

void pushCommand(char command, int value) {
  if (debug) {
    Serial.print(end_index);
    Serial.print(" [");
    Serial.print(command);
    Serial.print(value);
    Serial.println("]");
  }

  if (end_index < MAX_COMMANDS) {
    commands[end_index] = command;
    values[end_index] = value;
    end_index++;
    
  } else {
    // overflow
  }
  
}

bool doCommand(char command, int value) {
  bool finished = true;
  
  if (debug || ((command == 'd' || command == 'D') && value > 0)) {
    if (next_index != debug_previous_next_index || command != debug_previous_command ||
    value != debug_previous_value) {
      Serial.print(next_index);
      Serial.print(" <");
      Serial.print(command);
      Serial.print(value);
      Serial.println(">");

      debug_previous_next_index = next_index;
      debug_previous_command = command;
      debug_previous_value = value;
    }
  }

  int pin = A0;

  switch (value) {
    case 0:   pin = A0;   break;
    case 1:   pin = A1;   break;
    case 2:   pin = A2;   break;
    case 3:   pin = A3;   break;
    case 4:   pin = A4;   break;
    case 5:   pin = A5;   break;
  }

  switch (command) {
    case 'd':
    case 'D':
      debug = value;

      if (value == 2) {
        Serial.print("loops_per_milli = ");
        Serial.println(loops_per_milli);
      }
      break;

    case 'R':
    case 'r':
      if (storing_fade_values) {
        end_red = value;
        
      } else {
        red_value = value;
        analogWrite(redPin, 255 - value);
      }
      break;
      
    case 'G':
    case 'g':
      if (storing_fade_values) {
        end_green = value;
        
      } else {
        green_value = value;
        analogWrite(greenPin, 255 - value);
      }
        break;
      
    case 'B':
    case 'b':
      if (storing_fade_values) {
        end_blue = value;
        
      } else {
        blue_value = value;
        analogWrite(bluePin, 255 - value);
      }
        break;

    case 'S':
    case 's':
      {
        int intensity = random(256);
        switch(random(3)) {
          case 0:
            analogWrite(redPin, intensity);
            analogWrite(greenPin, 255);
            analogWrite(bluePin, 255);
            break;
          
          case 1:
            analogWrite(redPin, 255);
            analogWrite(greenPin, intensity);
            analogWrite(bluePin, 255);
            break;
          
          case 2:
            analogWrite(redPin, 255);
            analogWrite(greenPin, 255);
            analogWrite(bluePin, intensity);
            break;          
        }        
      }
        break;

    case 'F':
    case 'f':
      if (value == 0) {
        storing_fade_values = true;
        start_red = red_value;
        start_green = green_value;
        start_blue = blue_value;
        
      } else if (fading) {
        float percent = float(millis() - start_fade) / fade_length;

        if (percent > 1.0) {
          red_value = end_red;
          analogWrite(redPin, 255 - red_value);
          
          green_value = end_green;
          analogWrite(greenPin, 255 - green_value);
          
          blue_value = end_blue;
          analogWrite(bluePin, 255 - blue_value);

          fading = false;
          
        } else {
          red_value = start_red + (int)round(percent * (end_red - start_red));
          analogWrite(redPin, 255 - red_value);
          
          green_value = start_green + (int)round(percent * (end_green - start_green));
          analogWrite(greenPin, 255 - green_value);
          
          blue_value = start_blue + (int)round(percent * (end_blue - start_blue));
          analogWrite(bluePin, 255 - blue_value);

          finished = false;
       }
        
      } else {
        storing_fade_values = false;
        fading = true;
        start_fade = millis();
        fade_length = value;
        finished = false;        
      }
      break; 
            

    case 'H':
    case 'h':
      digitalWrite(pin, HIGH);
      break; 
            
    case 'L':
    case 'l':
      digitalWrite(pin, LOW); 
      break;

    case 'Q':
    case 'q':
      if (random(2) == 1) {
        digitalWrite(pin, HIGH); 
        
      } else {
        digitalWrite(pin, LOW);         
      }
      break;

    case 'T':
    case 't':
      if (!delaying) {
        delaying = true;
        delay_start = millis();
        delay_length = value;
        finished = false;
        
      } else if (millis() - delay_start < delay_length) {
        finished = false;
        
      } else {
        delaying = false;
      }
      break;

    case 'C':
    case 'c':
        recording = true;
        cycle_start_index = next_index + 1;
      break;

    case 'P':
    case 'p':
      if (recording) {
        // finished recording, begin playing
        // Serial.println("Finished recording");
        recording = false;
        cycle_count = 0;
        last_cycle = value;
        next_index = cycle_start_index;
        finished = false;   // prevent incrementing on return
        playing = true;
        
      } else if (playing) {
        // just finished a cycle
        // Serial.print("Finished cycle "); Serial.println(cycle_count);
        
        cycle_count++;
        if (last_cycle == 0 || cycle_count < last_cycle) {
          // proceed to next cycle
          next_index = cycle_start_index;
          finished = false;   // prevent incrementing on return

        } else {
          // finished all cycles
          // Serial.println("Finished all cycles");
          Serial.println("E");
          
          playing = false;         
        }
        
      } else {
        // spurious P command; ignore
      }
      break;
  }

  return finished;
}
