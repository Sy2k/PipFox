//----------------------- incluindo bibliotecas -----------------------
#include "Arduino.h"
#include "mbed.h"
#include <MCUFRIEND_kbv.h>

//----------------------- Configuracao do display -----------------------
MCUFRIEND_kbv tft;
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
BusOut motores[3] = {motor_x, motor_y, motor_z};

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
bool rotina_principal = false;  // 1 para rotina principal e 0 para outras rotinas

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

void desliga_motor(BusOut &motor) { motor = 0, 0, 0, 0; }

struct PontoSolta {
    int coord[3];
    int volume_desejado;
    int volume_atual;
};

struct Controlador {
    bool ref_feito[3];
    bool enable; // 0 -> emergencia; -> 1 funcionamento normal;
    volatile bool emergencia;
    int soltas;
    int max_coord[3];
    int min_coord[3];
    // ------ arrays ------
    PontoSolta solta[9];
    int coleta[3];
    int atual[3];
    int distancia_coleta_atual[3];
    int distancia_solta_coleta[3];
    int step[3];
    int step_rev[3]; // passo/rev motor x,y,z
    int passo[3];    // passo x, y, z (FUSO)
    int tempo;
    int destino[3];

    // --------------------- Rotina emergencia, display ---------------------------
    void variavel_default() {
        for (int i = 0; i < 3; i++) {
            ref_feito[i] = false;
            coleta[i] = 0;
            atual[i] = 0;
            distancia_coleta_atual[i] = 0;
            distancia_solta_coleta[i] = 0;
            step[i] = 0;
            step_rev[i] = 512;
        }
        passo[0] = 3;
        passo[1] = 3;
        passo[2] = 10;
        enable = true; // 0 -> emergencia; -> 1 funcionamento normal;
        emergencia = false;
        soltas = 0;
        tempo = 3;
    }

    void emerg() {
        for (int i = 0; i < 3; i++) {
            step[i] = 0;
            ref_feito[i] = false;
        }
        enable = false;
        emergencia = true;
    }

    void display() {
        tft.setTextColor(GREEN);
        tft.setTextSize(3);
        tft.setCursor(0, 10); //  Orientação do texto X,Y
        int dist_x = step[0] * passo[0] / step_rev[0];
        int dist_y = step[1] * passo[1] / step_rev[1];
        int dist_z = step[2] * passo[2] / step_rev[2];
        tft.printf("distancia_x=%.2f\ndistancia_y=%.2f\ndistancia_z=%.f\npassos_x=%."
                   "0f\npassos_y=%.0f",
                   dist_x, dist_y, dist_z, step[0], step[1]);
        tft.printf("\npassos_z=%.0f", step[2]);
    }

    void eixo_refere() {
        for (int i = 0; i < 3; i++) {
            pc.printf("referenciando eixo %d\r\n", i);
            bool finding_max = true;
            while (!ref_feito[i]) {
                if (emergencia) return;
                bool first_read = endstops.read();
                if (finding_max) {
                    if (emergencia) return;
                    aciona_motor(tempo, true, motores[i]);
                    wait_ms(3);
                    bool bateu = first_read && endstops.read();
                    if (bateu) {
                        finding_max = false;
                        max_coord[i] = step[i - 2];
                        if (emergencia) return;
                        aciona_motor(10, false, motores[i]);
                    }
                } else {
                    if (emergencia) return;
                    aciona_motor(tempo, false, motores[i]);
                    wait_ms(3);
                    bool bateu = first_read && endstops.read();
                    if (bateu) {
                        ref_feito[i] = true;
                        min_coord[i] = step[i - 2];
                        if (emergencia) return;
                        aciona_motor(10, true, motores[i]);
                    }
                }
            }
        }
    }

    void motor_joystick(int x, int y, bool z1, bool z2) {
        // BusOut motores[3] = {motor_x, motor_y, motor_z};
        if (emergencia) return;
        if (enable) {
            if (x > CXmax && step[0] < max_coord[0]) {
                if (emergencia) return;
                aciona_motor(tempo, true, motores[0]);
                step[0] += 4;
            } else if (x < CXmin && step[0] > min_coord[0]) {
                if (emergencia) return;
                aciona_motor(tempo, false, motores[0]);
                step[0] -= 4;
            } else {
                desliga_motor(motores[0]);
            }

            if (y > CYmax && step[1] < max_coord[1]) {
                if (emergencia) return;
                aciona_motor(tempo, true, motores[1]);
                step[1] += 4;
            } else if (y < CYmin && step[1] > min_coord[1]) {
                if (emergencia) return;
                aciona_motor(tempo, false, motores[1]);
                step[1] -= 4;
            } else {
                desliga_motor(motores[1]);
            }

            if (z1 && step[2] < max_coord[2]) {
                if (emergencia) return;
                aciona_motor(tempo, true, motores[2]);
                step[2] += 4;
            } else if (z2 && step[2] > min_coord[2]) {
                if (emergencia) return;
                aciona_motor(tempo, false, motores[2]);
                step[2] -= 4;
            } else {
                desliga_motor(motores[2]);
            }
            display();
        }
    }

    void ponto_coleta() {
        pc.printf("determinando coleta\r\n");
        coleta[0] = step[0];
        coleta[1] = step[1];
        coleta[2] = step[2];
    }

    void ponto_solta(int volume_desejado) {
        pc.printf("determinando solta\r\n");
        solta[soltas].coord[0] = step[0];
        solta[soltas].coord[1] = step[1];
        solta[soltas].coord[2] = step[2];
        solta[soltas].volume_desejado = volume_desejado;
    }

    void ir_ponto(int destino[3]) {
        // BusOut motores[3] = {motor_x, motor_y, motor_z};
        // Levantando pipeta no maximo
        while (step[2] < max_coord[2]) {
            if (emergencia) return;
            aciona_motor(tempo, false, motores[2]);
        }
        // Arrumando eixo x e y
        for (int i = 0; i < 2; i++) {
            // indo com eixo x e y para SAH
            while (step[i] < destino[i]) {
                if (emergencia) return;
                aciona_motor(tempo, false, motores[i]);
            }
            // indo com eixo x e y para SH
            while (step[i] > destino[i]) {
                if (emergencia) return;
                aciona_motor(tempo, true, motores[i]);
            }
        }
        // Descendo pipeta para ponto desejado
        while (step[2] < destino[2]) {
            if (emergencia) return;
            aciona_motor(tempo, false, motores[2]);
        }
    }

    void coletar() {
        ir_ponto(coleta);
        led_azul = true;
    }

    void soltar() {
        ir_ponto(solta[soltas].coord);
        led_azul = false;
        solta[soltas].volume_atual++;
        if (solta[soltas].volume_atual == solta[soltas].volume_desejado) {
            soltas++;
        }
    }
};

Controlador Controlador1;

void setup() { Controlador1.variavel_default(); }

void loop() {
    x = Ax.read_u16(); // ou Ax.read*1000()
    y = Ay.read_u16(); // ou Ay.read*1000()
    Controlador1.motor_joystick(x, y, z1, z2);
}