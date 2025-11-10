#include <Arduino.h>

/// Ultrasonic
#define trigPin 8
#define echoPin 9

/// Motor Driver
#define in1 4
#define in2 5
#define in3 6
#define in4 7
#define enA 10
#define enB 11

/// IR Sensorss
int irPins[2] = {A0, A1};   /// [0] = Left, [1] = Right
int IR_THRESHOLD = 500;

/// Encoder Motor
#define encoderLeft 2
#define encoderRight 3
volatile long countLeft = 0;     ///Normally Arduino will remember variable values ​​in its brain (RAM) for speed, but volatitle is a command that says, Don't keep it in youre head, read it again every time."
volatile long countRight = 0;

/// Sensors data
struct SensorData
{
  int irLeft;
  int irRight;
  long distance;
};

/// 1 = Left, 2 = Right
SensorData sensor;
int lastTurn = 0;  

void countLeftISR()
{ countLeft++; }
void countRightISR()
{ countRight++; }

/// all sensor dara
void readSensors(SensorData *data) {
  int valLeft = analogRead(irPins[0]);
  int valRight = analogRead(irPins[1]);

  data->irLeft = (valLeft > IR_THRESHOLD) ? 1 : 0;
  data->irRight = (valRight > IR_THRESHOLD) ? 1 : 0;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  long duration = pulseIn(echoPin, HIGH);
  data->distance = duration * 0.034 / 2;
}

/// Movement
void forward(int spd) {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, spd); analogWrite(enB, spd);
}

void backward(int spd) {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, spd); analogWrite(enB, spd);
}

void turnLeft(int spd) {
  digitalWrite(in1, LOW); digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH); digitalWrite(in4, LOW);
  analogWrite(enA, spd); analogWrite(enB, spd);
}

void turnRight(int spd) {
  digitalWrite(in1, HIGH); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, HIGH);
  analogWrite(enA, spd); analogWrite(enB, spd);
}

void stopMove() {
  digitalWrite(in1, LOW); digitalWrite(in2, LOW);
  digitalWrite(in3, LOW); digitalWrite(in4, LOW);
  analogWrite(enA, 0); analogWrite(enB, 0);
}

/// Find a way
void rotateToFindPath() {
  noInterrupts();     ///Toggles or temmporarily interrupts the Arduino to complete important commands without interruption, as some variables (such as countLeft, countRight) are constantly being modified by the Interrupt from the Encoder, and if we read their values ​​while they are changing, they will be corrupted.
  countLeft = 0;
  countRight = 0;
  interrupts();

  if (lastTurn == 1) { /// if turn left go to right
    turnRight(150);
  } else {
    turnLeft(150);
  }

  long currentL = 0, currentR = 0;
  while (true) {
    noInterrupts();
    currentL = countLeft;
    currentR = countRight;
    interrupts();

    ///
    if (currentL > 60 || currentR > 60) break;
  }

  stopMove();
  lastTurn = (lastTurn == 1) ? 2 : 1;
}

void setup() {
  Serial.begin(9600);

  for (int i = 0; i < 2; i++) pinMode(irPins[i], INPUT);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  pinMode(in1, OUTPUT); pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT); pinMode(in4, OUTPUT);
  pinMode(enA, OUTPUT); pinMode(enB, OUTPUT);

  pinMode(encoderLeft, INPUT_PULLUP);
  pinMode(encoderRight, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(encoderLeft), countLeftISR, RISING);     ///Let the Arduino know that if a signal comes from this pin, it's should immediately execute the specified function.(If a signal comes from this pin, the pulse counter (ISR) function is immediately called.)
  attachInterrupt(digitalPinToInterrupt(encoderRight), countRightISR, RISING);
}

/// Loop
void loop() {
  readSensors(&sensor);

  long tempL, tempR;
  noInterrupts();
  tempL = countLeft;
  tempR = countRight;
  interrupts();

  Serial.print("IR L: "); Serial.print(sensor.irLeft);
  Serial.print(" | IR R: "); Serial.print(sensor.irRight);
  Serial.print(" | Dist: "); Serial.print(sensor.distance);
  Serial.print(" | EncL: "); Serial.print(tempL);
  Serial.print(" | EncR: "); Serial.println(tempR);

  /// avoid dead end
  if (sensor.distance < 15) {
    stopMove();
    backward(150);
    delay(300);
    rotateToFindPath();
  }

  /// if left detect go to right
  else if (sensor.irLeft == 1 && sensor.irRight == 0) {
    countLeft = 0; countRight = 0;
    interrupts();

    turnRight(150);
    long tick = 0;
    while (tick < 30) {
      noInterrupts(); tick = countLeft; interrupts();
    }
    stopMove();
    lastTurn = 2;
  }

  /// if tight detect go to left
  else if (sensor.irRight == 1 && sensor.irLeft == 0) {
    noInterrupts();
    countLeft = 0; countRight = 0;
    interrupts();

    turnLeft(150);
    long tick = 0;
    while (tick < 30) {
      noInterrupts(); tick = countRight; interrupts();
    }
    stopMove();
    lastTurn = 1;
  }

  /// if detect 2 side it will rotate to backward
  else if (sensor.irLeft == 1 && sensor.irRight == 1) {
    backward(150);
    delay(300);
    rotateToFindPath();
  }

  // if nothing, Let it go
  else {
    forward(170);
  }
}