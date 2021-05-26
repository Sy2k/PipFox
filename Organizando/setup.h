// --------------------------- SETUP ------------------------------------------
#ifndef FUNCAO
#define FUNCAO
//----------------------- incluindo bibliotecas -----------------------
#include "mbed.h"
#include <MCUFRIEND_kbv.h>
#include "Arduino.h"

//----------------------- Configuracao do display -----------------------
MCUFRIEND_kbv tft;
uint8_t Orientation = 1;  
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
#define Xmax 65000 // X Joystick
#define Xmin 0 // X Joystick
#define CXmin 31000 // Centro Joystick X
#define CXmax 33500 // Centro Joystick X
#define CYmin 31000 // Centro Joystick Y
#define CYmax 34500 // Centro Joystick Y
#define Ymax 65000 // Y Joystick
#define Ymin 0 // Y Joystick

//----------------------- Monitor serial -----------------------
Serial pc(USBTX,USBRX);

//----------------------- Definição das portas dos motores -----------------------
BusOut motor_x(PC_4,PB_13,PB_14,PB_1);
BusOut motor_y(PB_2,PB_11,PB_12,PA_11);
BusOut motor_z(PA_12,PC_5,PC_6,PC_8);

//----------------------- Botoes -----------------------
InterruptIn botao_emergencia(PC_13); // BE
InterruptIn endstops(PA_15); // Endstop
DigitalIn enter(PB_15); // Enter
DigitalIn z1(PA_13); // movimentacao emZ+
DigitalIn z2(PC_15); // movimentacao em Z- 

//----------------------- Declaração das portas do Joystick (x e y) -----------------------
AnalogIn Ax (PC_3);
AnalogIn Ay (PC_2);

//----------------------- Declaração das portas dos leds -----------------------
DigitalOut led_vermelho(PD_2);
DigitalOut led_verde(PC_11);
DigitalOut led_amarelo(PC_10);
DigitalOut led_azul(PC_12);

//----------------------- debounce para o botao de emergencia -----------------------
Timer debounce_emer;
Timer debounce_endstop;
Timer tempo_de_funcionamento;

//----------------------- arrays -----------------------
char solta[3];
char coleta[3];
char atual[3];
char distancia_coleta_atual[3];
char distancia_solta_coleta[3];

// ----------------------- passos dos fusos e motor-----------------------
int passo_x = 3; // avanço linear do eixo X
int passo_y = 3; // avanço linear do eixo Y
int passo_z = 10; // avanço linear do eixo Z
int steps_rev_x = 512; //passos por revolução dos motores 
int steps_rev_y = 512; //passos por revolução dos motores 
int steps_rev_z = 512; //passos por revolução dos motores 

//----------------------- sistema está em estado de emergência ou não -----------------------
int estado_sis = 1; //0 -> emergencia; -> 1 funcionamento normal; 2-> fim do processo

// ----------------------- contadores de passos -----------------------
int step_x = 0;
int step_y = 0;
int step_z = 0;

// ----------------------- variaveis relacionadas ao motor e o joystick -----------------------
int x, y, vx, vx_inv, vy, vy_inv, vz, vz_inv;
bool movendo_em_x,movendo_em_y,movendo_em_z;

//----------------------- variáveis de referenciamento -----------------------
bool ref_x_feito = 0;
bool ref_y_feito = 0;
bool ref_z_feito = 0;

//variaveis variaveis responsaveis pela logica
bool determinar_ponto = 1; //1 -> ponto de coleta; 0 -> ponto de solta
bool print_valor_pos = 1;
bool tipo_de_movimento = 0; //0 atual para coleta ou 1 de pega para solta
bool rotina_principal = 0; //1 para rotina principal e 0 para outras rotinas

#endif 



