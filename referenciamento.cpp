#include "mbed.h"
//programa que seria importado dentro de outro programa e rodaria como uma rotina

Timer debouncez;
Timer debouncex;
Timer debouncey;

//Motor X - Setup
DigitalOut mx_f1 (D4);
DigitalOut mx_f2 (D5);
DigitalOut mx_f3 (D6);
DigitalOut mx_f4 (D7);

//Motor Y - Setup 
DigitalOut my_f1 (D10);
DigitalOut my_f2 (D11);
DigitalOut my_f3 (D12);
DigitalOut my_f4 (D14);

//Motor Z - Setup - > FALTA COLOCAR OS PINOSSSS certos
DigitalOut mz_f1 ();
DigitalOut mz_f2 ();
DigitalOut mz_f3 ();
DigitalOut mz_f4 ();

//Endstops
InterruptIn endstop_x (D1);
InterruptIn endstop_y (D2);
InterruptIn endstop_z (D3);

// Joystick - Setup
DigitalIn botao_js (PC_13); //talvez não seja utilizado
AnalogIn  Ax (A1);
AnalogIn Ay (A0);

// Declaracao variavel
int delay = 1;
int x, y; //verificar se ela é util

//estado do referenciamento -> talvez nao seja necessário
int ref_x_feito=0;
int ref_y_feito=0;
int ref_z_feito=0;

//ESSA PARTE VAI TER QUE REFAZER e ALTERAR OS VALORES
// X Joystick
#define Xmax 65000
#define Xmin 0

// Centro Joystick X
#define CXmin 31000
#define CXmax 32500

// Centro Joystick Y
#define CYmin 31000
#define CYmax 34500

// Y Joystick
#define Ymax 65000
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

// MOTOR X - Horario
void Mx_H(float dlay){
    dlay=dlay/10;
    fase(mx_f1, mx_f2, dlay);
    fase(mx_f2, mx_f3, dlay);
    fase(mx_f3, mx_f4, dlay);
    fase(mx_f4, mx_f1, dlay);
}

// MOTOR X - AntiHorario
void Mx_Ah(float dlay){
    dlay=dlay/10;
    fase(mx_f4, mx_f3, dlay);
    fase(mx_f3, mx_f2, dlay);
    fase(mx_f2, mx_f1, dlay);
    fase(mx_f1, mx_f4, dlay);
}

// MOTOR Y - Horario
void My_H(float dlay){
    dlay=dlay/10;
    fase(my_f1, my_f2, dlay);
    fase(my_f2, my_f3, dlay);
    fase(my_f3, my_f4, dlay);
    fase(my_f4, my_f1, dlay);
}

// MOTOR Y - AntiHorario
void My_Ah(float dlay){
    dlay=dlay/10;
    fase(my_f4, my_f3, dlay);
    fase(my_f3, my_f2, dlay);
    fase(my_f2, my_f1, dlay);
    fase(my_f1, my_f4, dlay);
}

// MOTOR Z - Horario
void Mz_H(float dlay){
    dlay=dlay/10;
    fase(mz_f1, mz_f2, dlay);
    fase(mz_f2, mz_f3, dlay);
    fase(mz_f3, mz_f4, dlay);
    fase(mz_f4, mz_f1, dlay);
}

// MOTOR Z - AntiHorario
void Mz_Ah(float dlay){
    dlay=dlay/10;
    fase(mz_f4, mz_f3, dlay);
    fase(mz_f3, mz_f2, dlay);
    fase(mz_f2, mz_f1, dlay);
    fase(mz_f1, mz_f4, dlay);
}

void Mx_off(){
    if(debouncex.read_ms()>10){
    mx_f1 =0;
    mx_f2 =0;
    mx_f3 =0;
    mx_f4 =0;
    ref_x_feito=1;
    }
    debouncex.reset();
    }

void My_off(){
    if(debouncey.read_ms()>10){
    my_f1 =0;
    my_f2 =0;
    my_f3 =0;
    my_f4 =0;
    ref_y_feito=1;
    }
    debouncey.reset();
    } 

void Mz_off(){
    if(debouncez.read_ms()>10){
    mz_f1 =0;
    mz_f2 =0;
    mz_f3 =0;
    mz_f4 =0;
    ref_z_feito=1;
    }
    debouncez.reset();
    } 
    
int main (){
    endstop_x.mode(PullUp);
    endstop_y.mode(PullUp);
    endstop_z.mode(PullUp);
    debouncex.start();
    debouncey.start();
    debouncez.start();
    while(1){
        // x = Ax.read_u16();//ou Ax.read*1000()
        // y = Ay.read_u16();//ou Ay.read*1000()    
    
    if(ref_x_feito=0 && ref_y_feito=0 && ref_z_feito=0){
    My_H(1);
    Mx_H(1);
    Mz_H(1);
    endstop_z.fall(Mz_off);
    endstop_y.fall(My_off);
    endstop_x.fall(Mx_off);
    }
    
    elif(ref_x_feito=0 && ref_y_feito=0){
        My_H(1);
        Mx_H(1);
        endstop_y.fall(My_off);
        endstop_x.fall(Mx_off);
    }

    elif(ref_x_feito=0 && ref_z_feito=0){
        Mx_H(1);
        Mz_H(1);
        endstop_x.fall(Mx_off);
        endstop_z.fall(Mz_off);
    }
    elif(ref_y_feito=0 && ref_z_feito=0){
        My_H(1);
        Mz_H(1);
        endstop_y.fall(My_off);
        endstop_z.fall(Mz_off);
    }
    elif(ref_y_feito=0){
        My_H(1);
        endstop_y.fall(My_off);
    }
    elif(ref_x_feito=0){
        Mx_H(1);
        endstop_y.fall(Mx_off);
    }
    elif(ref_z_feito=0){
        Mz_H(1);
        endstop_y.fall(Mz_off);
    }
    
    // if(ref_x_feito=1){
    //     My_H(1);
    //     Mx_off();
    // }
    // if(ref_y_feito=1){
    //     Mx_H(1);
    //     My_off();
    // }
    // if(ref_x_feito=1 && ref_y_feito=0){
    //     My_off();
    //     Mx_off();
    // }

    wait(0.01);
    }
    }
