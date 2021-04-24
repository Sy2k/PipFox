#include "mbed.h"
#include "Map.hpp"
//Motor X - Setup
DigitalOut mx_f1 (D2);
DigitalOut mx_f2 (D3);
DigitalOut mx_f3 (D4);
DigitalOut mx_f4 (D5);
 
//Motor Y - Setup 
DigitalOut my_f1 (D6);
DigitalOut my_f2 (D7);
DigitalOut my_f3 (D8);
DigitalOut my_f4 (D9);

// Joystick - Setup
DigitalIn botao (D10);
AnalogIn  Ax (A0);
AnalogIn Ay (A1);

// Declaracao variavel
int delay = 1;
int x, y;

// X Joystick
#define Xmax 1000
#define Xmin 0

// Centro Joystick X
#define CXmin 450
#define CXmax 550

// Centro Joystick Y
#define CYmin 450
#define CYmax 550

// Y Joystick
#define Ymax 1000
#define Ymin 0

//MAP
long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
// Acionamento das fases 
void fase(DigitalOut fa, DigitalOut fb, int delay){
    fa = 1;
    fb = 0;
    wait_ms(delay);
    fb = 1;
    wait_ms(delay);
    fa = 0;
}
void micro_passo(DigitalOut fa, DigitalOut fb, int delay){
    fa = 1;
    fb = 1;
    wait_ms(delay);
    fb = 0;
    fa = 0;
    wait_ms(delay);
}

//micro_passo(mx_f1,mx_f2,dlay);
//micro_passo(mx_f2,mx_f3,dlay);
//micro_passo(mx_f3,mx_f4,dlay);
//micro_passo(mx_f4,mx_f1,dlay);

// MOTOR X - Horario
void Mx_H(int dlay){
    micro_passo(mx_f1, mx_f2, dlay);
    micro_passo(mx_f2, mx_f3, dlay);
    micro_passo(mx_f3, mx_f4, dlay);
    micro_passo(mx_f4, mx_f1, dlay);
}

// MOTOR X - AntiHorario
void Mx_Ah(int dlay){
    micro_passo(mx_f4, mx_f3, dlay);
    micro_passo(mx_f3, mx_f2, dlay);
    micro_passo(mx_f2, mx_f1, dlay);
    micro_passo(mx_f1, mx_f4, dlay);
}

// MOTOR Y - Horario
void My_H(int dlay){
    micro_passo(my_f1, my_f2, dlay);
    micro_passo(my_f2, my_f3, dlay);
    micro_passo(my_f3, my_f4, dlay);
    micro_passo(my_f4, my_f1, dlay);
}

// MOTOR Y - AntiHorario
void My_Ah(int dlay){
    micro_passo(my_f4, my_f3, dlay);
    micro_passo(my_f3, my_f2, dlay);
    micro_passo(my_f2, my_f1, dlay);
    micro_passo(my_f1, my_f4, dlay);
}

int main (){
    while(1){
        x = Ax.read()*1000;
        y = Ay.read()*1000;   
        
        if(x > CXmax){
            int vx = map(x, CXmax, Xmax, 5, 0.5);
            Mx_H(vx); //dando certo
        }
        else if(x < CXmin){
            int vx_inv = map(x, Xmin, CXmin, 5, 0.5);
            Mx_Ah(vx_inv); //dando errado     
        }
        if(y > CYmax){
            int vy = map(y, CYmax, Ymax, 5, 0.5);
            My_H(vy);
        }
        else if(y < CYmin){
            int vy_inv = map(y, Ymin ,CYmin, 5, 0.5);
            My_Ah(vy_inv);        
        }
    }
}