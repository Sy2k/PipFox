//#include "TextLCD.h"


//CONFIGURACAO DO DISPLAY   
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft;
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

uint8_t Orientation = 1;  

#include "Arduino.h"

#include "mbed.h"
//Definição das portas dos motores
BusOut motor_x(PC_4,PB_13,PB_14,PB_1);

BusOut motor_y(PB_2,PB_11,PB_12,PA_11);

BusOut motor_z(PA_12,PC_5,PC_6,PC_8);

//LCD - NAO USADO MAIS
//TextLCD lcd(D8, D9, D4, D5, D6, D7); //definição das portas rs,e,d0,d1,d2,d3

//inicializar usb para visualizar dados em monitor serial
Serial pc(USBTX,USBRX);
//debounce para o botao de emergencia
Timer debounce_emer;

Timer debounce_endstop;

Timer tempo_de_funcionamento;

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
char atual[3];
char distancia_coleta_atual[3];
char distancia_solta_coleta[3];

// Declaracao variavel de posicionamento do joystick
int x, y, vx, vx_inv, vy, vy_inv, vz, vz_inv;
bool movendo_em_x,movendo_em_y,movendo_em_z;

    // int vx = 3;
    // int vx_inv = vx;
    // int vy = 3;
    // int vy_inv = vy;
    // int vz = 3;
    // int vz_inv = vz;

//variáveis que mostram em que direcao a movimentacao esta sendo feita


//variáveis que demonstram se o sistema está referenciado ou não em cada um dos eixos. 
bool ref_x_feito = 0;
bool ref_y_feito = 0;
bool ref_z_feito = 0;

//variável responsável por determinar se o sistema está em estado de emergência ou não. Estado igual a 1 significa que o sistema não está em estado de emergência
int estado_sis = 1;//0 -> emergencia; -> 1 funcionamento normal; 2-> fim do processo

//inicialização dos contadores de passos
int step_x = 0;
int step_y = 0;
int step_z = 0;
//variaveis variaveis responsaveis pela logica
bool determinar_ponto=1;//1 -> ponto de coleta; 0 -> ponto de solta
bool print_valor_pos=1;
bool tipo_de_movimento=0;//0 atual para coleta ou 1 de pega para solta
bool rotina_principal=0;//1 para rotina principal e 0 para outras rotinas

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
        tft.setTextColor(GREEN);
        tft.setTextSize(3);
        tft.setCursor(0,10); //  Orientação do texto X,Y
        int distancia_x = step_x*passo_linear_x/steps_por_revol_x;//calculo da distancia percorrida em x
        int distancia_y = step_y*passo_linear_y/steps_por_revol_y;//calculo da distancia percorrida em y
        int distancia_z = step_z*passo_linear_z/steps_por_revol_z;//calculo da distancia percorrida em z
        tft.printf("distancia_x=%.2f\ndistancia_y=%.2f\ndistancia_z=%.f\npassos_x=%.0f\npassos_y=%.0f", distancia_x, distancia_y, distancia_z,step_x,step_y);
        tft.printf("\npassos_z=%.0f",step_z);
}

void motor_x_sentido_1(int tempo){
    for(int i = 0; i <4;i++){
        motor_x = 1 << i;       //verificar se o estado do sistema eh 0 ou 1
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
    determinar_ponto=0;
    print_valor_pos=0;
    for(int i = 0; i < 3; ++i) {
            solta[i]=0;
            coleta[i]=0;
        } 
    
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
void setup(void){
    tft.reset();
    tft.begin();
    tft.setRotation(Orientation);
    tft.fillScreen(BLACK);  // Fundo do Display
    print_lcd(step_x,step_y,step_z);     //PRA MIM ISSO NAO FAZ SENTIDO ESTAR AQUI DENTRO
    delay(1000);//tava 1000 antes, ACHEI DEMAIS

    Mz_off();
    Mx_off();
    My_off();  
    movendo_em_x = 0;
    movendo_em_y = 0;
    movendo_em_z = 0;
    //valores de tempo entre cada passo determinado como 1 ms
    vx = 3;
    vx_inv = vx;
    vy = 3;
    vy_inv = vy;
    vz = 3;
    vz_inv = vz;
    botao_emergencia.mode(PullUp);//definição do botão de emergencia como PullUp -> ISSO DAQUI TA ESTRANHO
    pc.baud(9600);//definição do baud rate da comunicação serial usb
    botao_emergencia.fall(&be);
    botao_emergencia.rise(&sair_emer);
    endstops.fall(&ref);
}
    void loop(void){
            x = Ax.read_u16();//ou Ax.read*1000()
            y = Ay.read_u16();//ou Ay.read*1000()   
            if(estado_sis!=0){//não está em estado emergencia
                
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
                    if(print_valor_pos==1){
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

                    if(enter==0 && print_valor_pos==1){
                        pc.printf("\rclicaram em enter viu\n");
                        if(determinar_ponto==1){
                            coleta[0]=step_x;
                            coleta[1]=step_y;
                            coleta[2]=step_z;
                            determinar_ponto=0;
                        }
                        if(determinar_ponto==0){//determinar 
                            solta[0]=step_x;
                            solta[1]=step_y;
                            solta[2]=step_z;
                            print_valor_pos=0;    
                            pc.printf("\ragora vamos printar\n");
 
                        }
                    }
                    
                    atual[0]=step_x;
                    atual[1]=step_y;
                    atual[2]=step_z;   

                    //calculo para as posições 
                    for(int i = 0; i < 3; ++i) {
                                distancia_coleta_atual[i]=coleta[i]-atual[i];
                                distancia_solta_coleta[i]=solta[i]-coleta[i];
                            } 

                    if(print_valor_pos==0){//fazendo calculo para distancia entre o ponto de coleta e o atual
                        for(int i = 0; i < 3; ++i) {
                                printf("%d\n", distancia_coleta_atual[i]);
                            }                             
                    }
                    //local atual para de coleta 
                     if(tipo_de_movimento==0 && rotina_principal==1){
                        tempo_de_funcionamento.reset();//zera para caso o valor nao seja 0
                        tempo_de_funcionamento.start();//começa o contador de tempo
                        //para X
                        int m_dx= distancia_coleta_atual[0];
                        int dx=m_dx/4;
                        pc.printf("%d",m_dx);
                        
                        if(m_dx>0){
                            for (int e =0; e<dx;e++){
                            motor_x_sentido_1(vx);
                            }
                        }
                        
                        if(m_dx<0){
                            for (int e =0; e<dx;e++){
                            motor_x_sentido_2(vx);
                            }
                        }
                        //para Y
                        int m_dy= distancia_coleta_atual[1];
                        int dy=m_dy/4;
                        pc.printf("%d",m_dy);

                        if(m_dy>0){
                            for (int e =0; e<dy;e++){
                            motor_y_sentido_1(vy);
                            }
                        }
                        if(m_dy<0){
                            for (int e =0; e<dy;e++){
                            motor_y_sentido_2(vy);
                            }
                        }
                        //para Z
                        int m_dz= distancia_coleta_atual[2];
                        int dz=m_dz/4;
                        pc.printf("%d",m_dz);
                        
                        if(m_dz>0){
                            for (int e =0; e<dz;e++){
                            motor_z_sentido_1(vz);
                            }
                        }
                        if(m_dz<0){
                            for (int e =0; e<dz;e++){
                            motor_z_sentido_2(vz);
                            }
                    }
                    
                    tipo_de_movimento=1;//movimentação da rotina principal
                    }
                    //movimento do ponto de COLETA para SOLTA
                    if(tipo_de_movimento==1 && rotina_principal==1){ 
                            int m_dx= distancia_solta_coleta[0];
                            int dx=m_dx/4;
                            pc.printf("%d",m_dx);

                            if(m_dx>0){
                            for (int e =0; e<dx;e++){
                            motor_x_sentido_1(vx);
                                }
                            }
                            if(m_dx<0){
                                for (int e =0; e<dx;e++){
                                motor_x_sentido_2(vx);
                                }
                            }
                            //para Y
                            int m_dy= distancia_solta_coleta[1];
                            int dy=m_dy/4;
                            pc.printf("%d",m_dy);
                            if(m_dy>0){
                                for (int e =0; e<dy;e++){
                                motor_y_sentido_1(vy);
                                }
                            }
                            if(m_dy<0){
                                for (int e =0; e<dy;e++){
                                motor_y_sentido_2(vy);
                                }
                            }
                            //para Z
                            int m_dz= distancia_solta_coleta[2];
                            int dz=m_dz/4;
                            pc.printf("%d",m_dz);
                            if(m_dz>0){
                                for (int e =0; e<dz;e++){
                                motor_z_sentido_1(vz);
                                }
                            }
                            if(m_dz<0){
                                for (int e =0; e<dz;e++){
                                motor_z_sentido_2(vz);
                                }
                            }
                        estado_sis=2;//maquina passou para o estado de conclusao
                    }

                    if(estado_sis==1){
                    print_lcd(step_x, step_y, step_z); //função de print dos pulsos e deslocamentos
                    }
                    if(estado_sis==2){
                        tft.printf("acabou a operação depois de x segundos %d", tempo_de_funcionamento.read());
                    }
                }
        }
        else{
            // be();
        }
    }


                            // int coleta[] = {[step_x, step_y, step_z,volume],[],[]};
