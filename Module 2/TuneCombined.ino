#define BLUETOOTH 1

#include <LiquidCrystal.h>
#if BLUETOOTH
  #include <BLEMidi.h>
#endif

const int rs = 14, en = 27, d4 = 26, d5 = 25, d6 = 33, d7 = 32;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

#define JOYSTICK_X_PIN 15
#define JOYSTICK_Y_PIN 12
#define JOYSTICK_BUTTON_PIN 4
#define BUTTON_PIN 2
#define SPEAKER_PIN 16
#define SWITCH_PIN 19

// Forth definitions
short stack[100];
unsigned int sp = 0;

unsigned int loop_ptr = 0;
unsigned int loop_count = 0;
bool defining_loop = false;
bool ignoring_branch = false;

// INPUT HANDLING CODE

String menu_items[] = {
  "scale",
  "octave",
  "rand",
  "nice"
};

String prog_items[] = {
  "220 247 262 294 330 349 392",
  "5 do 2 5 i - pow 131 * loop",
  "131 2 5 0 rand pow *",
  "2 10 do dup dup * loop"
};

const int menu_size = sizeof(menu_items) / sizeof(menu_items[0]);

void setup() {
  pinMode(JOYSTICK_X_PIN, INPUT);
  pinMode(JOYSTICK_Y_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);
  pinMode(SPEAKER_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP);

  Serial.begin(9600);

  #if BLUETOOTH
    BLEMidiServer.begin("TunePlayer");
  #endif
  
  lcd.begin(16, 2);
  navigateIntro();
  navigateMenu();
}

void loop() {}

void navigateIntro() {
  state = INTRO;
  
  for(int i = 0; i < 2; i++) {
    lcd.clear();
    delay(200);
    
    lcd.print(" Choose a tune! ");
    delay(500);
  }

  delay(1000);
  lcd.clear();
}

void navigateMenu() {
  unsigned int menu_ind = 0;
  bool selected = false;
  int joystick_y;

  displayMenu(menu_ind);
  delay(250);

  while(true) {
    do {
      joystick_y = map(analogRead(JOYSTICK_Y_PIN), 0, 4096, -100, 100);
      selected = digitalRead(JOYSTICK_BUTTON_PIN) == LOW;
      delay(10);
    } while(!selected && abs(joystick_y) < 50);
    if(selected) break;

    menu_ind = scrollMenu(menu_ind, joystick_y / abs(joystick_y));
    
    displayMenu(menu_ind);
    
    delay(500);
  }

  navigateTune(menu_ind);
}

void navigateTune(unsigned int menu_ind) {
  unsigned int bpm = 200;
  
  lcd.clear();
  lcd.print(menu_items[menu_ind]);
  lcd.print(": ");
  lcd.setCursor(0, 1);
  lcd.print("bpm: ");
  lcd.print(bpm);
  delay(250);
  
  unsigned int loader_len = 0;
  const unsigned int loader_start = menu_items[menu_ind].length() + 2;
  unsigned int stack_reader = 0;

  while(true) {
    lcd.setCursor(loader_start, 0);
    
    if(digitalRead(BUTTON_PIN) == LOW) {
      loader_len = (loader_len + 1) % (16 - loader_start);
      for(int i = 0; i < 16 - loader_start; i++) {
        lcd.print(i <= loader_len ? "=" : " ");
      }

      if(stack_reader >= sp) {
        sp = 0;
        interpret(prog_items[menu_ind]);
        stack_reader = 0;
      }

      short freq = stack[stack_reader++];
      Serial.print("Freq: ");
      Serial.println(freq);

      int midi = freqToMidi(freq);
      #if BLUETOOTH
        BLEMidiServer.noteOn(0, midi, 127);
      #endif
      if(digitalRead(SWITCH_PIN) == HIGH) tone(SPEAKER_PIN, freq);
      delay(round((60.0 / bpm) * 1000));
      #if BLUETOOTH
        BLEMidiServer.noteOff(0, midi, 127);
      #endif
      noTone(SPEAKER_PIN);
    } else {
      loader_len = 0;
      lcd.setCursor(loader_start, 0);
      for(int i = 0; i < 16 - loader_start; i++) {
        lcd.print(" ");
      }

      if(digitalRead(JOYSTICK_BUTTON_PIN) == LOW) break;

      int joystick_x = map(analogRead(JOYSTICK_X_PIN), 0, 4096, -100, 100);
      if(abs(joystick_x) > 10) {
        bpm = clamp(bpm + (joystick_x / -10), 0, 1000);
      }

      lcd.setCursor(5, 1);
      lcd.print(bpm);
      lcd.print("       ");
      
      sp = 0;
      delay(100);
    }
  }

  sp = 0;
  navigateMenu();
}

void displayMenu(unsigned int menu_ind) {
  state = MENU;

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(menu_items[menu_ind]);
  lcd.print(" +");
  
  if(menu_size - menu_ind >= 2) {
    lcd.setCursor(0, 1);
    lcd.print(menu_items[menu_ind + 1]);
  }
}

unsigned int scrollMenu(unsigned int menu_ind, int dir) {
  int new_ind = menu_ind + dir;
  
  if(new_ind >= 0 && new_ind < menu_size) return new_ind;
  return menu_ind;
}

int clamp(int n, int lower, int upper) {
  return min(max(n, lower), upper);
}

// END OF INPUT CODE

// START OF FORTH INTERPETER

void playStack() {
  for(int i = 0; i < sp; i++) {
    tone(SPEAKER_PIN, stack[i]);
    delay(250);
    noTone(13); 
  }
}

void push(short n) {
  stack[sp++] = n;
}

short pop() {
  return stack[--sp];
}

void interpret(String prog) {
  String token = "";
  unsigned int pc = 0;
  char curr;
  
  while(pc <= prog.length()) {
    if(pc == prog.length()) {
      if(token.length() > 0) {
        interpretToken(token, &pc);
        token = "";
      }
    } else {
      curr = prog[pc];
      if(curr == ' ') {
        if(token.length() > 0) {
          interpretToken(token, &pc);
          token = "";
        }
      } else {
        token += curr;
      }
    }
    pc++;
  }
}

void interpretToken(String token, unsigned int *pc) {
  if(defining_loop && !token.equals("loop")) return;
  if(ignoring_branch && !(token.equals("else") || token.equals("done"))) return;
  
  if(token.equals("+")) {
    short right = pop();
    short left = pop();
    push(left + right);
  } else if(token.equals("-")) {
    short right = pop();
    short left = pop();
    push(left - right);
  } else if(token.equals("*")) {
    short right = pop();
    short left = pop();
    push(left * right);
  } else if(token.equals("/")) {
    short right = pop();
    short left = pop();
    push(left / right);
  } else if(token.equals("%")) {
    short right = pop();
    short left = pop();
    push(left % right);
  } else if(token.equals("=")) {
    short right = pop();
    short left = pop();
    push(left == right);
  } else if(token.equals("!=")) {
    short right = pop();
    short left = pop();
    push(left != right);
  } else if(token.equals("<")) {
    short right = pop();
    short left = pop();
    push(left < right);
  } else if(token.equals("<=")) {
    short right = pop();
    short left = pop();
    push(left <= right);
  } else if(token.equals(">")) {
    short right = pop();
    short left = pop();
    push(left > right);
  } else if(token.equals(">=")) {
    short right = pop();
    short left = pop();
    push(left >= right);
  } else if(token.equals("pow")) {
    short right = pop();
    short left = pop();
    push(pow(left, right));
  } else if(token.equals(".")) {
    short value = pop();
    Serial.print("Printing: ");
    Serial.println(value);
  } else if(token.equals("drop")) {
    pop();
  } else if(token.equals("dup")) {
    Serial.println("Duping");
    short value = pop();
    push(value);
    push(value);
  } else if(token.equals("over")) {
    short top = pop();
    short bottom = pop();
    push(bottom);
    push(top);
    push(bottom);
  } else if(token.equals("rot")) {
    short one = pop();
    short two = pop();
    short three = pop();
    push(one);
    push(two);
    push(three);
  } else if(token.equals("do")) {
    short count = pop();
    loop_count = count;
    loop_ptr = *pc;
    defining_loop = true;
  } else if(token.equals("loop")) {
    if(loop_count-- > 0) (*pc) = loop_ptr;
    defining_loop = false;
  } else if(token.equals("delay")) {
    short ms = pop();
    delay(ms);
  } else if(token.equals("i")) {
    push(loop_count);
  } else if(token.equals("if")) {
    short cond = pop();
    ignoring_branch = cond ? false : true;
  } else if(token.equals("done")) {
    ignoring_branch = false;
  } else if(token.equals("else")) {
    ignoring_branch = !ignoring_branch;
  } else if(token.equals("rand")) {
    short lower = pop();
    short upper = pop();
    push(random(lower, upper));
  } else {
    push(token.toInt());
  }
}

// END OF FORTH INTERPRETER

// START OF MIDI CODE

int freqToMidi(unsigned int freq) {
  float ratio = freq / 440.0;
  return round((12 * log2(ratio)) + 69);
}


// END OF MIDI CODE
