
/**
 * Lockitron Test
 * Shawn Hymel @ SparkFun Electronics
 * May 18, 2015
 *
 * Lockitron red motor wire -> A01
 * Lockitron black motor wire -> A02
 */
 
#include <RFduinoBLE.h>
 
#define DEBUG 0

// Constants
const uint8_t LOCK_OPEN = 0;
const uint8_t LOCK_CLOSED = 1;
const uint8_t MSG_LOCK = 0x10;
const uint8_t MSG_UNLOCK = 0x11;
const uint8_t MSG_STATE_REQ = 0x12;

// Motor speed and direction definitions
const uint8_t MOTOR_SPEED = 200;
const uint8_t MOTOR_CW = 0;
const uint8_t MOTOR_CCW = 1;

// Pin definitions
const uint8_t AIN1_PIN = 0;
const uint8_t AIN2_PIN = 1;
const uint8_t PWMA_PIN = 2;
const uint8_t SW_1A_PIN = 3;
const uint8_t SW_1B_PIN = 4;
const uint8_t SW_2A_PIN = 5;
const uint8_t SW_2B_PIN = 6;

// Switch state variables
uint8_t sw_1a;
uint8_t sw_1b;
uint8_t sw_2a;
uint8_t sw_2b;

// Lock state
uint8_t lock_state;

void setup()
{
#if DEBUG
  Serial.begin(9600);
#else
  // Set up motor pins
  pinMode(AIN1_PIN, OUTPUT);
  pinMode(AIN2_PIN, OUTPUT);
  pinMode(PWMA_PIN, OUTPUT);
#endif
  
  // Set up switch pins
  pinMode(SW_1A_PIN, INPUT_PULLUP);
  pinMode(SW_1B_PIN, INPUT_PULLUP);
  pinMode(SW_2A_PIN, INPUT_PULLUP);
  pinMode(SW_2B_PIN, INPUT_PULLUP);
  
  // Reset the lock to unlocked position
  resetLock();
  lock_state = LOCK_OPEN;
  
  // Start BLE
  RFduinoBLE.advertisementData = "lock";
  RFduinoBLE.begin();
}

void loop()
{ 
#if DEBUG

  // Read pins
  sw_1a = digitalRead(SW_1A_PIN);
  sw_1b = digitalRead(SW_1B_PIN);
  sw_2a = digitalRead(SW_2A_PIN);
  sw_2b = digitalRead(SW_2B_PIN);

  // Print results
  Serial.print("1A:");
  Serial.print(sw_1a);
  Serial.print(" 1B:");
  Serial.print(sw_1b);
  Serial.print(" 2A:");
  Serial.print(sw_2a);
  Serial.print(" 2B:");
  Serial.print(sw_2b);
  Serial.println();
  
  delay(100);
  
#else
  // Stay in low power mode
  //RFduinoBLE_ULPDelay(INFINITE);
#endif
}

void RFduinoBLE_onReceive(char *data, int len)
{
#if DEBUG
  Serial.print("BLE recv: ");
  Serial.println(data[0]);
#endif

  // Parse the message
  switch ( data[0] )
  {
    case MSG_LOCK:
#if DEBUG
      Serial.println("Locking");
#else
      lock();
#endif
      break;
    case MSG_UNLOCK:
#if DEBUG
      Serial.println("Unlocking");
#else
      unlock();
#endif
      break;
    case MSG_STATE_REQ:
#if DEBUG
      Serial.print("Sending state ");
      Serial.println(lock_state);
#endif
      RFduinoBLE.send(lock_state);
      break;
    default:
      break;
  }
}

void resetLock()
{
  // Move motor to reset its position
  moveMotor(MOTOR_SPEED, MOTOR_CCW);
  do
  {
    sw_1a = digitalRead(SW_1A_PIN);
    sw_1b = digitalRead(SW_1B_PIN);
    sw_2a = digitalRead(SW_2A_PIN);
    sw_2b = digitalRead(SW_2B_PIN);
  }
  while ( !((sw_2a == 1) && (sw_2b == 1)));
  stopMotor();
}

void lock()
{
  // Move motor to lock the deadbolt
  moveMotor(MOTOR_SPEED, MOTOR_CW);
  do
  {
    sw_1a = digitalRead(SW_1A_PIN);
    sw_1b = digitalRead(SW_1B_PIN);
    sw_2a = digitalRead(SW_2A_PIN);
    sw_2b = digitalRead(SW_2B_PIN);
  }
  while ( !((sw_1a == 0) && (sw_1b == 1) && 
            (sw_2a == 0) && (sw_2b == 1)) );
  stopMotor();
  delay(100);
  
  // Move motor back to starting position
  moveMotor(MOTOR_SPEED, MOTOR_CCW);
  do
  {
    sw_1a = digitalRead(SW_1A_PIN);
    sw_1b = digitalRead(SW_1B_PIN);
    sw_2a = digitalRead(SW_2A_PIN);
    sw_2b = digitalRead(SW_2B_PIN);
  }
  while ( !((sw_2a == 1) && (sw_2b == 1)) );
  stopMotor();
  lock_state = LOCK_OPEN;
}

void unlock()
{
  // Move motor to lock the deadbolt
  moveMotor(MOTOR_SPEED, MOTOR_CCW);
  do
  {
    sw_1a = digitalRead(SW_1A_PIN);
    sw_1b = digitalRead(SW_1B_PIN);
    sw_2a = digitalRead(SW_2A_PIN);
    sw_2b = digitalRead(SW_2B_PIN);
  }
  while ( !((sw_1a == 1) && (sw_1b == 0) && 
            (sw_2a == 1) && (sw_2b == 0) ));
  stopMotor();
  delay(100);
  
  // Move motor back to starting position
  moveMotor(MOTOR_SPEED, MOTOR_CW);
  do
  {
    sw_1a = digitalRead(SW_1A_PIN);
    sw_1b = digitalRead(SW_1B_PIN);
    sw_2a = digitalRead(SW_2A_PIN);
    sw_2b = digitalRead(SW_2B_PIN);
  }
  while ( !((sw_2a == 1) && (sw_2b == 1)) );
  stopMotor();
  lock_state = LOCK_CLOSED;
}

void moveMotor(uint8_t spd, uint8_t dir)
{
  boolean ain1;
  boolean ain2;
  
  // Define direction pins
  if ( dir )
  {
    ain1 = HIGH;
    ain2 = LOW;
  } 
  else
  {
    ain1 = LOW;
    ain2 = HIGH;
  }
  
  // Set motor to GO!
  digitalWrite(AIN1_PIN, ain1);
  digitalWrite(AIN2_PIN, ain2);
  analogWrite(PWMA_PIN, spd);
}

void stopMotor()
{
  analogWrite(PWMA_PIN, 0);
}
