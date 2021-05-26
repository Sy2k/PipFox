// --------------------------- SETUP ------------------------------------------
#ifndef FUNCAO
#define FUNCAO
#include "TextLCD.h"
#include "mbed.h"//Definição das portas dos motores
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
int x, y;
bool movendo_em_x,movendo_em_y,movendo_em_z;

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

#endif 
