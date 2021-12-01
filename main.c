#include <stdio.h>
#include <stdlib.h>

void (*FunctionPointer)();

void State_Initial();
void State_Forward();
void State_Backward();
void State_RotateRight();
void State_RotateLeft();

void State_Initial()
{
  printf("Inicio\n");
  FunctionPointer = State_Forward;
}

void State_Forward()
{
  printf("Andar para frente\n");
  FunctionPointer = State_Backward;
}

void State_Backward()
{
  printf("Andar para tras\n");
  FunctionPointer = State_RotateRight;
}

void State_RotateRight(){
  printf("Girar para direita\n");
  FunctionPointer = State_RotateLeft;
}

void State_RotateLeft()
{
  printf("Girar para esquerda\n");
  FunctionPointer = State_Initial;
}


int main(void) {
  FunctionPointer = State_Initial;

  while(1)
  {
    (*FunctionPointer)();
  }


  return 0;
}