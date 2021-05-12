#include "mbed.h"
//Botões
BusOut motor_x(D2,D3,D4,D5);

BusOut motor_y(D7,D8,D9,D10);

BusOut motor_z(D11,D12,D13,D14);
//Botao de emergencia
DigitalIn botao_emergencia(D6);
//Endstops
DigitalIn botao_x1(PA_11);
DigitalIn botao_y1(PB_12);
DigitalIn botao_z1(PB_11);
//DEFINIR PORTAS PRA ESSES TB

// DigitalIn botao_x2(**);
// DigitalIn botao_y2(**);
// DigitalIn botao_z2(**);

//botoes para controle da movimentação em Z
//DigitalIn mov_z1(**);
//DigitalIn mov_z2(**);

//Joystick
AnalogIn Ax (A1);
AnalogIn Ay (A0);

// Declaracao variavel
int x, y;

int ref_x_feito=0;
int ref_y_feito=0;

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

void motor_x_sentido_1(float tempo){
        for(int i = 0; i <4;i++){
            motor_x = 1 << i;
            wait(tempo);
        }
    }

void motor_x_sentido_2(float tempo){
        for(int i= 3;i>-1;i--){
            motor_x = 1 << i;
            wait(tempo);
        }
    }
    
void motor_y_sentido_1(float tempo){
        for(int i = 0; i <4;i++){
            motor_y = 1 << i;
            wait(tempo);
        }
        }


void motor_y_sentido_2(float tempo){
        for(int i= 3;i>-1;i--){
            motor_y = 1 << i;
            wait(tempo);
        }
    }
    
void motor_z_sentido_1(float tempo){
        for(int i = 0; i <4;i++){
            motor_z = 1 << i;
            wait(tempo);
        }
        }

void motor_z_sentido_2(float tempo){
        for(int i= 3;i>-1;i--){
            motor_z = 1 << i;
            wait(tempo);
        }
    }

void Mx_off(){
    motor_x=(0,0,0,0)
    }
void My_off(){
    motor_y=(0,0,0,0)
    } 
void Mz_off(){
    motor_z=(0,0,0,0)
    } 

int main(){
    Mz_off();
    Mx_off();
    My_off();
    //se for usar map retirar as variaveis abaixo
    int vx=0.01;
    int vx_inv=vx;
    int vy=0.01;
    int vy_inv=vy;
    int vz=0.01;
    int vz_inv=vz;
 while(1){
        x = Ax.read_u16();//ou Ax.read*1000()
        y = Ay.read_u16();//ou Ay.read*1000()   
        
        if(x > CXmax){
           // int vx = map(x, CXmax, Xmax, 5, 0.5);
            motor_x_sentido_1(vx); //dando certo
        }
        else if(x < CXmin){
            //int vx_inv = map(x, CXmin, Xmin, 0.5, 5);
            motor_x_sentido_2(vx_inv); //dando errado     
        }
        if(y > CYmax){
            //int vy = map(y, CYmax, Ymax, 5, 0.5);
            motor_y_sentido_1(vy);
        }
        else if(y < CYmin){
           // int vy_inv = map(y, CYmin ,Ymin, 0.5, 5);
            motor_y_sentido_2(vy_inv);        
        }
        if(mov_z1.read() == 0){
            motor_z_sentido_1(vz);
        }
        if(mov_z2.read() == 0){
            motor_z_sentido_2(vz);
        }
        else if(mov_z1.read()=1 ||mov_z2.read()=1){
            Mz_off();
        }
     

    //endstops
        // if(botao_y.read()==0){
            
        // }
        // if(botao_x.read()==0){
        //     motor_x_sentido_1();
        // }

        // if(botao_z.read()==0){
        //     motor_z_sentido_1();
        // }      
    }
    }

   
    


    wait(0.01);
