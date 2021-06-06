//----------------------- incluindo bibliotecas -----------------------
#include "Arduino.h"
#include "WiiNunchuck.h"
#include "mbed.h"
#include <MCUFRIEND_kbv.h>
#include "TouchScreen_kbv_mbed.h"
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
#define Xmax 220  // X Joystick
#define Xmin 20   // X Joystick
#define CXmin 125 // Centro Joystick X
#define CXmax 135 // Centro Joystick X
#define CYmin 120 // Centro Joystick Y
#define CYmax 130 // Centro Joystick Y
#define Ymax 210  // Y Joystick
#define Ymin 20   // Y Joystick

#define MINPRESSURE 10
#define MAXPRESSURE 1000

bool mais = 0;
bool menos = 0;
int vol  = 0;
int k = 0;



int pos_x[10];
int pos_y[10];
int pos_z[10];
int vol_ml[9];

//**********Configurações Touch***********//
const int TS_LEFT=121,TS_RT=922,TS_TOP=82,TS_BOT=890;
const PinName XP = D8, YP = A3, XM = A2, YM = D9;   //next common configuration
DigitalInOut YPout(YP);
DigitalInOut XMout(XM);

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

TouchScreen_kbv ts = TouchScreen_kbv(XP, YP, XM, YM, 300);
TSPoint_kbv tp;

//****************************************************************************//

//-----------------------WII---------------------------

#define p_sda PB_9
#define p_scl PB_8
WiiNunchuck Jorge(p_sda, p_scl);

//----------------------- Monitor serial -----------------------
Serial pc(USBTX, USBRX);

//----------------------- Definição das portas dos motores -----------------------
BusOut motor_x(PC_4, PB_13, PB_14, PB_1);
BusOut motor_y(PB_2, PB_11, PB_12, PA_11);
BusOut motor_z(PA_12, PC_5, PC_6, PC_8);
BusOut motores[3] = {BusOut(PC_4, PB_13, PB_14, PB_1), BusOut(PB_2, PB_11, PB_12, PA_11),
                     BusOut(PA_12, PC_5, PC_6, PC_8)};

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
DigitalIn z2(PB_7); // movimentacao em Z- PA_3

//----------------------- Declaração das portas do Joystick (x e y) -----------------------
AnalogIn Ax(PC_3);
AnalogIn Ay(PC_2);

// ----------------------- variaveis relacionadas ao motor e o joystick -----------------------
int x, y, vx, vy, vz;

//-------------------------- variaveis relacionadas a coisas do display---------------
int h = 0;
int etapa_atual = 0;
int pos;
int status = 0;
//bool welcome_ = true;
//----------------------- variaveis variaveis responsaveis pela logica -----------------------

bool tipo_de_movimento = false; // 0 atual para coleta ou 1 de pega para solta
bool rotina_principal = false;  // 1 para rotina principal e 0 para outras rotinas
int()


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

//***********************Funções da tela touch********************************//


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
    bool determinar_ponto; 
    //int qual_tela;
    bool det_vol;
    bool pontos_finalizados;
    bool inicializacao;
    //bool welcome;
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
        determinar_ponto = true; // 1 -> ponto de coleta; 0 -> ponto de solta
        //qual_tela = 0; //0 -> welcome; 1->referenciando; 2 -> ponto de solta; 3-> ponto de coleta
        passo[0] = 3;
        passo[1] = 3;
        passo[2] = 10;
        inicializacao=true;
        enable = true; // 0 -> emergencia; -> 1 funcionamento normal;
        emergencia = false;
        soltas = 0;
        tempo = 3;
        det_vol= false;
        pontos_finalizados=false;
    }

    void emerg() {
        pc.printf("emergencia \r\n");
        for (int i = 0; i < 3; i++) {
            step[i] = 0;
            ref_feito[i] = false;
        }
        enable = false;
        emergencia = true;
    }

    void sair_emerg() {
        pc.printf("sair emergencia \r\n");
        enable = true;
        emergencia = false;
        welcome_=true;
        variavel_default();
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
                    step[i] += 4;
                    wait_ms(3); // "Debounce" - first_read
                    bool bateu = first_read && endstops.read();
                    if (!bateu) {
                        finding_max = false;
                        max_coord[i] = step[i];
                        if (emergencia) return;
                        aciona_motor(3, false, motores[i]);
                        wait(1);
                    }

                } else {
                    if (emergencia) return;
                    aciona_motor(tempo, false, motores[i]);
                    step[i] -= 4;
                    wait_ms(3);
                    bool bateu = first_read && endstops.read();
                    if (!bateu) {
                        ref_feito[i] = true;
                        min_coord[i] = step[i];
                        if (emergencia) return;
                        aciona_motor(3, true, motores[i]);
                        wait(1);
                    }
                }

            }
        }
        pc.printf("referenciamento_concluido");
        pc.printf("max_coord x:%d y:%d z:%d \r\n", max_coord[0], max_coord[1],max_coord[2]);
        pc.printf("min_coord x:%d y:%d z:%d \r\n", min_coord[0], min_coord[1],min_coord[2]);
    }

    void motor_joystick(int x, int y, DigitalIn &z11, DigitalIn &z22) {
        if (emergencia) return;
        bool bateu = endstops.read();
        if (enable && bateu) {
            if (x > CXmax && step[0] < max_coord[0]) {
                if (emergencia) return;
                pc.printf("Motor X sentido 1\r\n");
                aciona_motor(tempo, true, motores[0]);
                step[0] += 4;
            } else if (x < CXmin && step[0] > min_coord[0]) {
                if (emergencia) return;
                pc.printf("Motor X sentido 2\r\n");
                aciona_motor(tempo, false, motores[0]);
                step[0] -= 4;
            } else {
                // pc.printf("Motor X desligado\r\n");
                desliga_motor(motores[0]);
            }

            if (y > CYmax && step[1] < max_coord[1]) {
                if (emergencia) return;
                pc.printf("Motor Y sentido 1\r\n");
                aciona_motor(tempo, true, motores[1]);
                step[1] += 4;
            } else if (y < CYmin && step[1] > min_coord[1]) {
                if (emergencia) return;
                pc.printf("Motor Y sentido 2\r\n");
                aciona_motor(tempo, false, motores[1]);
                step[1] -= 4;
            } else {
                // pc.printf("Motor Y desligado\r\n");
                desliga_motor(motores[1]);
            }
            bool estado = z11;
            bool estado2 = z22;
            //pc.printf("estado2 %d\r\n", estado2);

            wait(0.1);
            bool bateu_z1 = z11 && estado;
            bool bateu_z2 = z22 && estado2;
            //pc.printf("bateu_z2 %d\r\n", bateu_z2);
            
            if (!bateu_z1 && step[2] < max_coord[2]) {
                if (emergencia) return;
                aciona_motor(tempo, true, motores[2]);
                pc.printf("Motor Z sentido 1\r\n");
                step[2] += 4;
            } else if (!bateu_z2 && step[2] > min_coord[2]) {
                if (emergencia) return;
                pc.printf("Motor Z sentido 2\r\n");
                aciona_motor(tempo, false, motores[2]);
                step[2] -= 4;
            } else {
                desliga_motor(motores[2]);
            }
            //display();
        }
    }

    void ponto_coleta() {
        pc.printf("determinando coleta\r\n");
        coleta[0] = step[0];
        coleta[1] = step[1];
        coleta[2] = step[2];
        determinar_ponto=false;
    }

    void ponto_solta(int volume_desejado) {
        pc.printf("determinando solta\r\n");
        solta[soltas].coord[0] = step[0];
        solta[soltas].coord[1] = step[1];
        solta[soltas].coord[2] = step[2];
        solta[soltas].volume_desejado = volume_desejado;
    }

    void ir_ponto(int destino[3]) {
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
void fail_safe() { Controlador1.emerg(); }
void sair_failsafe() { Controlador1.sair_emerg(); }
void setup() {
    Controlador1.variavel_default();
    bot_emerg.mode(PullUp); // definição do botão de emergencia como PullUp
    bot_emerg.fall(&fail_safe);
    bot_emerg.rise(&sair_failsafe);

    //COISAS TELA
    tft.reset();
    tft.begin();
    tft.setRotation(Orientation);
    tft.fillScreen(BLACK);  // Fundo do Display
    wait(0.75);
}

void loop() {
    x = Jorge.joyx();
    y = Jorge.joyy();

    if (Controlador1.enable) {
        // feito o referencimaneto - movimentação do eixo x, y, z

        if(Controlador1.inicializacao){//inicialização da tela com mensagem de bem-vindo 
        //RODAR UMA VEZ
            welcome();  
            Controlador1.inicializacao = false;
        }

        bool estado_ref = Controlador1.ref_feito[0] && Controlador1.ref_feito[1] && Controlador1.ref_feito[2];
        else if(!estado_ref){
            //mostrar tela para iniciar ref caso nao esteja referenciado
            tela_referenciamento_xyz();
            bool estado_enter = enter;
           
            wait(0.1);
            bool enter_deb = enter && estado_enter;

            while (enter != 0)//nao sei se precisa desse while
            {
               if(!enter_deb){
                    pc.printf("referenciando\r\n");
                    Controlador1.eixo_refere();
                }
            }
        }
        else if(Controlador1.estado_ref){//estando referenciado
            if(Controlador1.determinar_ponto){//ponto de coleta
                bool estado_enter = enter;
                wait(0.1);
                bool enter_deb = enter && estado_enter;

                while (enter_deb)//fica esperando enquanto valor=1
                {
                    tela_selecao_ponto_coleta();
                    Controlador1.motor_joystick(x, y, z1, z2);
                    if(!enter_deb){
                        pc.printf("determinando ponto de coleta\r\n");
                        Controlador1.ponto_coleta();
                    }
                }
            }


            if(!Controlador1.determinar_ponto && !Controlador1.pontos_finalizados){//ponto de solta
                tela_selecao_ponto_coleta();
                Controlador1.motor_joystick(x, y, z1, z2);

                bool estado_enter = enter;
                wait(0.1);
                bool enter_deb = enter && estado_enter;
                bool outro_ponto = true;

                if (enter_deb && outro_ponto)//se botao for pressionado entra
                {                    
                    //while(!enter_deb){//enquanto botao nao ta pressionado
                        while(!Controlador1.det_vol){//enquanto volume naquele ponto nao tiver sido determinado
                            pc.printf("determinando ponto numero X de solta\r\n");
                            tela_selecao_volume();

                            bool estado_enter = enter;
                            wait(0.1);
                            bool enter_deb = enter && estado_enter;

                            if(enter_deb==0){
                                Controlador1.det_vol=true;
                                Controlador1.ponto_coleta(volume);
                            }
                        }

                        tela_perguntando_se_tem_mais_pontos();
                        //caso n tenha mais pontos alterar variavel "outro_ponto" para 0
                    }
                    
                }
            }

        }
        
    
    }