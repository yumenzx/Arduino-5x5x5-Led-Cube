#include <TimerOne.h>

#define layer1 47
#define layer2 48
#define layer3 49
#define layer4 50
#define layer5 51

#define button1 21
#define button2 20
#define ledPin 53

#define DELAY_TIME_BETWEEN_TRANSITIONS 400
#define DELAY_TIME DELAY_TIME_BETWEEN_TRANSITIONS
#define DELAY_TIME_BETWEEN_LETTERS 750

#define DEFAULT_ANIMATION_TIME 10

#define NO_INTERRUPT_CONDITION 0
#define ANIMATION_SUCCESS 0
#define STOP 1
#define CHANGE_ANIMATION 2
#define BLUETOOTH_MESSAGE 3

#define NR_OF_ANIMATIONS 8

#define SET(reg, pin) (reg |= _BV(pin))
#define CLR(reg, pin) (reg &= ~(_BV(pin)))

volatile bool shouldRun;
volatile bool shouldChangeAnimation;
volatile uint8_t currentAnimation;

volatile bool multiplexLayers;
volatile uint8_t activeLayer;
volatile uint8_t l1, l2, l3, l4, l5;

void setup() {
  Serial1.begin(9600);

  // initialize variables
  shouldRun = false;
  shouldChangeAnimation = false;
  currentAnimation = 1;
  multiplexLayers = false;

  // initialize timer
  Timer1.initialize(3200);
  Timer1.attachInterrupt(update);

  pinMode(ledPin, OUTPUT);

  // interrupt buttons
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  digitalWrite(button1, HIGH);
  digitalWrite(button2, HIGH);
  attachInterrupt(digitalPinToInterrupt(button1), changeActiveState, RISING);
  attachInterrupt(digitalPinToInterrupt(button2), changeCurrentAnimation, RISING);

  for (uint8_t i = 22; i <= 51; pinMode(i++, OUTPUT))
    ;
  SET(PORTB, 0);
}

void loop() {
  uint8_t state;
  if (shouldRun) {
    CLR(PORTB, 0);
    for (uint8_t current_animation = 1; current_animation <= NR_OF_ANIMATIONS; current_animation++) {
      state = playAnimation(current_animation, DEFAULT_ANIMATION_TIME);
      if (state != ANIMATION_SUCCESS)
        goto process_state;
    }

    goto done;

process_state:
    disableAllLayers();
    setAllInactive();

    switch (state) {
      case STOP:
        goto done;
      case CHANGE_ANIMATION:
        currentAnimation = currentAnimation == NR_OF_ANIMATIONS ? 1 : (currentAnimation + 1);
        shouldChangeAnimation = false;

        state = playAnimation(currentAnimation, 2*DEFAULT_ANIMATION_TIME);
        if (state != ANIMATION_SUCCESS)
          goto process_state;
        break;
      case BLUETOOTH_MESSAGE:
        state = processBluetoothMessage();
        if (state != ANIMATION_SUCCESS)
          goto process_state;
        break;
    }

  } else if (Serial1.available()) {
    processBluetoothMessage();
  }

done:;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
void changeActiveState() {
  shouldRun = not shouldRun;
}

void changeCurrentAnimation() {
  shouldChangeAnimation = true;
}

void update() {
  if (multiplexLayers == false)
    return;

  activeLayer = activeLayer == layer5 ? layer1 : (activeLayer + 1);
  setActiveLayer(activeLayer);
  switch (activeLayer) {
    case layer1:
      (l1 >> 4) & 1 == 1 ? SET(PORTA, 0) : CLR(PORTA, 0);
      (l1 >> 3) & 1 == 1 ? SET(PORTA, 5) : CLR(PORTA, 5);
      (l1 >> 2) & 1 == 1 ? SET(PORTC, 5) : CLR(PORTC, 5);
      (l1 >> 1) & 1 == 1 ? SET(PORTC, 0) : CLR(PORTC, 0);
      (l1 >> 0) & 1 == 1 ? SET(PORTL, 7) : CLR(PORTL, 7);
      break;
    case layer2:
      (l2 >> 4) & 1 == 1 ? SET(PORTA, 0) : CLR(PORTA, 0);
      (l2 >> 3) & 1 == 1 ? SET(PORTA, 5) : CLR(PORTA, 5);
      (l2 >> 2) & 1 == 1 ? SET(PORTC, 5) : CLR(PORTC, 5);
      (l2 >> 1) & 1 == 1 ? SET(PORTC, 0) : CLR(PORTC, 0);
      (l2 >> 0) & 1 == 1 ? SET(PORTL, 7) : CLR(PORTL, 7);
      break;
    case layer3:
      (l3 >> 4) & 1 == 1 ? SET(PORTA, 0) : CLR(PORTA, 0);
      (l3 >> 3) & 1 == 1 ? SET(PORTA, 5) : CLR(PORTA, 5);
      (l3 >> 2) & 1 == 1 ? SET(PORTC, 5) : CLR(PORTC, 5);
      (l3 >> 1) & 1 == 1 ? SET(PORTC, 0) : CLR(PORTC, 0);
      (l3 >> 0) & 1 == 1 ? SET(PORTL, 7) : CLR(PORTL, 7);
      break;
    case layer4:
      (l4 >> 4) & 1 == 1 ? SET(PORTA, 0) : CLR(PORTA, 0);
      (l4 >> 3) & 1 == 1 ? SET(PORTA, 5) : CLR(PORTA, 5);
      (l4 >> 2) & 1 == 1 ? SET(PORTC, 5) : CLR(PORTC, 5);
      (l4 >> 1) & 1 == 1 ? SET(PORTC, 0) : CLR(PORTC, 0);
      (l4 >> 0) & 1 == 1 ? SET(PORTL, 7) : CLR(PORTL, 7);
      break;
    case layer5:
      (l5 >> 4) & 1 == 1 ? SET(PORTA, 0) : CLR(PORTA, 0);
      (l5 >> 3) & 1 == 1 ? SET(PORTA, 5) : CLR(PORTA, 5);
      (l5 >> 2) & 1 == 1 ? SET(PORTC, 5) : CLR(PORTC, 5);
      (l5 >> 1) & 1 == 1 ? SET(PORTC, 0) : CLR(PORTC, 0);
      (l5 >> 0) & 1 == 1 ? SET(PORTL, 7) : CLR(PORTL, 7);
      break;
  }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t processBluetoothMessage() {
  String message = "";
  message.reserve(64);

  while (Serial1.available()) {
    char chr = (char)Serial1.read();
    message += chr;
    delay(10);
  }
  // last 2 chars of message is chars 13 and 10
  message[message.length() - 2] = '\0';
  message.toUpperCase();

  if (message == "#OFF") {
    shouldRun = false;
  } else if (message == "#ON") {
    shouldRun = true;
  } else if (message[0] == '#') {
    uint8_t wantedAnimation = message[1] - '0';
    if (wantedAnimation > 0 && wantedAnimation <= NR_OF_ANIMATIONS)
      return playAnimation(wantedAnimation, 2*DEFAULT_ANIMATION_TIME);
  } else {
    SET(PORTB, 0);
    multiplexLayers = true;
    for (uint8_t i = 0; i < message.length() - 2; i++) {
      mapCharacter(message[i]);
      delay(DELAY_TIME_BETWEEN_LETTERS);
    }
    multiplexLayers = false;
    disableAllLayers();
    CLR(PORTB, 0);
  }

  return ANIMATION_SUCCESS;
}

uint8_t playAnimation(uint8_t animation, uint8_t duration) {
  unsigned long startTime = millis();
  unsigned long elapsedTime;
  uint8_t state;

  currentAnimation = animation;
  do {
    switch (animation) {
      case 1:
        state = animation1();
        break;
      case 2:
        state = animation2();
        break;
      case 3:
        state = animation3();
        break;
      case 4:
        state = animation4();
        break;
      case 5:
        state = animation5();
        break;
      case 6:
        state = animation6();
        break;
      case 7:
        state = animation7();
        break;
      case 8:
        state = animation8();
        break;
      default:
        break;
    }
    if (state != ANIMATION_SUCCESS)
      return state;
    elapsedTime = millis();
  } while (elapsedTime < startTime + duration * 1000);

  return ANIMATION_SUCCESS;
}

uint8_t verifyConditions() {
  if (shouldRun == false)
    return STOP;

  if (shouldChangeAnimation)
    return CHANGE_ANIMATION;

  if (Serial1.available())
    return BLUETOOTH_MESSAGE;

  return NO_INTERRUPT_CONDITION;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
void setActiveLayer(uint8_t layer) {
  layer == layer1 ? SET(PORTL, 2) : CLR(PORTL, 2);
  layer == layer2 ? SET(PORTL, 1) : CLR(PORTL, 1);
  layer == layer3 ? SET(PORTL, 0) : CLR(PORTL, 0);
  layer == layer4 ? SET(PORTB, 3) : CLR(PORTB, 3);
  layer == layer5 ? SET(PORTB, 2) : CLR(PORTB, 2);
}

void setAllActive() {
  for (uint8_t i = 22; i < 47; digitalWrite(i++, HIGH))
    ;
}

void setAllInactive() {
  for (uint8_t i = 22; i < 47; digitalWrite(i++, LOW))
    ;
}

void enableAllLayers() {
  SET(PORTL, 2);
  SET(PORTL, 1);
  SET(PORTL, 0);
  SET(PORTB, 3);
  SET(PORTB, 2);
}

void disableAllLayers() {
  //for (uint8_t layer = layer1; layer <= layer5; digitalWrite(layer++, LOW));
  CLR(PORTL, 2);
  CLR(PORTL, 1);
  CLR(PORTL, 0);
  CLR(PORTB, 3);
  CLR(PORTB, 2);
}

uint8_t arrayToPin(uint8_t row, uint8_t column) {
  return row * 5 + column + 22;
}



////////////////////////////////////////////////////////////////////////////////////////////////////
uint8_t animation1() {

  uint8_t state;
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;

  setAllActive();
  uint8_t current_layer;

  for (current_layer = layer1; current_layer <= layer5; current_layer++) {
    setActiveLayer(current_layer);
    delay(DELAY_TIME);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (current_layer = layer4; current_layer >= layer2; current_layer--) {
    setActiveLayer(current_layer);
    delay(DELAY_TIME);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }

  setAllInactive();
  disableAllLayers();

  return ANIMATION_SUCCESS;
}

uint8_t animation2() {
  uint8_t state;
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;

  setAllActive();

  uint8_t current_layer;

  for (current_layer = layer1; current_layer <= layer5; current_layer++) {
    digitalWrite(current_layer, HIGH);
    delay(DELAY_TIME);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (current_layer = layer5; current_layer >= layer2; current_layer--) {
    digitalWrite(current_layer, LOW);
    delay(DELAY_TIME);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }

  setAllInactive();
  disableAllLayers();

  return ANIMATION_SUCCESS;
}

uint8_t animation3() {

  uint8_t state;
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;

  setAllInactive();
  int8_t i;
  enableAllLayers();

  for (i = 0; i < 5; i++) {
    digitalWrite(arrayToPin(0, i), HIGH);
    delay(DELAY_TIME / 5);
    digitalWrite(arrayToPin(0, i), LOW);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (i = 1; i < 5; i++) {
    digitalWrite(arrayToPin(i, 4), HIGH);
    delay(DELAY_TIME / 5);
    digitalWrite(arrayToPin(i, 4), LOW);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (i = 3; i >= 0; i--) {
    digitalWrite(arrayToPin(4, i), HIGH);
    delay(DELAY_TIME / 5);
    digitalWrite(arrayToPin(4, i), LOW);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (i = 3; i >= 1; i--) {
    digitalWrite(arrayToPin(i, 0), HIGH);
    delay(DELAY_TIME / 5);
    digitalWrite(arrayToPin(i, 0), LOW);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }

  setAllInactive();
  disableAllLayers();

  return ANIMATION_SUCCESS;
}

uint8_t animation4() {

  uint8_t state;
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;

  setAllInactive();
  int8_t i;
  enableAllLayers();

  for (i = 0; i < 5; i++) {
    digitalWrite(arrayToPin(0, i), HIGH);
    delay(DELAY_TIME / 4);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (i = 1; i < 5; i++) {
    digitalWrite(arrayToPin(i, 4), HIGH);
    delay(DELAY_TIME / 4);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }

  for (i = 3; i >= 0; i--) {
    digitalWrite(arrayToPin(4, i), HIGH);
    delay(DELAY_TIME / 4);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (i = 3; i > 0; i--) {
    digitalWrite(arrayToPin(i, 0), HIGH);
    delay(DELAY_TIME / 4);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  digitalWrite(arrayToPin(1, 1), HIGH);
  delay(DELAY_TIME / 4);
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;
  digitalWrite(arrayToPin(1, 2), HIGH);
  delay(DELAY_TIME / 4);
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;
  digitalWrite(arrayToPin(1, 3), HIGH);
  delay(DELAY_TIME / 4);
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;
  digitalWrite(arrayToPin(2, 3), HIGH);
  delay(DELAY_TIME / 4);
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;
  digitalWrite(arrayToPin(3, 3), HIGH);
  delay(DELAY_TIME / 4);
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;
  digitalWrite(arrayToPin(3, 2), HIGH);
  delay(DELAY_TIME / 4);
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;
  digitalWrite(arrayToPin(3, 1), HIGH);
  delay(DELAY_TIME / 4);
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;
  digitalWrite(arrayToPin(2, 1), HIGH);
  delay(DELAY_TIME / 4);
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;
  digitalWrite(arrayToPin(2, 2), HIGH);
  delay(DELAY_TIME / 4);
  state = verifyConditions();
  if (state != NO_INTERRUPT_CONDITION)
    return state;

  setAllInactive();
  disableAllLayers();

  return ANIMATION_SUCCESS;
}

uint8_t animation5() {
  uint8_t state;

  enableAllLayers();
  setAllActive();

  uint8_t current_layer;

  for (uint8_t i = 0; i < 5; i++) {
    digitalWrite(layer5, LOW);
    digitalWrite(layer1, HIGH);
    delay(DELAY_TIME / 4);
    digitalWrite(layer4, LOW);
    digitalWrite(layer5, HIGH);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;

    delay(DELAY_TIME / 4);
    digitalWrite(layer3, LOW);
    digitalWrite(layer4, HIGH);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;

    delay(DELAY_TIME / 4);
    digitalWrite(layer2, LOW);
    digitalWrite(layer3, HIGH);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
    delay(DELAY_TIME / 4);
    digitalWrite(layer1, LOW);
    digitalWrite(layer2, HIGH);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
    delay(DELAY_TIME / 4);
  }

  setAllInactive();
  disableAllLayers();

  return ANIMATION_SUCCESS;
}

uint8_t animation6() {
  uint8_t state;

  for (uint8_t i = 0; i < 3; i++) {
    setAllInactive();
    for (uint8_t j = 0; j < 5; j++) {
      digitalWrite(arrayToPin(0, j), HIGH);
      digitalWrite(arrayToPin(1, j), HIGH);
      digitalWrite(arrayToPin(2, j), HIGH);
      digitalWrite(arrayToPin(3, j), HIGH);
      digitalWrite(arrayToPin(4, j), HIGH);
      if (j == 0) {
        digitalWrite(layer1, HIGH);
        digitalWrite(layer2, HIGH);
        digitalWrite(layer3, HIGH);
        digitalWrite(layer4, HIGH);
        digitalWrite(layer5, HIGH);
      }
      delay(DELAY_TIME/4);
      state = verifyConditions();
      if (state != NO_INTERRUPT_CONDITION)
        return state;
      digitalWrite(arrayToPin(0, j), LOW);
      digitalWrite(arrayToPin(1, j), LOW);
      digitalWrite(arrayToPin(2, j), LOW);
      digitalWrite(arrayToPin(3, j), LOW);
      digitalWrite(arrayToPin(4, j), LOW);
    }
  }

  setAllInactive();

  return ANIMATION_SUCCESS;
}

uint8_t animation7() {
  uint8_t state;

  enableAllLayers();
  setAllInactive();

  int8_t i;
  for (i = 0; i < 5; i++) {
    digitalWrite(arrayToPin(i, 0), HIGH);
    delay(DELAY_TIME / 4);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (i = 4; i >= 0; i--) {
    digitalWrite(arrayToPin(i, 1), HIGH);
    delay(DELAY_TIME / 4);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (i = 0; i < 5; i++) {
    digitalWrite(arrayToPin(i, 2), HIGH);
    delay(DELAY_TIME / 4);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (i = 4; i >= 0; i--) {
    digitalWrite(arrayToPin(i, 3), HIGH);
    delay(DELAY_TIME / 4);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }
  for (i = 0; i < 5; i++) {
    digitalWrite(arrayToPin(i, 4), HIGH);
    delay(DELAY_TIME / 4);
    state = verifyConditions();
    if (state != NO_INTERRUPT_CONDITION)
      return state;
  }

  return ANIMATION_SUCCESS;
}

uint8_t animation8() {
  uint8_t state;

  setAllInactive();
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t j = 0; j < 5; j++) {
      digitalWrite(arrayToPin(0, j), HIGH);
      digitalWrite(arrayToPin(1, j), HIGH);
      digitalWrite(arrayToPin(2, j), HIGH);
      digitalWrite(arrayToPin(3, 4 - j), HIGH);
      digitalWrite(arrayToPin(4, 4 - j), HIGH);

      if (j == 0) {
        digitalWrite(layer5, HIGH);
        digitalWrite(layer1, HIGH);
        digitalWrite(layer2, HIGH);
        digitalWrite(layer3, HIGH);
        digitalWrite(layer4, HIGH);
      }

      delay(DELAY_TIME / 2);
      state = verifyConditions();
      if (state != NO_INTERRUPT_CONDITION)
        return state;
      digitalWrite(arrayToPin(0, j), LOW);
      digitalWrite(arrayToPin(1, j), LOW);
      digitalWrite(arrayToPin(2, j), LOW);
      digitalWrite(arrayToPin(3, 4 - j), LOW);
      digitalWrite(arrayToPin(4, 4 - j), LOW);
    }
  }

  setAllInactive();
  return ANIMATION_SUCCESS;
}

void mapCharacter(char c) {
  switch (c) {
    case 'A':
      l5 = 0b01110;
      l4 = 0b10001;
      l3 = 0b11111;
      l2 = 0b10001;
      l1 = 0b10001;
      break;
    case 'B':
      l5 = 0b11110;
      l4 = 0b10001;
      l3 = 0b11110;
      l2 = 0b10001;
      l1 = 0b11110;
      break;
    case 'C':
      l5 = 0b01110;
      l4 = 0b10001;
      l3 = 0b10000;
      l2 = 0b10001;
      l1 = 0b01110;
      break;
    case 'D':
      l5 = 0b11110;
      l4 = 0b10001;
      l3 = 0b10001;
      l2 = 0b10001;
      l1 = 0b11110;
      break;
    case 'E':
      l5 = 0b11111;
      l4 = 0b10000;
      l3 = 0b11110;
      l2 = 0b10000;
      l1 = 0b11111;
      break;
    case 'F':
      l5 = 0b11111;
      l4 = 0b10000;
      l3 = 0b11110;
      l2 = 0b10000;
      l1 = 0b10000;
      break;
    case 'G':
      l5 = 0b01111;
      l4 = 0b10000;
      l3 = 0b10111;
      l2 = 0b10001;
      l1 = 0b01111;
      break;
    case 'H':
      l5 = 0b10001;
      l4 = 0b10001;
      l3 = 0b11111;
      l2 = 0b10001;
      l1 = 0b10001;
      break;
    case 'I':
      l5 = 0b11111;
      l4 = 0b00100;
      l3 = 0b00100;
      l2 = 0b00100;
      l1 = 0b11111;
      break;
    case 'J':
      l5 = 0b11111;
      l4 = 0b00010;
      l3 = 0b00010;
      l2 = 0b10010;
      l1 = 0b01100;
      break;
    case 'K':
      l5 = 0b10001;
      l4 = 0b10010;
      l3 = 0b11100;
      l2 = 0b10010;
      l1 = 0b10001;
      break;
    case 'L':
      l5 = 0b10000;
      l4 = 0b10000;
      l3 = 0b10000;
      l2 = 0b10000;
      l1 = 0b11111;
      break;
    case 'M':
      l5 = 0b10001;
      l4 = 0b11011;
      l3 = 0b10101;
      l2 = 0b10001;
      l1 = 0b10001;
      break;
    case 'N':
      l5 = 0b10001;
      l4 = 0b11001;
      l3 = 0b10101;
      l2 = 0b10011;
      l1 = 0b10001;
      break;
    case 'O':
      l5 = 0b01110;
      l4 = 0b10001;
      l3 = 0b10001;
      l2 = 0b10001;
      l1 = 0b01110;
      break;
    case 'P':
      l5 = 0b11110;
      l4 = 0b10001;
      l3 = 0b11110;
      l2 = 0b10000;
      l1 = 0b10000;
      break;
    case 'Q':
      l5 = 0b01110;
      l4 = 0b10001;
      l3 = 0b10101;
      l2 = 0b10010;
      l1 = 0b01101;
      break;
    case 'R':
      l5 = 0b11110;
      l4 = 0b10001;
      l3 = 0b11110;
      l2 = 0b10010;
      l1 = 0b10001;
      break;
    case 'S':
      l5 = 0b01111;
      l4 = 0b10000;
      l3 = 0b01110;
      l2 = 0b00001;
      l1 = 0b11110;
      break;
    case 'T':
      l5 = 0b11111;
      l4 = 0b00100;
      l3 = 0b00100;
      l2 = 0b00100;
      l1 = 0b00100;
      break;
    case 'U':
      l5 = 0b10001;
      l4 = 0b10001;
      l3 = 0b10001;
      l2 = 0b10001;
      l1 = 0b01110;
      break;
    case 'V':
      l5 = 0b10001;
      l4 = 0b01010;
      l3 = 0b01010;
      l2 = 0b00100;
      l1 = 0b00100;
      break;
    case 'W':
      l5 = 0b10001;
      l4 = 0b10101;
      l3 = 0b10101;
      l2 = 0b01010;
      l1 = 0b01010;
      break;
    case 'X':
      l5 = 0b10001;
      l4 = 0b01010;
      l3 = 0b00100;
      l2 = 0b01010;
      l1 = 0b10001;
      break;
    case 'Y':
      l5 = 0b10001;
      l4 = 0b01010;
      l3 = 0b00100;
      l2 = 0b00100;
      l1 = 0b00100;
      break;
    case 'Z':
      l5 = 0b11111;
      l4 = 0b00010;
      l3 = 0b00100;
      l2 = 0b01000;
      l1 = 0b11111;
      break;
    case ' ':
      l5 = 0b00000;
      l4 = 0b00000;
      l3 = 0b00000;
      l2 = 0b00000;
      l1 = 0b11111;
      break;
    case '0':
      l5 = 0b01110;
      l4 = 0b10001;
      l3 = 0b10001;
      l2 = 0b10001;
      l1 = 0b01110;
      break;
    case '1':
      l5 = 0b01100;
      l4 = 0b00100;
      l3 = 0b00100;
      l2 = 0b00100;
      l1 = 0b01110;
      break;
    case '2':
      l5 = 0b01110;
      l4 = 0b10001;
      l3 = 0b00010;
      l2 = 0b00100;
      l1 = 0b11111;
      break;
    case '3':
      l5 = 0b11110;
      l4 = 0b00001;
      l3 = 0b01111;
      l2 = 0b00001;
      l1 = 0b11110;
      break;
    case '4':
      l5 = 0b10001;
      l4 = 0b10001;
      l3 = 0b01111;
      l2 = 0b00001;
      l1 = 0b00001;
      break;
    case '5':
      l5 = 0b01111;
      l4 = 0b10000;
      l3 = 0b11111;
      l2 = 0b00001;
      l1 = 0b11111;
      break;
    case '6':
      l5 = 0b01111;
      l4 = 0b10000;
      l3 = 0b11110;
      l2 = 0b10001;
      l1 = 0b01110;
      break;
    case '7':
      l5 = 0b01111;
      l4 = 0b00001;
      l3 = 0b00010;
      l2 = 0b00100;
      l1 = 0b01000;
      break;
    case '8':
      l5 = 0b01110;
      l4 = 0b10001;
      l3 = 0b01110;
      l2 = 0b10001;
      l1 = 0b01110;
      break;
    case '9':
      l5 = 0b01110;
      l4 = 0b10001;
      l3 = 0b01111;
      l2 = 0b00001;
      l1 = 0b11110;
      break;
    default:
      l1 = 0b01110;
      l2 = 0b10001;
      l3 = 0b00110;
      l4 = 0b00000;
      l5 = 0b00100;
      break;
  }
}