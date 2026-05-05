#include <Arduino_FreeRTOS.h>
#include <EEPROM.h>
#include <semphr.h>



// constants won't change. Used here to
// set pin numbers:
const int ledPin1 = 13;  // the number of the LED pin
const int ledPin2 = 11;
const int inputPin = 2;
const int outputPin = 3;

int fanstate = 0;
int fanspeed = 0;

bool buttonCooldown = false;

// Variables will change:
int ledState1 = LOW;  // ledState used to set the LED
int ledState2 = LOW;
int previousMillis1 = 0;  // will store last time LED was updated
int previousMillis2 = 0;

// the follow variables is a long because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
int interval1 = 1000;  // interval at which to blink (milliseconds)
int interval2 = 700;
bool userAsk = false;
bool userSpeed = false;
int ledSelect = -1;
int ledSpeed = 0;

bool skip = false;

unsigned int currentMillis1 = millis();
unsigned int currentMillis2 = millis();


SemaphoreHandle_t xSerialSemaphore;

int fn0() {
  Serial.print("\n");
  Serial.print("Select which LED you want to change\n");
  userAsk = true;
  delay(1);
  return 0;
}

int fn1() {
  ledSelect = Serial.parseInt();
  Serial.print(ledSelect);
  if (ledSelect == 1 || ledSelect == 2) {
    Serial.print(F("How fast do you want it to blink (ms)"));
    userSpeed = true;
  } else if (ledSelect != 10) {
    Serial.print(F("Invalid input, must be 1 or 2, input again\n"));
  }

  delay(1);
  return 0;
}

int fn2() {
  ledSpeed = Serial.parseInt();
  Serial.print(ledSpeed);
  if (ledSelect == 1 && ledSpeed > -1) {
    interval1 = ledSpeed;
    userSpeed = false;
    userAsk = false;
    EEPROM.put(0,interval1);
  } else if (ledSelect == 2 && ledSpeed > -1) {
    interval2 = ledSpeed;
    userSpeed = false;
    userAsk = false;
    EEPROM.put(1,interval2);
  }

  delay(1); 
  return 0;
}

int fn3() {
  // save the last time you blinked the LED
  previousMillis1 = currentMillis1;

  // if the LED is off turn it on and vice-versa:
  if (ledState1 == LOW)
    ledState1 = HIGH;
  else
    ledState1 = LOW;

  // set the LED with the ledState of the variable:
  digitalWrite(ledPin1, ledState1);
  delay(1);
  return 0;
}

int fn4() {
  // save the last time you blinked the LED
  previousMillis2 = currentMillis2;

  // if the LED is off turn it on and vice-versa:
  if (ledState2 == LOW)
    ledState2 = HIGH;
  else
    ledState2 = LOW;

  // set the LED with the ledState of the variable:
  digitalWrite(ledPin2, ledState2);
delay(1);
  return 0;
}


int fn5(){
  if(fanstate==0){
    fanstate = 1;
    fanspeed = 1;
  }
  else if(fanstate==1){
    fanstate = 2;
    fanspeed = 2;
  }
  else if(fanstate == 2){
    fanstate = 3;
    fanspeed = 3;
  }
  else if(fanstate == 3){
    fanstate = 4;
    fanspeed = 2;
  }
  else if(fanstate == 4){
    fanstate = 5;
    fanspeed = 1;
  }
  else{
    fanstate = 0;
    fanspeed = 0;
  }

  buttonCooldown = true;
  analogWrite(outputPin,fanspeed*(255/3));
  delay(1);
  Serial.print(fanspeed);
  return 0;
}

int fn6(){
  int temp = 5/3;
  analogWrite(outputPin,temp*fanspeed);
  EEPROM.put(2,fanspeed);
  delay(1);
return 0;
}

int fn7(){
  buttonCooldown = false;
delay(1);
  return 0;
}


int (*fnptr[8])() = {fn0,fn1,fn2,fn3,fn4,fn5,fn6,fn7};


void setup() {
  Serial.begin(9600);
  Serial.setTimeout(5000);

  EEPROM.get(0, interval1);
  EEPROM.get(1,interval2);
  EEPROM.get(2,fanspeed);

  Serial.print("Interval 1: ");
  Serial.println(interval1);
  Serial.print("Interval 2: ");
  Serial.println(interval2);
  Serial.print("Fanspeed: ");
  Serial.println(fanspeed);

  if(interval1 < 0 || interval1 > 10000){interval1 = 1000;}
  if(interval2 < 0 || interval2 > 10000){interval2 = 700;}
  if(fanspeed < 0 || fanspeed > 5){fanspeed = 0;}

  // set the digital pin as output:
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  currentMillis1 = millis();
  currentMillis2 = millis();
  if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xSerialSemaphore = xSemaphoreCreateMutex();  // Create a mutex semaphore we will use to manage the Serial Port
    if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  }
  xTaskCreate(MyTask1, "Task2", 100, NULL, 2, NULL);

  xTaskCreate(MyTask2, "Task2", 100, NULL, 2, NULL);

}


void loop() {
  
}


static void MyTask1(void* pvParameters)
{
 
  while(1)
  { 
    if (!userAsk) {
      fnptr[0]();
    }

    if (Serial.available() > 0 && !userSpeed) {
      fnptr[1]();
    }

    if (Serial.available() > 0 && userSpeed) {
      fnptr[2]();
    }


    if (currentMillis1 - previousMillis1 > interval1) {
      fnptr[3]();
        digitalWrite(ledPin2, HIGH);

      
    }

    if (currentMillis2 - previousMillis2 > interval2) {
      fnptr[4]();
    }
    currentMillis1 = millis();
    currentMillis2 = millis();
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }
    vTaskDelay(110/portTICK_PERIOD_MS);
}
    
}



static void MyTask2(void* pvParameters)
{
 
  while(1)

  { 
    if(digitalRead(inputPin) == 1 && !buttonCooldown){
            Serial.print("Pressed Button\n");

      fnptr[5]();
      fnptr[6]();
    
    }

    if(digitalRead(inputPin) == 0 && buttonCooldown){
            Serial.print("Button unpressed");

    fnptr[7]();
        if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
    }

    }
    vTaskDelay(120/portTICK_PERIOD_MS);
  }
}


