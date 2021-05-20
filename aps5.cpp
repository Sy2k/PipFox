#include "TextLCD.h"

#include "mbed.h"
//Motores
BusOut motor_x(PC_4,PB_13,PB_14,PB_1);

BusOut motor_y(PB_2,PB_11,PB_12,PA_11);

BusOut motor_z(PA_12,PC_5,PC_6,PC_8);
//LCD
TextLCD lcd(D8, D9, D4, D5, D6, D7); //rs,e,d0,d1,d2,d3

//inicializar usb
Serial pc(USBTX,USBRX);


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

//inicialização dos contadores de passos
int step_x = 0;
int step_y = 0;
int step_z = 0;

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

void print_lcd(int step_x, int step_y, int step_z){
       lcd.cls();
       int distancia_x = step_x*passo_linear_x/steps_por_revol_x;
       int distancia_y = step_y*passo_linear_y/steps_por_revol_y;
       int distancia_z = step_z*passo_linear_z/steps_por_revol_z;
       lcd.locate(5,1);
       lcd.printf("\rdistancia_x=%2d, distancia_y=%2d\ndistancia_z=%2d ", distancia_x, distancia_y, distancia_z);
}

void motor_x_sentido_1(int tempo){
        for(int i = 0; i <4;i++){
            motor_x = 1 << i;
            wait_ms(tempo);
            if(estado_sis==0){
                break;
            }
        }
    }

void motor_x_sentido_2(int tempo){
        for(int i= 3;i>-1;i--){
            motor_x = 1 << i;
            wait_ms(tempo);
            if(estado_sis==0){
                break;
            }
        }
    }
    
void motor_y_sentido_1(int tempo){
        for(int i = 0; i <4;i++){
            motor_y = 1 << i;
            wait_ms(tempo);
            if(estado_sis==0){
                break;
            }
        }
        }


void motor_y_sentido_2(int tempo){
        for(int i= 3;i>-1;i--){
            motor_y = 1 << i;
            wait_ms(tempo);
            if(estado_sis==0){
                break;
            }
        }
    }
    
void motor_z_sentido_1(int tempo){
        for(int z = 0; z <4;z++){
            motor_z = 1 << z;
            wait_ms(tempo);
            if(estado_sis==0){
                break;
            }
        }
        }

void motor_z_sentido_2(int tempo){
        for(int z= 3;z>-1;z--){
            motor_z = 1 << z;
            wait_ms(tempo);
            if(estado_sis==0){
                break;
            }
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
    step_x = 0;
    step_y = 0;
    step_z = 0;
    pc.printf("\restado de emergencia\n");
    //entrada no estado de emergencia e perdendo o referenciamento com botao de emergencia
    }
void sair_emer(){
    estado_sis=1;
    ref_x_feito=0;
    ref_y_feito=0;
    ref_z_feito=0;
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
    int vx=1;
    int vx_inv=vx;
    int vy=1;
    int vy_inv=vy;
    int vz=1;
    int vz_inv=vz;
    botao_emergencia.mode(PullUp);
    pc.baud(9600);
    botao_emergencia.rise(&be);
    botao_emergencia.fall(&sair_emer);
    endstop_z.fall(&Mz_ref);//isso daqui vai funcionar um por vez?
    endstop_y.fall(&My_ref);
    endstop_x.fall(&Mx_ref);
 while(1){
        x = Ax.read_u16();//ou Ax.read*1000()
        y = Ay.read_u16();//ou Ay.read*1000()   
        if(estado_sis==1){//não está em estado emergencia
            //HOMING
        if(ref_x_feito==0 && ref_y_feito==0 && ref_z_feito==0){
            while(ref_x_feito==0 && ref_y_feito==0 && ref_z_feito==0){
                pc.printf("\rdentro_refxyz\n");
                motor_x_sentido_1(vx);
                motor_y_sentido_1(vy);
                motor_z_sentido_1(vz);
        }
        }
        
        if(ref_x_feito==0 && ref_y_feito==0){
            while(ref_x_feito==0 && ref_y_feito==0){
                pc.printf("\rdentro_refxy\n");
                motor_y_sentido_1(vy);
                motor_x_sentido_1(vx);
            }
        }

        if(ref_x_feito==0 && ref_z_feito==0){
            while(ref_x_feito==0 && ref_z_feito==0){
                pc.printf("\rdentro_refxz\n");
                motor_x_sentido_1(vx);
                motor_z_sentido_1(vz);
            }
        }
        else if(ref_y_feito==0 && ref_z_feito==0){
            while(ref_y_feito==0 && ref_z_feito==0){
                pc.printf("\rdentro_refyz\n");
                motor_y_sentido_1(vy);
                motor_z_sentido_1(vz);
            }
        }
        if(ref_y_feito==0){
            while(ref_y_feito==0){
                pc.printf("\rdentro_refy\n");
                motor_y_sentido_1(vy);
            }
        }
        if(ref_x_feito==0){
            while(ref_x_feito==0){
                pc.printf("\rdentro_refx\n");
                motor_x_sentido_1(vx);
            }
        }
        if(ref_z_feito==0){
            while(ref_z_feito==0){
                pc.printf("\rdentro_refz\n");
                motor_z_sentido_1(vz);
            }
        }
        
        
        else{
            pc.printf("\rreferenciado\n");
            endstop_x.fall(NULL);
            endstop_y.fall(NULL);
            endstop_z.fall(NULL);
            //wait(0.01);
            endstop_x.fall(&endstop_crash);//interrupção devido à colisão de um endstop
            endstop_y.fall(&endstop_crash);//interrupção devido à colisão de um endstop
            endstop_z.fall(&endstop_crash);//interrupção devido à colisão de um endstop
        
            // int step_x = 0;
            // int step_y = 0;
            // int step_z = 0;
            
            //Condições para a movimentação
            if(x > CXmax)
            {
            // int vx = map(x, CXmax, Xmax, 5, 0.5);
                motor_x_sentido_1(vx); //dando certo
                pc.printf("\rmotor_x_sentido1\n");

                step_x++;
            }
            if(x < CXmin)
            {
                //int vx_inv = map(x, CXmin, Xmin, 0.5, 5);
                motor_x_sentido_2(vx_inv); //dando errado
                step_x--;
            }
            if(y > CYmax)
            {
                //int vy = map(y, CYmax, Ymax, 5, 0.5);
                motor_y_sentido_1(vy);
                step_y++;
            }
            if(y < CYmin)
            {
            // int vy_inv = map(y, CYmin ,Ymin, 0.5, 5);
                motor_y_sentido_2(vy_inv);     
                step_y--;
            }
        
            if(botoes_nucleo.read()>0.1000 && botoes_nucleo.read()<0.1020){
                motor_z_sentido_1(vz);
                step_z++;
            }

            if(botoes_nucleo.read()>0.25 && botoes_nucleo.read()<0.26){
                motor_z_sentido_2(vz_inv);
                step_z--;
            }

            // if(botoes_nucleo>0.800 && botoes_nucleo<0.815){
            //    // enter
            // }
            
            //print_lcd(step_x, step_y, step_z);//função de print dos pulsos e deslocamentos
        }
    }
            else{
                be();
            }
}
}
