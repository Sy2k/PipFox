#include "mbed.h"
//Motores
BusOut motor_x(PC_4,PB_13,PB_14,PB_1);

BusOut motor_y(PB_2,PB_11,PB_12,PA_11);

BusOut motor_z(PA_12,PC_5,PC_6,PC_8);
//Botao de emergencia
DigitalIn botao_emergencia(PC_13);
//Endstops
DigitalIn endstop_x(PC_15);
DigitalIn endstop_y(PA_15);
DigitalIn endstop_z(PB_15);
//DEFINIR PORTAS PRA ESSES TB

//botoes para controle da movimentação em Z
AnalogIn botoes_nucleo(A0);

//passos dos fusos
passo_linear_x=3;//avanço linear do eixo X
passo_linear_y=3;//avanço linear do eixo Y
passo_linear_z=10;//avanço linear do eixo Z

//passos por revolução dos motores 
steps_por_revol_x=512;
steps_por_revol_y=512;
steps_por_revol_z=512;

//Declaração das portas do Joystick (x e y)
AnalogIn Ax (PC_3);
AnalogIn Ay (PC_2);
//declaração das portas dos leds
DigitalOut led_vermelho(PD_2);
DigitalOut led_verde(PD_2);
DigitalOut led_amarelo(PD_2);
DigitalOut led_azul(PC_12);

// Declaracao variavel de posicionamento do joystick
int x, y;

//variáveis que demonstram se o sistema está referenciado ou não em cada um dos eixos. 
int ref_x_feito=0;
int ref_y_feito=0;
int ref_z_feito=0;

//variável responsável por determinar se o sistema está em estado de emergência ou não. Estado igual a 1 significa que o sistema não está em estado de emergência
int estado=1;

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
        for(int z = 0; z <4;z++){
            motor_z = 1 << z;
            wait(tempo);
        }
        }

void motor_z_sentido_2(float tempo){
        for(int z= 3;z>-1;z--){
            motor_z = 1 << z;
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

void emer(){
    estado=0;
    ref_x_feito=0;
    ref_y_feito=0;
    ref_z_feito=0;

    //entrada no estado de emergencia e perdendo o referenciamento
    }
int main(){
    Mz_off();
    Mx_off();
    My_off();
    //valores de tempo entre cada passo
    int vx=0.01;
    int vx_inv=vx;
    int vy=0.01;
    int vy_inv=vy;
    int vz=0.01;
    int vz_inv=vz;
 while(1){
        botao_emergencia.rise(&emer);
        
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