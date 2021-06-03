//----------------------- incluindo bibliotecas -----------------------
//#include "Arduino.h"
#include "mbed.h"
//#include <MCUFRIEND_kbv.h>

//----------------------- Configuracao do display -----------------------
//MCUFRIEND_kbv tft;
uint8_t Orientation = 1;
#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

//   !!!CALIBRAR!!!
#define Xmax 65000  // X Joystick
#define Xmin 0      // X Joystick
#define CXmin 31000 // Centro Joystick X
#define CXmax 33500 // Centro Joystick X
#define CYmin 31000 // Centro Joystick Y
#define CYmax 34500 // Centro Joystick Y
#define Ymax 65000  // Y Joystick
#define Ymin 0      // Y Joystick

//----------------------- Monitor serial -----------------------
Serial pc(USBTX, USBRX);

//----------------------- Definição das portas dos motores -----------------------
BusOut motor_x(PC_4, PB_13, PB_14, PB_1);
BusOut motor_y(PB_2, PB_11, PB_12, PA_11);
BusOut motor_z(PA_12, PC_5, PC_6, PC_8);

//----------------------- Declaração das portas dos leds -----------------------
DigitalOut led_vermelho(PD_2);
DigitalOut led_verde(PC_11);
DigitalOut led_amarelo(PC_10);
DigitalOut led_azul(PC_12);

//----------------------- Botoes -----------------------
InterruptIn bot_emerg(PC_13); // Botão de emergência
DigitalIn endstops(PA_15);
DigitalIn enter(PB_15);
DigitalIn z1(PA_13); // movimentacao em Z+
DigitalIn z2(PC_15); // movimentacao em Z-

//----------------------- Declaração das portas do Joystick (x e y) -----------------------
AnalogIn Ax(PC_3);
AnalogIn Ay(PC_2);

// ----------------------- variaveis relacionadas ao motor e o joystick -----------------------
int x, y, vx, vy, vz;

//----------------------- variaveis variaveis responsaveis pela logica -----------------------
bool determinar_ponto = true; // 1 -> ponto de coleta; 0 -> ponto de solta
bool print_valor_pos = true;
bool tipo_de_movimento = false; // 0 atual para coleta ou 1 de pega para solta
bool rotina_principal = false;  

//----------------------- antigo Controlador -----------------------


bool ref_feito[3]={false, false, false};
bool enable = true; // 0 -> emergencia; -> 1 funcionamento normal; 2-> fim do processo
volatile bool emergencia = false;
int soltas = 0;
int max_coord[3];
int min_coord[3];
// ------ arrays ------
int coleta[3]={0,0,0};
int atual[3]={0,0,0};
int distancia_coleta_atual[3]={0,0,0};
int distancia_solta_coleta[3]={0,0,0};
int step[3]={0,0,0};
int step_rev[3]={512,512,512}; // passo/rev motor x,y,z
int passo[3]={3,3,10};    // passo x, y, z (FUSO)
int tempo=3;
int destino[3];

void aciona_motor(int tempo, bool sentido, BusOut &motor) {
    if (sentido) {
        for (int i = 0; i < 4; i++) {
            motor = 1 << i; // verificar se o estado do sistema eh 0 ou 1
            wait_ms(tempo);
        }
    } else {
        for (int i = 3; i > -1; i--) {
            motor = 1 << i;
            wait_ms(tempo);
        }
    }
}

void desliga_motor(BusOut &motor) { motor = 0, 0, 0;}
    
int coord[3];
int volume_desejado;
int volume_atual;


int solta[9];

//emerg(step
void emerg(int s1[3], bool ref[3], bool en, bool emerge) {
    for (int i = 0; i < 3; i++) {
        s1[i] = 0;
        ref[i] = false;
    }
    en = false;
    emerge = true;
}

void eixo_refere_x() {
    pc.printf("referenciando eixo x\r\n");
    bool finding_max = true;
    if (emergencia) return;
    bool first_read = endstops.read();
    if (finding_max) {
        if (emergencia) return;
        aciona_motor(tempo, true, motor_x);
        wait_ms(3);
        bool bateu = first_read && endstops.read();
        if (bateu) {
            finding_max = false;
            max_coord[0] = step[0];
            if (emergencia) return;
            aciona_motor(10, true, motor_x);
        }
    } else {
        if (emergencia) return;
        aciona_motor(tempo, false, motor_x);
        wait_ms(3);
        bool bateu = first_read && endstops.read();
        if (bateu) {
            ref_feito[0] = true;
            min_coord[0] = step[0];
            if (emergencia) return;
            aciona_motor(10, false, motor_x);
        }
    }
}
