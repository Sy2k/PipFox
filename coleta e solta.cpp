#include "TextLCD.h"

#include "mbed.h"
//Definição das portas dos motores
BusOut motor_x(PC_4,PB_13,PB_14,PB_1);

BusOut motor_y(PB_2,PB_11,PB_12,PA_11);

BusOut motor_z(PA_12,PC_5,PC_6,PC_8);

//LCD
TextLCD lcd(D8, D9, D4, D5, D6, D7); //definição das portas rs,e,d0,d1,d2,d3

//inicializar usb para visualizar dados em monitor serial
Serial pc(USBTX,USBRX);
//debounce para o botao de emergencia
Timer debounce_emer;

Timer debounce_endstop;

//Botao de emergencia
InterruptIn botao_emergencia(PC_13);

//Endstops - interrupções
InterruptIn endstops(PA_15); //ou pc14
DigitalIn enter(PB_15);

//botoes para controle da movimentação em Z
AnalogIn botoes_nucleo(A0);

//passos dos fusos
int passo_linear_x = 3;//avanço linear do eixo X
int passo_linear_y = 3;//avanço linear do eixo Y
int passo_linear_z = 10;//avanço linear do eixo Z

//passos por revolução dos motores 
int steps_por_revol_x = 512;
int steps_por_revol_y = 512;
int steps_por_revol_z = 512;

//Declaração das portas do Joystick (x e y)
AnalogIn Ax (PC_3);
AnalogIn Ay (PC_2);

//declaração das portas dos leds
DigitalOut led_vermelho(PD_2);
DigitalOut led_verde(PC_11);
DigitalOut led_amarelo(PC_10);
DigitalOut led_azul(PC_12);

//arrays
char solta[3];
char coleta[3];

// Declaracao variavel de posicionamento do joystick
int x, y;
bool movendo_em_x,movendo_em_y,movendo_em_z;

//variáveis que mostram em que direcao a movimentacao esta sendo feita


//variáveis que demonstram se o sistema está referenciado ou não em cada um dos eixos. 
bool ref_x_feito = 0;
bool ref_y_feito = 0;
bool ref_z_feito = 0;

//variável responsável por determinar se o sistema está em estado de emergência ou não. Estado igual a 1 significa que o sistema não está em estado de emergência
bool estado_sis = 1;

//inicialização dos contadores de passos
int step_x = 0;
int step_y = 0;
int step_z = 0;

bool determinar_coleta=1;
int printar=1;

//ESSA PARTE VAI TER QUE REFAZER e ALTERAR OS VALORES
// X Joystick
#define Xmax 65000
#define Xmin 0

// Centro Joystick X
#define CXmin 31000
#define CXmax 33500

// Centro Joystick Y
#define CYmin 31000
#define CYmax 34500

// Y Joystick
#define Ymax 65000
#define Ymin 0

void print_lcd(int step_x, int step_y, int step_z)
{
       lcd.cls();//limpar a tela
    //    float distancia_x = step_x*passo_linear_x/steps_por_revol_x;//calculo da distancia percorrida em x
    //    float distancia_y = step_y*passo_linear_y/steps_por_revol_y;//calculo da distancia percorrida em y
    //    float distancia_z = step_z*passo_linear_z/steps_por_revol_z;//calculo da distancia percorrida em z
    //    lcd.printf("dx=%.0f dy=%.0f\ndz=%.0f", distancia_x, distancia_y, distancia_z);
    //    wait(1);//COLOCAR NA MESMA POSICAO
    //    lcd.cls();
       lcd.printf("Px=%2d Py=%2d\nPz=%2d", step_x, step_y, step_z);
    //    wait(1);
}

void motor_x_sentido_1(int tempo){
    for(int i = 0; i <4;i++){
        motor_x = 1 << i;
        wait_ms(tempo);
    }
}

void motor_x_sentido_2(int tempo){
    for(int i= 3;i>-1;i--){
        motor_x = 1 << i;
        wait_ms(tempo);
    }
}
    
void motor_y_sentido_1(int tempo){
    for(int i = 0; i <4;i++){
        motor_y = 1 << i;
        wait_ms(tempo);
    }
}


void motor_y_sentido_2(int tempo){
    for(int i= 3;i>-1;i--){
        motor_y = 1 << i;
        wait_ms(tempo);
    }
}
    
void motor_z_sentido_1(int tempo){
    for(int z = 0; z <4;z++){
        motor_z = 1 << z;
        wait_ms(tempo);
    }
}

void motor_z_sentido_2(int tempo){
    for(int z = 3; z > -1;z--){
        motor_z = 1 << z;
        wait_ms(tempo);
    }
}

void Mx_off(){
    motor_x=0,0,0,0;//mao desligar o motor - somente cortar a alimentacao 
}
void My_off(){
    motor_y=0,0,0,0;
} 
void Mz_off(){
    motor_z=0,0,0,0;
} 

void ref(){
    if (movendo_em_x==1)
    {
        pc.printf("\r referenciou em x\n");
        // lcd.printf("\r referenciou em x\n");
        movendo_em_x = 0;
        ref_x_feito=1;
        Mx_off();
        debounce_endstop.start();
    }

    else if(movendo_em_y==1 && debounce_endstop.read_ms()>500)
    {
        pc.printf("\r referenciou em y\n");
        // lcd.printf("\r referenciou em y\n");
        movendo_em_y = 0;
        ref_y_feito=1;
        My_off();
        debounce_endstop.reset();
        debounce_endstop.start();
    }

    else if(movendo_em_z==1 && debounce_endstop.read_ms()>500)
    {
        pc.printf("\r referenciou em z\n");
        // lcd.printf("\r referenciou em z\n");
        movendo_em_z = 0;
        ref_z_feito=1;
        Mz_off();
        debounce_endstop.reset();
        step_x=0;
        step_y=0;
        step_z=0;
    }
}


void be(){
    estado_sis = 0;//entrou em estado de emergencia
    step_x = 0;//pensar se precisa checar antes de zerar o valor 
    step_y = 0;
    step_z = 0;
    ref_x_feito=0;//zerar o referenciamento de todos os eixos
    ref_y_feito=0;
    ref_z_feito=0;
    determinar_coleta=0;
    printar=0;
    char solta[3]={0,0,0};
    char coleta[3]={0,0,0};
    
    debounce_emer.start();//iniciar timer para debounce
    pc.printf("\r estado de emergencia\n");
    // lcd.printf("\r estado de emergencia\n");
    //entrada no estado de emergencia e perdendo o referenciamento com botao de emergencia
}
void sair_emer(){
    if(debounce_emer.read_ms()>15){//CHECAR
        estado_sis=1;
        ref_x_feito=0;//zerar o referenciamento de todos os eixos
        ref_y_feito=0;
        ref_z_feito=0;
        debounce_emer.reset();//resetar timer de reset
        pc.printf("\r saindo do estado de emergência\n");
        // lcd.printf("\r saindo do estado de emergência\n");
    }
}
// void endstop_crash(){
//     estado_sis=0;
//     ref_x_feito=0;
//     ref_y_feito=0;
//     ref_z_feito=0;
//     pc.printf("\r estado de emergencia devido a batida \n");
// }
int main(){
    Mz_off();
    Mx_off();
    My_off();  
    movendo_em_x = 0;
    movendo_em_y = 0;
    movendo_em_z = 0;
    //valores de tempo entre cada passo determinado como 1 ms
    int vx = 3;
    int vx_inv = vx;
    int vy = 3;
    int vy_inv = vy;
    int vz = 3;
    int vz_inv = vz;
    botao_emergencia.mode(PullUp);//definição do botão de emergencia como PullUp -> ISSO DAQUI TA ESTRANHO
    pc.baud(9600);//definição do baud rate da comunicação serial usb
    botao_emergencia.fall(&be);
    botao_emergencia.rise(&sair_emer);
    endstops.fall(&ref);
    while(1){
            x = Ax.read_u16();//ou Ax.read*1000()
            y = Ay.read_u16();//ou Ay.read*1000()   
            if(estado_sis==1){//não está em estado emergencia
                
                //HOMING - referenciamento dos eixos
                if(ref_x_feito==0){
                    movendo_em_x=1;
                    while(ref_x_feito==0){
                        pc.printf("\rdentro_refx\n");
                        // lcd.printf("\rdentro_refx\n");
                        motor_x_sentido_1(vx);
                        if(estado_sis==0){
                            break;
                        }
                    }
                }
                else if(ref_y_feito == 0){
                    movendo_em_y=1;
                    while(ref_y_feito == 0){
                        pc.printf("\rdentro_refy\n");
                        // lcd.printf("\rdentro_refy\n");
                        motor_y_sentido_1(vy);
                        if(estado_sis==0){
                            break;
                        }
                    }
                }

                else if(ref_z_feito ==0){
                    movendo_em_z=1;
                    while(ref_z_feito == 0){
                        pc.printf("\rdentro_refz\n");
                        // lcd.printf("\rdentro_refz\n");
                        motor_z_sentido_1(vz);
                        if(estado_sis==0){
                            break;
                        }
                    }
                }
                
                else{
                    if(printar==1){
                    pc.printf("\rreferenciado e esperando enter para determinar ponto de coleta\n");
                    }
                    //Condições para a movimentação dependendo da posição do joystick
                    if(x > CXmax && estado_sis == 1)
                    {
                    // int vx = map(x, CXmax, Xmax, 5, 0.5);
                        motor_x_sentido_1(vx); //dando certo
                        pc.printf("\rmotor_x_sentido1\n");
                        step_x+=4;
                    }
                    if(x < CXmin && estado_sis == 1)
                    {
                        //int vx_inv = map(x, CXmin, Xmin, 0.5, 5);
                        motor_x_sentido_2(vx_inv); //dando errado
                        step_x-=4; 
                    }
                    if(y > CYmax && estado_sis == 1)
                    {
                        //int vy = map(y, CYmax, Ymax, 5, 0.5);
                        motor_y_sentido_1(vy);
                        step_y+=4;
                    }
                    if(y < CYmin && estado_sis == 1)
                    {
                    // int vy_inv = map(y, CYmin ,Ymin, 0.5, 5);
                        motor_y_sentido_2(vy_inv);     
                        step_y-=4;
                    }
                
                    if(botoes_nucleo.read()>0.095 && botoes_nucleo.read()<0.110){
                        motor_z_sentido_1(vz);
                        step_z+=4;
                    }

                    if(botoes_nucleo.read()>0.25 && botoes_nucleo.read()<0.26){
                        motor_z_sentido_2(vz_inv);
                        step_z-=4;
                    }

                    if(enter==0 && printar==1){
                        pc.printf("\rclicaram em enter viu\n");
                        if(determinar_coleta==1){
                            coleta[0]=step_x;
                            coleta[1]=step_y;
                            coleta[2]=step_z;
                            determinar_coleta==0;
                        }
                        if(determinar_coleta==1){
                            solta[0]=step_x;
                            solta[1]=step_y;
                            solta[2]=step_z;
                            printar=0;    
                            pc.printf("\ragora vamos printar\n");
 
                        }
                    }
                    if(printar==0){
                        for(int i = 0; i < 3; ++i) {
                                printf("%d\n", coleta[i]);
                            }                            
                            for(int i = 0; i < 3; ++i) {
                                printf("%d\n", solta[i]);
                            }    
                    }
                    
                    print_lcd(step_x, step_y, step_z); //função de print dos pulsos e deslocamentos
                }
        }
        else{
            // be();
        }
    }
}

                            // int coleta[] = {[step_x, step_y, step_z,volume],[],[]};
