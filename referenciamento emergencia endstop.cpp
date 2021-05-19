#include "mbed.h"
//Motores
BusOut motor_x(PC_4,PB_13,PB_14,PB_1);

BusOut motor_y(PB_2,PB_11,PB_12,PA_11);

BusOut motor_z(PA_12,PC_5,PC_6,PC_8);
//Botao de emergencia
InterruptIn botao_emergencia(PC_13);
//Endstops
InterruptIn endstop_x(PC_15);
InterruptIn endstop_y(PB_15);
InterruptIn endstop_z(PA_15);
//DEFINIR PORTAS PRA ESSES TB

//botoes para controle da movimentação em Z
AnalogIn botoes_nucleo(A0);

//passos dos fusos
int passo_linear_x=3;//avanço linear do eixo X
int passo_linear_y=3;//avanço linear do eixo Y
int passo_linear_z=10;//avanço linear do eixo Z

//passos por revolução dos motores 
int steps_por_revol_x=512;
int steps_por_revol_y=512;
int steps_por_revol_z=512;

//Declaração das portas do Joystick (x e y)
AnalogIn Ax (PC_3);
AnalogIn Ay (PC_2);
//declaração das portas dos leds
DigitalOut led_vermelho(PD_2);
DigitalOut led_verde(PC_11);
DigitalOut led_amarelo(PC_10);
DigitalOut led_azul(PC_12);

// Declaracao variavel de posicionamento do joystick
int x, y;

//variáveis que demonstram se o sistema está referenciado ou não em cada um dos eixos. 
int ref_x_feito=0;
int ref_y_feito=0;
int ref_z_feito=0;

//variável responsável por determinar se o sistema está em estado de emergência ou não. Estado igual a 1 significa que o sistema não está em estado de emergência
int estado_sis=1;

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
    motor_x=0,0,0,0;
    }
void My_off(){
    motor_y=0,0,0,0;
    } 
void Mz_off(){
    motor_z=0,0,0,0;
    } 

void Mx_ref(){
    motor_x=0,0,0,0;//desliga o motor 
    ref_x_feito=1;//define que o referenciamento foi concluido
    }
void My_ref(){
    motor_y=0,0,0,0;
    ref_y_feito=1;
    } 
void Mz_ref(){
    motor_z=0,0,0,0;
    ref_z_feito=1;
    } 

void be(){
    estado_sis=0;
    ref_x_feito=0;
    ref_y_feito=0;
    ref_z_feito=0;

    //entrada no estado de emergencia e perdendo o referenciamento com botao de emergencia
    }
void sair_emer(){
    estado_sis=1;
}
void endstop_crash(){
    estado_sis=0;
    ref_x_feito=0;
    ref_y_feito=0;
    ref_z_feito=0;
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
    botao_emergencia.mode(PullUp);
 while(1){
        botao_emergencia.fall(&be);
        x = Ax.read_u16();//ou Ax.read*1000()
        y = Ay.read_u16();//ou Ay.read*1000()   
        if(estado_sis==1){//não está em estado emergencia
            //HOMING
        if(ref_x_feito==0 && ref_y_feito==0 && ref_z_feito==0){
        motor_x_sentido_1(vx);
        motor_y_sentido_1(vy);
        motor_z_sentido_1(vz);
        endstop_z.fall(&Mz_ref);//isso daqui vai funcionar um por vez?
        endstop_y.fall(&My_ref);
        endstop_x.fall(&Mx_ref);
        }
        
        else if(ref_x_feito==0 && ref_y_feito==0){
            motor_y_sentido_1(vy);
            motor_x_sentido_1(vx);
            endstop_y.fall(&My_ref);
            endstop_x.fall(&Mx_ref);
        }

        else if(ref_x_feito==0 && ref_z_feito==0){
            motor_x_sentido_1(vx);
            motor_z_sentido_1(vz);
            endstop_x.fall(&Mx_ref);
            endstop_z.fall(&Mz_ref);
        }
        else if(ref_y_feito==0 && ref_z_feito==0){
            motor_y_sentido_1(vy);
            motor_z_sentido_1(vz);
            endstop_y.fall(&My_ref);
            endstop_z.fall(&Mz_ref);
        }
        else if(ref_y_feito==0){
            motor_y_sentido_1(vy);
            endstop_y.fall(&My_ref);
        }
        else if(ref_x_feito==0){
            motor_x_sentido_1(vx);
            endstop_x.fall(&Mx_ref);
        }
        else if(ref_z_feito==0){
            motor_z_sentido_1(1);
            endstop_z.fall(&Mz_ref);
        }
        
        else{
            endstop_x.fall(&endstop_crash);//interrupção devido à colisão de um endstop
            endstop_y.fall(&endstop_crash);//interrupção devido à colisão de um endstop
            endstop_z.fall(&endstop_crash);//interrupção devido à colisão de um endstop

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
        // if(mov_z1.read() == 0){
        //     motor_z_sentido_1(vz);
        // }
        // if(mov_z2.read() == 0){
        //     motor_z_sentido_2(vz);
        // }
        // else if(mov_z1.read()=1 ||mov_z2.read()=1){
        //     Mz_off();
        // }
     
        if(botoes_nucleo>0.1000 && botoes_nucleo<0.1020){
            motor_z_sentido_1(vz);
        }

        if(botoes_nucleo>0.25 && botoes_nucleo<0.26){
            motor_z_sentido_2(vz_inv);
        }

        // if(botoes_nucleo>0.800 && botoes_nucleo<0.815){
        //    // enter
        // }
        }
        }
        else{
            be();
            botao_emergencia.rise(&sair_emer);
        }
 }

    }
