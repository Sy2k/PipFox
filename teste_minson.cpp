//************************ Biblioteca
//*****************************************//
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

// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
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

//--------------- Definição das portas dos motores --------------
BusOut motor_x(PC_4, PB_13, PB_14, PB_1);
BusOut motor_y(PB_2, PB_11, PB_12, PA_11);
BusOut motor_z(PA_12, PC_5, PC_6, PC_8);

//----------------------- Botoes -----------------------
InterruptIn botao_emergencia(PC_13); // BE
DigitalIn endstops(PA_15);           // Endstop
DigitalIn enter(PB_15);              // Enter
DigitalIn z1(PA_13);                 // movimentacao emZ+
DigitalIn z2(PC_15);                 // movimentacao em Z-

//-------- Declaração das portas do Joystick (x e y)
//-----------------------
AnalogIn Ax(PC_3);
AnalogIn Ay(PC_2);

//----------------------- Declaração das portas dos leds -----------------------
DigitalOut led_vermelho(PD_2);
DigitalOut led_verde(PC_11);
DigitalOut led_amarelo(PC_10);
DigitalOut led_azul(PC_12);

//----------------------- debounce para o botao de emergencia
//-----------------------
Timer debounce_emer;
Timer debounce_endstop;
Timer debounce_enter;
Timer tempo_de_funcionamento;

//----------------------- arrays -----------------------
char solta[3];
char coleta[3];
char atual[3];
char distancia_coleta_atual[3];
char distancia_solta_coleta[3];

// ----------------------- passos dos fusos e motor-----------------------
int passo_x = 3;       // avanço linear do eixo X
int passo_y = 3;       // avanço linear do eixo Y
int passo_z = 10;      // avanço linear do eixo Z
int steps_rev_x = 512; // passos por revolução dos motores
int steps_rev_y = 512; // passos por revolução dos motores
int steps_rev_z = 512; // passos por revolução dos motores

//---------- sistema está em estado de emergência ou não------------
int estado_sis = 1; // 0 emergencia;1 funcionamento normal; 2 fim do processo
int posicao = 0;

// ----------------------- contadores de passos -----------------------
int step_x = 0;
int step_y = 0;
int step_z = 0;

// ---------- variaveis relacionadas ao motor e o joystick -------------
int x, y, vx, vx_inv, vy, vy_inv, vz, vz_inv;
bool movendo_em_x, movendo_em_y, movendo_em_z;

//----------------------- variáveis de referenciamento -----------------------
bool ref_x_feito = 0;
bool ref_y_feito = 0;
bool ref_z_feito = 0;

//-------- variaveis variaveis responsaveis pela logica --------------
bool determinar_ponto = 1; // 1 -> ponto de coleta; 0 -> ponto de solta
bool print_valor_pos = 1;
bool tipo_de_movimento = 0; // 0 atual para coleta ou 1 de pega para solta
bool rotina_principal = 0;  // 1 para rotina principal e 0 para outras rotinas

void print_lcd(int step_x, int step_y, int step_z) {
    tft.setTextColor(GREEN);
    tft.setTextSize(3);
    tft.setCursor(0, 10);                             //  Orientação do texto X,Y
    int distancia_x = step_x * passo_x / steps_rev_x; // calculo da distancia percorrida em x
    int distancia_y = step_y * passo_y / steps_rev_y; // calculo da distancia percorrida em y
    int distancia_z = step_z * passo_z / steps_rev_z; // calculo da distancia percorrida em z
    tft.printf("distancia_x=%.2f\ndistancia_y=%.2f\ndistancia_z=%.f\npassos_x=%."
               "0f\npassos_y=%.0f",
               distancia_x, distancia_y, distancia_z, step_x, step_y);
    tft.printf("\npassos_z=%.0f", step_z);
}

void motor_x_sentido_1(int tempo) {
    for (int i = 0; i < 4; i++) {
        motor_x = 1 << i; // verificar se o estado do sistema eh 0 ou 1
        wait_ms(tempo);
    }
}

void motor_x_sentido_2(int tempo) {
    for (int i = 3; i > -1; i--) {
        motor_x = 1 << i;
        wait_ms(tempo);
    }
}

void motor_y_sentido_1(int tempo) {
    for (int i = 0; i < 4; i++) {
        motor_y = 1 << i;
        wait_ms(tempo);
    }
}

void motor_y_sentido_2(int tempo) {
    for (int i = 3; i > -1; i--) {
        motor_y = 1 << i;
        wait_ms(tempo);
    }
}

void motor_z_sentido_1(int tempo) {
    for (int z = 0; z < 4; z++) {
        motor_z = 1 << z;
        wait_ms(tempo);
    }
}

void motor_z_sentido_2(int tempo) {
    for (int z = 3; z > -1; z--) {
        motor_z = 1 << z;
        wait_ms(tempo);
    }
}

void Mx_off() {
    motor_x = 0, 0, 0, 0; // mao desligar o motor - somente cortar a alimentacao
}
void My_off() { motor_y = 0, 0, 0, 0; }
void Mz_off() { motor_z = 0, 0, 0, 0; }

void ref() {
    if (movendo_em_x == 1) {
        pc.printf("\r referenciou em x\n");
        // lcd.printf("\r referenciou em x\n");
        movendo_em_x = 0;
        ref_x_feito = 1;
        Mx_off();
        debounce_endstop.start();
    }

    else if (movendo_em_y == 1 && debounce_endstop.read_ms() > 250) {
        pc.printf("\r referenciou em y\n");
        // lcd.printf("\r referenciou em y\n");
        movendo_em_y = 0;
        ref_y_feito = 1;
        My_off();
        debounce_endstop.reset();
        debounce_endstop.start();
    }

    else if (movendo_em_z == 1 && debounce_endstop.read_ms() > 250) {
        pc.printf("\r referenciou em z\n");
        // lcd.printf("\r referenciou em z\n");
        movendo_em_z = 0;
        ref_z_feito = 1;
        Mz_off();
        debounce_endstop.reset();
        step_x = 0;
        step_y = 0;
        step_z = 0;
        pc.printf("\rreferenciado e esperando enter para determinar ponto de "
                  "coleta\n");
        wait(3);
    }
}

void be() {
    estado_sis = 0; // entrou em estado de emergencia
    step_x = 0;     // pensar se precisa checar antes de zerar o valor
    step_y = 0;
    step_z = 0;
    ref_x_feito = 0; // zerar o referenciamento de todos os eixos
    ref_y_feito = 0;
    ref_z_feito = 0;
    determinar_ponto = 1;
    print_valor_pos = 1;
    tipo_de_movimento = 0;
    posicao = 0;
    rotina_principal = 0;
    for (int i = 0; i < 3; ++i) {
        solta[i] = 0;
        coleta[i] = 0;
    }
    debounce_emer.start(); // iniciar timer para debounce
    pc.printf("\r estado de emergencia\n");
    // lcd.printf("\r estado de emergencia\n");
    // entrada no estado de emergencia e perdendo o referenciamento com botao de
    // emergencia
}
void sair_emer() {
    if (debounce_emer.read_ms() > 15) { // CHECAR
        estado_sis = 1;
        ref_x_feito = 0; // zerar o referenciamento de todos os eixos
        ref_y_feito = 0;
        ref_z_feito = 0;
        determinar_ponto = 1;
        tipo_de_movimento = 0;
        print_valor_pos = 1;
        posicao = 0;
        rotina_principal = 0;
        debounce_emer.reset(); // resetar timer de reset
        pc.printf("\r saindo do estado de emergência\n");
        // lcd.printf("\r saindo do estado de emergência\n");
    }
}

void setup(void) {
    tft.reset();
    tft.begin();
    tft.setRotation(Orientation);
    tft.fillScreen(BLACK); // Fundo do Display
    print_lcd(step_x, step_y,
              step_z); // PRA MIM ISSO NAO FAZ SENTIDO ESTAR AQUI DENTRO
    delay(1000);       // tava 1000 antes, ACHEI DEMAIS

    Mz_off();
    Mx_off();
    My_off();
    movendo_em_x = 0;
    movendo_em_y = 0;
    movendo_em_z = 0;
    // valores de tempo entre cada passo determinado como 1 ms
    vx = 3;
    vx_inv = vx;
    vy = 3;
    vy_inv = vy;
    vz = 3;
    vz_inv = vz;
    botao_emergencia.mode(PullUp); // definição do botão de emergencia como PullUp
                                   // -> ISSO DAQUI TA ESTRANHO
    pc.baud(9600);                 // definição do baud rate da comunicação serial usb
    botao_emergencia.fall(&be);
    botao_emergencia.rise(&sair_emer);
    endstops.fall(&ref);
}
void loop(void) {
    x = Ax.read_u16();     // ou Ax.read*1000()
    y = Ay.read_u16();     // ou Ay.read*1000()
    if (estado_sis != 0) { // não está em estado emergencia

        // HOMING - referenciamento dos eixos
        if (ref_x_feito == 0) {
            movendo_em_x = 1;
            while (ref_x_feito == 0) {
                pc.printf("\rdentro_refx\n");
                // lcd.printf("\rdentro_refx\n");
                motor_x_sentido_1(vx);
                if (estado_sis == 0) {
                    break;
                }
            }
        } else if (ref_y_feito == 0) {
            movendo_em_y = 1;
            while (ref_y_feito == 0) {
                pc.printf("\rdentro_refy\n");
                // lcd.printf("\rdentro_refy\n");
                motor_y_sentido_1(vy);
                if (estado_sis == 0) {
                    break;
                }
            }
        }

        else if (ref_z_feito == 0) {
            movendo_em_z = 1;
            while (ref_z_feito == 0) {
                pc.printf("\rdentro_refz\n");
                // lcd.printf("\rdentro_refz\n");
                motor_z_sentido_1(vz);
                if (estado_sis == 0) {
                    break;
                }
            }
        }
        // if(ref_z_feito==1){
        //         pc.printf("\rreferenciado e esperando enter para determinar
        //         ponto de coleta\n");
        //     }

        else {
            if (rotina_principal == 0) { // rotina principal que nao eh a de
                                         // movimentacao automatica (PIPETAGEM)
                // Condições para a movimentação dependendo da posição do
                // joystick
                if (x > CXmax && estado_sis == 1) {
                    // int vx = map(x, CXmax, Xmax, 5, 0.5);
                    motor_x_sentido_1(vx); // dando certo
                    pc.printf("\rmotor_x_sentido1\n");
                    step_x += 4;
                }
                if (x < CXmin && estado_sis == 1) {
                    // int vx_inv = map(x, CXmin, Xmin, 0.5, 5);
                    motor_x_sentido_2(vx_inv); // dando errado
                    step_x -= 4;
                }
                if (y > CYmax && estado_sis == 1) {
                    // int vy = map(y, CYmax, Ymax, 5, 0.5);
                    motor_y_sentido_1(vy);
                    step_y += 4;
                }
                if (y < CYmin && estado_sis == 1) {
                    // int vy_inv = map(y, CYmin ,Ymin, 0.5, 5);
                    motor_y_sentido_2(vy_inv);
                    step_y -= 4;
                }

                if (z1 == 1) {
                    motor_z_sentido_1(vz);
                }

                if (z2 == 1) {
                    motor_z_sentido_2(vz_inv);
                }
                if (z1 == 0 || z2 == 0) {
                    Mz_off();
                }
                if (estado_sis == 1) {
                    print_lcd(step_x, step_y,
                              step_z); // função de print dos pulsos e deslocamentos
                }
                if (enter == 0 && print_valor_pos == 1 && posicao == 0) {
                    pc.printf("\rdeterminando coleta\n");
                    coleta[0] = step_x;
                    coleta[1] = step_y;
                    coleta[2] = step_z;
                    determinar_ponto = 0;
                    debounce_enter.start();
                }
                if (determinar_ponto == 0 && enter == 0 && posicao == 0 &&
                    debounce_enter.read_ms() > 250) { // determinar
                    pc.printf("\rdeterminando solta e esperando enter para "
                              "iniciar calculos\n");
                    solta[0] = step_x;
                    solta[1] = step_y;
                    solta[2] = step_z;
                    posicao = 1;
                    debounce_enter.reset();
                    debounce_enter.start();
                }

                // calculo para as posições

                if (posicao == 1 && enter == 0 && debounce_enter.read_ms() > 250) {
                    atual[0] = step_x;
                    atual[1] = step_y;
                    atual[2] = step_z;
                    for (int i = 0; i < 3; ++i) {
                        distancia_coleta_atual[i] = coleta[i] - atual[i];
                        distancia_solta_coleta[i] = solta[i] - coleta[i];
                    }
                    pc.printf("\rguardando posicao \n");
                    // print_valor_pos=0;
                    pc.printf("\ragora vamos printar\n");

                    for (int i = 0; i < 3; ++i) {
                        printf("dist coleta para atual: %d\n", distancia_coleta_atual[i]);
                    }
                    wait(3);
                    for (int i = 0; i < 3; ++i) {
                        printf("dist solta para coleta: %d\n", distancia_solta_coleta[i]);
                    }
                    wait(3);
                    rotina_principal = 1;
                    debounce_enter.reset();
                }
                // if(print_valor_pos==0){//fazendo calculo para distancia entre
                // o ponto de coleta e o atual
            }
            //}
            // local atual para de coleta
            else {
                if (tipo_de_movimento == 0 &&
                    rotina_principal == 1) { // NAO SEI SE ISSO DEVERIA SER UM WHILE
                    pc.printf("\rcomecando movimentacao para ponto de coleta\n");
                    tempo_de_funcionamento.reset(); // zera para caso o valor nao seja 0
                    tempo_de_funcionamento.start(); // começa o contador de tempo
                    // para X
                    int contador = 0;

                    if (contador == 0) {
                        int m_dx = distancia_coleta_atual[0];
                        int dx = m_dx / 4;
                        pc.printf("passos em x %d", m_dx);
                        if (m_dx > 0) {
                            for (int e = 0; e < dx; e++) {
                                motor_x_sentido_1(vx);
                                step_x += 4;
                            }
                        }

                        else if (m_dx < 0) {
                            for (int e = 0; e < dx; e++) {
                                motor_x_sentido_2(vx);
                                step_x -= 4;
                            }
                        }
                        contador += 1;
                    }
                    // para Y
                    if (contador == 1) {

                        int m_dy = distancia_coleta_atual[1];
                        int dy = m_dy / 4;
                        pc.printf("passos em y %d", m_dy);

                        if (m_dy > 0) {
                            for (int e = 0; e < dy; e++) {
                                motor_y_sentido_1(vy);
                                step_y += 4;
                            }
                        } else if (m_dy < 0) {
                            for (int e = 0; e < dy; e++) {
                                motor_y_sentido_2(vy);
                                step_y -= 4;
                            }
                        }
                        contador += 1;
                    }
                    // para Z
                    if (contador == 2) {
                        int m_dz = distancia_coleta_atual[2];
                        int dz = m_dz / 4;
                        pc.printf("passos em z %d", m_dz);

                        if (m_dz > 0) {
                            for (int e = 0; e < dz; e++) {
                                motor_z_sentido_1(vz);
                                step_z += 4;
                            }
                        } else if (m_dz < 0) {
                            for (int e = 0; e < dz; e++) {
                                motor_z_sentido_2(vz);
                                step_z -= 4;
                            }
                        }
                        contador += 1;
                    }
                    tipo_de_movimento = 1; // movimentação da rotina principal
                }
                // movimento do ponto de COLETA para SOLTA
                if (tipo_de_movimento == 1 && rotina_principal == 1) {
                    int contador = 0;

                    if (contador == 0) {

                        int m_dx = distancia_solta_coleta[0];
                        int dx = m_dx / 4;
                        pc.printf("%d", m_dx);
                        if (m_dx > 0) {
                            for (int e = 0; e < dx; e++) {
                                motor_x_sentido_1(vx);
                            }
                        }
                        if (m_dx < 0) {
                            for (int e = 0; e < dx; e++) {
                                motor_x_sentido_2(vx);
                            }
                        }
                        contador += 1;
                    }

                    if (contador == 1) {

                        // para Y
                        int m_dy = distancia_solta_coleta[1];
                        int dy = m_dy / 4;
                        pc.printf("%d", m_dy);
                        if (m_dy > 0) {
                            for (int e = 0; e < dy; e++) {
                                motor_y_sentido_1(vy);
                            }
                        }
                        if (m_dy < 0) {
                            for (int e = 0; e < dy; e++) {
                                motor_y_sentido_2(vy);
                            }
                        }
                        contador += 1;
                    }
                    if (contador == 2) {
                        // para Z
                        int m_dz = distancia_solta_coleta[2];
                        int dz = m_dz / 4;
                        pc.printf("%d", m_dz);
                        if (m_dz > 0) {
                            for (int e = 0; e < dz; e++) {
                                motor_z_sentido_1(vz);
                            }
                        }
                        if (m_dz < 0) {
                            for (int e = 0; e < dz; e++) {
                                motor_z_sentido_2(vz);
                            }
                        }
                        contador += 1;
                    }
                    estado_sis = 2; // maquina passou para o estado de conclusao
                }
            }

            if (estado_sis == 2) {
                pc.printf("acabou a operação depois de x segundos %d",
                          tempo_de_funcionamento.read());
            }
        }
    } else {
        // be();
    }
}
