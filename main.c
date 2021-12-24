#include <stdio.h>
#include <stdlib.h>

// #include "mbed.h"
// #include "Servo.h"

Servo servo_vertical_legA(p21);
Servo servo_horizontal_legA(p22);
Servo servo_vertical_legB(p23);
Servo servo_horizontal_legB(p24);
Servo servo_vertical_legC(p25);
Servo servo_horizontal_legC(p26);


void (*FunctionPointer)();

void State_Initial();
void State_Forward();
void State_Backward();
void State_RotateRight();
void State_RotateLeft();
void State_Null();

void State_Initial()
{
  printf("Inicio\n");
  FunctionPointer = State_Forward;
}

void State_Forward()
{
  printf("Andar para frente\n");

  for(float i = 0; i < 1; i+=0.05)
  {
    servo_vertical_legA     = i;
    servo_vertical_legB     = i;
    servo_vertical_legC     = i;
    wait(0.1);
    servo_horizontal_legA   = i;
    servo_horizontal_legB   = i;
    servo_horizontal_legC   = i;
    wait(0.1);

  }

}

void State_Backward()
{
  for(float i = 1; i < 1; i-=0.05)
  {
    servo_vertical_legA     = i;
    servo_vertical_legB     = i;
    servo_vertical_legC     = i;
    wait(0.1);
    servo_horizontal_legA   = i;
    servo_horizontal_legB   = i;
    servo_horizontal_legC   = i;
    wait(0.1);
  }

}

void State_RotateRight(){
  printf("Girar para direita\n");

  for(float i = 0; i < 1; i+=0.05)
  {
    servo_vertical_legA     = i;
    servo_vertical_legB     = i;
    servo_vertical_legC     = i;
    wait(0.1);
    servo_horizontal_legA   = 1-i;
    servo_horizontal_legB   = 1-i;
    servo_horizontal_legC   = 1-i;
    wait(0.1);

  }


  
}

void State_RotateLeft()
{
  printf("Girar para esquerda\n");

  for(float i = 0; i < 1; i+=0.05)
  {
    servo_vertical_legA     = 1-i;
    servo_vertical_legB     = 1-i;
    servo_vertical_legC     = 1-i;
    wait(0.1);
    servo_horizontal_legA   = i;
    servo_horizontal_legB   = i;
    servo_horizontal_legC   = i;
    wait(0.1);

  }

}

void State_Null()
{

}


int main(void) {

  servo.calibrate();

  servo_vertical_legA   = 0;
  servo_vertical_legB   = 0;
  servo_vertical_legC   = 0;
  servo_horizontal_legA = 0;
  servo_horizontal_legB = 0;
  servo_horizontal_legC = 0;


  FunctionPointer = State_Initial;

  while(1)
  {
    (*FunctionPointer)();
  }


  return 0;
}