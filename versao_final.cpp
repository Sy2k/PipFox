/*                                                      *\
|*                        SETUP                         *|
\*                                                      */
//----------------------- incluindo bibliotecas -----------------------
#include "Arduino.h"
#include "TouchScreen_kbv_mbed.h"
#include "WiiNunchuck.h"
#include "mbed.h"
#include <MCUFRIEND_kbv.h>
//----------------------- Monitor serial -----------------------
Serial pc(USBTX, USBRX);

/*                                       *\
|*   Configurações do display - Setup    *|
\*                                       */
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
#define MINPRESSURE 10
#define MAXPRESSURE 1000

bool mais = 0;
bool menos = 0;
int vol = 1;
int num_pontos_solta;

bool referenciando = false; // variavel booleana para saber se esta referenciando ou n
bool fail_safe_out = false; // booleana para saber se esta referenciando ou n

Timer funcionamento;

int pos_x[10];
int pos_y[10];
int pos_z[10];
int vol_ml[9];

//--------------- Configurações Touch ---------------
const int TS_LEFT = 121, TS_RT = 922, TS_TOP = 82, TS_BOT = 890;
const PinName XP = D8, YP = A3, XM = A2, YM = D9; // next common configuration
DigitalInOut YPout(YP);
DigitalInOut XMout(XM);

long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

TouchScreen_kbv ts = TouchScreen_kbv(XP, YP, XM, YM, 300);
TSPoint_kbv tp;

//----------------------- Definição das portas dos motores -----------------------
BusOut motor_x(PC_4, PB_13, PB_14, PB_1);
BusOut motor_y(PB_2, PB_11, PB_12, PA_11); // PC_3 substituiu PB_2
BusOut motor_z(PA_12, PC_5, PC_6, PC_8);
BusOut motores[3] = {BusOut(PC_4, PB_13, PB_14, PB_1),
                     BusOut(PB_2, PB_11, PB_12, PA_11), // PC_3 substituiu PB_2
                     BusOut(PA_12, PC_5, PC_6, PC_8)};

//----------------------- WII ---------------------------
#define p_sda PB_9 // SDA
#define p_scl PB_8 // SCL
#define Xmax 220   // X Joystick
#define Xmin 20    // X Joystick
#define CXmin 125  // Centro Joystick X
#define CXmax 135  // Centro Joystick X
#define CYmin 120  // Centro Joystick Y
#define CYmax 130  // Centro Joystick Y
#define Ymax 210   // Y Joystick
#define Ymin 20    // Y Joystick
WiiNunchuck Nunchuck(p_sda, p_scl);

//----- Declaração das portas dos leds ------
DigitalOut led_vermelho(PD_2); // led: estado de emergência
DigitalOut led_verde(PC_11);   // led: ligado/desligado
DigitalOut led_amarelo(PC_10); // led: esperando uma ação do usuário
DigitalOut pipeta(PA_14);      // led para representar a pipeta

//------- Botoes - Emergênica, endstops, enter, movimentação Z+ e Z --------------
InterruptIn bot_emerg(PC_13);
DigitalIn endstops(PA_15);
DigitalIn enter(PB_15);
DigitalIn z1(PA_13);
DigitalIn z2(PB_7);

// ------------ variaveis relacionadas ao motor e o joystick -------------
int x, y, vx, vy, vz;

//------------ variaveis relacionadas a coisas do display---------
int h = 0;
int etapa_atual = 0;
int pos;
int status = 0;

/*                           *\
|*    Funções do display     *|
\*                           */
void tela_ref_em_anda() {
    tft.setTextColor(GREEN);          // Setagem de cor
    tft.setTextSize(3);               // Setagem de tamanho
    tft.setCursor(3, 125);            // Orientação do texto X,Y
    tft.println("Fazendo homing..."); // texto que será mostrado
}

void tela_ref_finalizado() {
    tft.setTextColor(GREEN);         // Setagem de cor
    tft.setTextSize(3);              // Setagem de tamanho
    tft.setCursor(3, 55);            // Orientação do texto X,Y
    tft.println("Homing concluido"); // texto que será mostrado
    wait(3);
}

// --- tela para apagar ---
void apaga_tela() { tft.fillScreen(BLACK); }

// --- tela para definir coleta ---
void tela_def_coleta() {
    led_amarelo = true;
    tft.setTextColor(GREEN); // Setagem de cor
    tft.setTextSize(4);      // Tamanho do Texto no Display
    tft.setCursor(80, 40);   // Orientação do texto X,Y
    tft.println("Definir");  // texto que será mostrado

    tft.setTextSize(4);    // Tamanho do Texto no Display
    tft.setCursor(80, 90); // Orientação do texto X,Y
    tft.println("Ponto");

    tft.setTextSize(4);       // Tamanho do Texto no Display
    tft.setCursor(80, 140);   // Orientação do texto X,Y
    tft.println("de Coleta"); // texto que será mostrado
}
// --- tela para sair da emergência ---
void sair_emer() {
    bool esperando = true;
    while (esperando) {
        tft.setTextColor(RED);                      // Setagem de cor
        tft.setTextSize(3);                         // Tamanho do Texto no Display
        tft.setCursor(35, 50);                      // Orientação do texto X,Y
        tft.println("SAIR DA TELA");                // texto que será mostrado
        tft.setCursor(110, 80);                     // Orientação do texto X,Y
        tft.println("EMERG.?");                     // texto que será mostrado
        tft.drawRoundRect(20, 40, 280, 70, 1, RED); // Setagem de cor
        tft.setTextSize(1);                         // Tamanho do Texto no Display
        tft.setTextColor(BLUE);                     // Setagem de cor
        tft.setCursor(110, 180);                    // Orientação do texto X,Y
        tft.println("Press. ENTER para sair");      // Tamanho do Texto no Display
    }
}

// --- Tela para definir solta ---
void tela_def_solta() {
    tft.setTextColor(GREEN);  // Setagem de cor
    tft.setTextSize(4);       // Tamanho do Texto no Display
    tft.setCursor(70, 40);    // Orientação do texto X,Y
    tft.println("Defina um"); // texto que será mostrado
    tft.setTextSize(4);       // Tamanho do Texto no Display
    tft.setCursor(70, 90);    // Orientação do texto X,Y
    tft.println("ponto");     // texto que será mostrado
    tft.setTextSize(4);       // Tamanho do Texto no Display
    tft.setCursor(70, 140);   // Orientação do texto X,Y
    tft.println("de solta");  // texto que será mostrado
}

// --- Tela para perguntar se deseja sair da emergência ---
void tela_pergun_saida_emer() {
    bool esperando = true;
    while (esperando) {
        tft.setTextColor(RED);                      // Setagem de cor
        tft.setTextSize(3);                         // Tamanho do Texto no Display
        tft.setCursor(35, 50);                      // Orientação do texto X,Y
        tft.println("SAIR DA TELA");                // texto que será mostrado
        tft.setCursor(110, 80);                     // Orientação do texto X,Y
        tft.println("EMERG.?");                     // texto que será mostrado
        tft.drawRoundRect(20, 40, 280, 70, 1, RED); // Setagem de cor
        tft.setTextSize(2);                         // Tamanho do Texto no Display
        tft.setTextColor(BLUE);                     // Setagem de cor
        tft.setCursor(35, 180);                     // Orientação do texto X,Y
        tft.println("Press. ENTER para sair");      // texto que será mostrado
        bool estado_enter = enter;                  // DEBOUNCE
        wait_ms(50);                                // DEBOUNCE
        bool enter_deb = enter && estado_enter;     // DEBOUNCE

        if (!estado_enter) {
            esperando = false;
        }
    }
}

// --- Tela dos recipientes ---
void tela_recipientes() {
    tft.setTextColor(GREEN);
    tft.setTextSize(15);
    tft.setCursor(45, 115);
    tft.println("-");

    tft.setTextColor(GREEN);
    tft.setTextSize(15);
    tft.setCursor(45, 5);
    tft.println("+");

    tft.setTextColor(GREEN);
    tft.setTextSize(2);
    tft.setCursor(160, 50);
    tft.println("Num. Recipi.");
    tft.setCursor(160, 80);

    tft.setTextColor(GREEN);
    tft.setTextSize(2);
    tft.setCursor(160, 80);
    tft.fillRoundRect(205, 80, 154, 50, 5, BLACK);
    tft.printf("NUM = %d ", num_pontos_solta);
}

// --- [TOUCH] para tela de terminar o numero de recipientes ---
void funcao_touch_det_num_recip(void) {
    bool definindo = true; // Variavel para ver se esta definindo o numero de recipiente
    tft.setTextSize(2);
    tft.setTextColor(MAGENTA, BLUE);
    num_pontos_solta = 1;
    while (definindo) {
        tp = ts.getPoint();
        YPout.output();
        XMout.output();
        if (tp.z < MINPRESSURE && tp.z > MAXPRESSURE) {
            tp.x = tft.width() - (map(tp.x, TS_RT, TS_LEFT, tft.width(), 0));
            tp.y = tft.height() - (map(tp.y, TS_BOT, TS_TOP, tft.height(), 0));
        }
        if (tp.x >= 620 && tp.x <= 805 && tp.y >= 160) {
            if (mais == 0) {
                num_pontos_solta = num_pontos_solta++;
                if (num_pontos_solta > 9) {
                    num_pontos_solta = 9;
                }
                tft.setTextColor(GREEN);
                tft.setTextSize(2);
                tft.setCursor(160, 80);
                tft.fillRoundRect(205, 80, 154, 50, 5, BLACK);
                tft.printf("NUM = %d ", num_pontos_solta);
                wait(0.5);
            }
        }

        if (tp.x >= 130 && tp.x <= 500 && tp.y >= 120 && tp.y <= 400) {
            if (menos == 0) {

                num_pontos_solta = num_pontos_solta--;
                if (num_pontos_solta < 0) {
                    num_pontos_solta = 0;
                }

                tft.setTextColor(GREEN);
                tft.setTextSize(2);
                tft.setCursor(160, 80);
                tft.fillRoundRect(205, 80, 154, 50, 5, BLACK);
                tft.printf("NUM = %d ", num_pontos_solta);
                wait(0.5);
            }
        }
        bool estado_enter = enter;
        wait_ms(50);
        bool enter_deb = enter && estado_enter;
        if (!estado_enter) {
            definindo = false;
        }
    }
}

void tela_ref_det_vol(void) {
    int status = 0;
    tft.setTextColor(GREEN);
    tft.setTextSize(15);
    tft.setCursor(45, 115);
    tft.println("-");
    tft.setTextColor(GREEN);
    tft.setTextSize(15);
    tft.setCursor(45, 5);
    tft.println("+");
    tft.setTextColor(GREEN);
    tft.setTextSize(2);
    tft.setCursor(160, 50);
    tft.println("Volume (ml)");
    tft.setCursor(160, 80);
    tft.setTextColor(GREEN);
    tft.setTextSize(2);
    tft.setCursor(160, 80);
    tft.fillRoundRect(205, 80, 154, 50, 5, BLACK);
    tft.printf("ml(s) = %d ", vol);
    tft.setTextSize(2);
    tft.setTextColor(MAGENTA, BLUE);
    while (status == 0) {
        tp = ts.getPoint();
        YPout.output();
        XMout.output();
        if (tp.z < MINPRESSURE && tp.z > MAXPRESSURE) {
            tp.x = tft.width() - (map(tp.x, TS_RT, TS_LEFT, tft.width(), 0));
            tp.y = tft.height() - (map(tp.y, TS_BOT, TS_TOP, tft.height(), 0));
        }

        if (tp.x >= 620 && tp.x <= 805 && tp.y >= 160) {
            if (mais == 0) {
                vol = vol++;
                tft.setTextColor(GREEN);
                tft.setTextSize(2);
                tft.setCursor(160, 80);
                tft.fillRoundRect(205, 80, 154, 50, 5, BLACK);
                tft.printf("ML = %d ", vol);
                wait(0.5);
            }
        }

        if (tp.x >= 130 && tp.x <= 500 && tp.y >= 120 && tp.y <= 400) {
            if (menos == 0) {
                vol = vol--;
                if (vol <= 0) {
                    vol = 0;
                }
                tft.setTextColor(GREEN);
                tft.setTextSize(2);
                tft.setCursor(160, 80);
                tft.fillRoundRect(205, 80, 154, 50, 5, BLACK);
                tft.printf("ML = %d ", vol);
                wait(0.5);
            }
        }
        bool estado_enter = enter;
        wait_ms(50);
        bool enter_deb = enter && estado_enter;
        if (!enter_deb) {
            status = 1;
        }
    }
}

void welcome() {
    led_amarelo = false;
    led_vermelho = false;
    tft.setTextColor(BLUE);
    tft.setTextSize(5);    // Tamanho do Texto no Display
    tft.setCursor(20, 40); // Orientação do texto X,Y
    tft.println("Welcome");
    wait(1);
    tft.setTextColor(GREEN);
    tft.setTextSize(3);      // Tamanho do Texto no Display
    tft.setCursor(130, 100); // Orientação do texto X,Y
    tft.println("To");
    wait(1);
    tft.setTextColor(RED);
    tft.setTextSize(5);      // Tamanho do Texto no Display
    tft.setCursor(120, 140); // Orientação do texto X,Y
    tft.println("PipFox");
    wait(0.5);
}

void tela_iniciar_rotina() {
    bool esperando = true;
    while (esperando) {
        led_amarelo = true;
        tft.setTextColor(GREEN);
        tft.setTextSize(4);    // Tamanho do Texto no Display
        tft.setCursor(50, 70); // Orientação do texto X,Y
        tft.println("Realizar");
        tft.setTextSize(4);
        tft.setCursor(50, 120); // Orientação do texto X,Y
        tft.println("Rotina?");
        bool estado_enter = enter;
        wait_ms(50);
        bool enter_deb = enter && estado_enter;
        if (!enter_deb) {
            esperando = false;
        }
        led_amarelo = false;
    }
}

void tela_realizando_rotina() {
    led_verde = true;
    tft.setTextColor(GREEN);
    tft.setTextSize(4);    // Tamanho do Texto no Display
    tft.setCursor(50, 70); // Orientação do texto X,Y
    tft.println("Realizando");

    tft.setTextSize(4);
    tft.setCursor(50, 120); // Orientação do texto X,Y
    tft.println("Rotina!");
}

void tela_emergencia() {
    led_vermelho = true;
    led_verde = false;
    tft.fillScreen(BLACK); // Fundo do Display
    tft.fillRect(15, 55, 300, 100, RED);
    tft.setTextColor(YELLOW);
    tft.setCursor(25, 120); // Orientação X,Y
    tft.setTextSize(4);
    tft.println("EMERGENCIA");
    tft.setCursor(25, 160);
    tft.setTextSize(2);
    tft.print("Esperando comando");
}

void tela_sair_emergencia() {
    led_vermelho = true;
    tft.fillScreen(BLACK); // Fundo do Display
    tft.fillRect(15, 55, 300, 100, GREEN);
    tft.setTextColor(YELLOW);
    tft.setCursor(25, 100); // Orientação X,Y
    tft.setTextSize(4);
    tft.println("Saindo");
    tft.setCursor(25, 160);
    tft.setTextSize(2);
    tft.print("EMERGENCIA");
    wait(1);
}

/*                                     *\
|*    Funções do gerais e Estrutura     *|
\*                                     */
// ---------- MOTORES ACIONAMENTO ----------
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
// ---------- MOTORES DESLIGAR ----------
void desliga_motor(BusOut &motor) { motor = 0, 0, 0, 0; }
/*                             Estrutura PONTOSOLTA                                 *\
    essa estrutura se consiste em fazer o vetor para os pontos de solta, já que
    juntaremos com uma lista feita no controlador com as coordenas, v desejado, vatual
\*                             Estrutura PONTOSOLTA                                  */
struct PontoSolta {
    int coord[3];
    int volume_desejado;
    int volume_atual;
};

/*                              Estrutura Controlador                                   *\
    Ela terá variaveis que irão continuamente serem atualizadas durante
    todo o processo de pipetagem, para isso todas as funções que envolvem os parametros
    irão ficar  dentro dessa estrutrua e será chamada na rotina principal
\*                              Estrutura Controlador                                   */
struct Controlador {
    bool ref_feito[3];
    bool enable; // 0 -> emergencia; -> 1 funcionamento normal;
    bool determinar_ponto;
    // int qual_tela;
    bool det_vol;
    bool pontos_finalizados;
    bool inicializacao;
    bool coleta_feita;
    bool processo_iniciado;
    bool processo_concluido;
    bool primeira_solta;
    bool estado_ref;
    volatile bool emergencia;
    long soltas;
    int numero_pontos_solta; // nao sei se é a mesma coisa que a variavel de cima
    int max_coord[3];
    int min_coord[3];
    // ------ arrays ------
    PontoSolta solta[9]; // vetor
    int coleta[3];
    int atual[3];
    int distancia_coleta_atual[3];
    int distancia_solta_coleta[3];
    int step[3];
    float step_rev[3]; // passo/rev motor x,y,z
    int passo[3];      // passo x, y, z (FUSO)
    int tempo;
    int destino[3];

    // --------------------- Rotina emergencia, display ---------------------------
    void variavel_default() {
        pc.printf("\rzerando valores\n");
        // qual_tela = 0; //0 -> welcome; 1->referenciando; 2 -> ponto de solta; 3-> ponto de coleta
        passo[0] = 3;
        passo[1] = 3;
        passo[2] = 10;
        primeira_solta = true;
        inicializacao = true;
        enable = true; // 0 -> emergencia; -> 1 funcionamento normal;
        emergencia = false;
        soltas = 0;
        tempo = 3;
        det_vol = false;
        pontos_finalizados = false;
        coleta_feita = false;
        processo_concluido = false;
        numero_pontos_solta = 0; // numero de pontos de solta começa não definido
        determinar_ponto = true; // 1 -> ponto de coleta; 0 -> ponto de solta
        processo_iniciado = false;
        estado_ref = false;
        for (int i = 0; i < 3; i++) {
            ref_feito[i] = false;
            coleta[i] = 0;
            atual[i] = 0;
            distancia_coleta_atual[i] = 0;
            distancia_solta_coleta[i] = 0;
            step[i] = 0;
            step_rev[i] = 512;
        }
    }
    void emerg() {
        pc.printf("emergencia \r\n");
        for (int i = 0; i < 3; i++) {
            step[i] = 0;
            ref_feito[i] = false;
        }
        enable = false;
        emergencia = true;
        processo_concluido = false;
        apaga_tela();
        tela_emergencia();
    }

    void tela_comecar_ref() {
        bool esperando = true;
        led_amarelo = false;
        led_vermelho = false;
        tft.setTextColor(GREEN);
        tft.setTextSize(3);
        tft.setCursor(25, 80);
        tft.println("Iniciar homing");
        while (esperando && enable) {
            if (emergencia) return;
            bool estado_enter = enter;
            wait_ms(50);
            bool enter_deb = enter && estado_enter;
            if (!enter_deb) {
                esperando = false;
            }
        }
    }

    void sair_emerg() {
        apaga_tela();
        tela_sair_emergencia();
        pc.printf("sair emergencia \r\n");
        inicializacao = true;
        enable = true; // 0 -> emergencia; -> 1 funcionamento normal;
        emergencia = false;
        soltas = 0;
        tempo = 3;
        det_vol = false;
        referenciando = false;
        pontos_finalizados = false;
        coleta_feita = false;
        processo_concluido = false;
        numero_pontos_solta = 0; // numero de pontos de solta começa não definido
        determinar_ponto = true; // 1 -> ponto de coleta; 0 -> ponto de solta
        primeira_solta = true;
        estado_ref = false;
        for (int i = 0; i < 3; i++) {
            ref_feito[i] = false;
            coleta[i] = 0;
            atual[i] = 0;
            distancia_coleta_atual[i] = 0;
            distancia_solta_coleta[i] = 0;
            step[i] = 0;
            step_rev[i] = 512;
            solta[i].coord[i] = 0;
            solta[i].volume_desejado = 0;
            solta[i].volume_atual = 0;
        }
        tft.reset();
        tft.begin();
        tft.setRotation(Orientation);
        tft.fillScreen(BLACK);
        apaga_tela();
    }

    // --- Parte da rotina de emergencia ---- Referente aos endstops ------
    void chave_fim_curso() {
        if (!endstops && ref_feito[0] && ref_feito[1] && ref_feito[2]) {
            emerg();
            apaga_tela();
            tela_pergun_saida_emer();
            sair_emerg();
        }
    }

    void tela_mostrar_ponto_coleta_def(float x, float y, float z) {
        float x_cm = x * passo[0] / (step_rev[0]);
        float y_cm = y * passo[1] / (step_rev[1]);
        float z_cm = z * passo[2] / (step_rev[2]);
        tft.setTextColor(BLUE);
        tft.setTextSize(3);   // Tamanho do Texto no Display
        tft.setCursor(3, 10); // Orientação do texto X,Y
        tft.println("Ponto de coleta");
        tft.setTextColor(WHITE);
        tft.setTextSize(3); // Tamanho do Texto no Display
        tft.setCursor(10, 70);
        tft.println("Pos X =");
        tft.setCursor(160, 70); // tava 240 antes
        tft.printf("%.2f cm", x_cm);
        tft.setCursor(10, 120);
        tft.println("Pos Y =");
        tft.setCursor(160, 120);
        tft.printf("%.2f cm", y_cm);
        tft.setCursor(10, 170);
        tft.println("Pos Z =");
        tft.setCursor(160, 170);
        tft.printf("%.2f cm", z_cm);
        pc.printf("%f %f %f", x_cm, y_cm, z_cm);
    }

    void tela_mostrar_ponto_solta_def(float x, float y, float z, int n) {
        float x_cm = x * passo[0] / (step_rev[0]);
        float y_cm = y * passo[1] / (step_rev[1]);
        float z_cm = z * passo[2] / (step_rev[2]);
        // Título da sessão
        tft.setTextColor(BLUE);
        tft.setTextSize(3);   // Tamanho do Texto no Display
        tft.setCursor(3, 10); // Orientação do texto X,Y
        tft.printf("Ponto de solta %d", n);
        tft.setTextColor(WHITE);
        tft.setTextSize(3); // Tamanho do Texto no Display
        tft.setCursor(10, 70);
        tft.println("Pos X =");
        tft.setCursor(160, 70);
        tft.printf("%.2f cm", x_cm);
        tft.setCursor(10, 120);
        tft.println("Pos Y =");
        tft.setCursor(160, 120);
        tft.printf("%.2f cm", y_cm);
        tft.setCursor(10, 170);
        tft.println("Pos Z =");
        tft.setCursor(160, 170);
        tft.printf("%.2f cm", z_cm);
    }

    void eixo_refere() {
        if (referenciando) {
            for (int i = 0; i < 3; i++) {
                bool finding_max = true;
                while (!ref_feito[i] && referenciando) {
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
                            max_coord[i] = step[i] - 64; // subtração dos 8 steps
                            if (emergencia) return;
                            aciona_motor(3, false, motores[i]);
                            aciona_motor(3, false, motores[i]);
                            aciona_motor(3, false, motores[i]);
                            aciona_motor(3, false, motores[i]);
                            aciona_motor(3, false, motores[i]);
                            aciona_motor(3, false, motores[i]);
                            aciona_motor(3, false, motores[i]);
                            aciona_motor(3, false,
                                         motores[i]); // +64 steps para desativar o endstop
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
                            min_coord[i] = step[i] + 64; // adição dos 8 steps
                            if (emergencia) return;
                            aciona_motor(3, true, motores[i]);
                            aciona_motor(3, true, motores[i]);
                            aciona_motor(3, true, motores[i]);
                            aciona_motor(3, true, motores[i]);
                            aciona_motor(3, true, motores[i]);
                            aciona_motor(3, true, motores[i]);
                            aciona_motor(3, true, motores[i]);
                            aciona_motor(3, true, motores[i]);
                            wait(1);
                        }
                    }
                }
            }
            // Depois do referenciamento feito - será zerado e o plano cartesiano apenas terá
            // valores positivos >=0
            for (int i = 0; i < 3; i++) {
                max_coord[i] += abs(min_coord[i]);
                min_coord[i] = 0;
                step[i] = 0;
            }
            pc.printf("referenciamento_concluido \r\n");
            pc.printf("max_coord x:%d y:%d z:%d \r\n", max_coord[0], max_coord[1], max_coord[2]);
            pc.printf("min_coord x:%d y:%d z:%d \r\n", min_coord[0], min_coord[1], min_coord[2]);
            referenciando = false;
        }
    }

    void motor_joystick(int x, int y, DigitalIn &z11, DigitalIn &z22) {
        if (emergencia) return;
        if (!enter) return;
        bool bateu = endstops.read();
        if (enable && bateu) {
            if (x > CXmax && step[0] < max_coord[0]) {
                if (emergencia) return;
                aciona_motor(tempo, true, motores[0]);
                step[0] += 4;
                aciona_motor(tempo, true, motores[0]);
                step[0] += 4;
                aciona_motor(tempo, true, motores[0]);
                step[0] += 4;
                aciona_motor(tempo, true, motores[0]);
                step[0] += 4;
                aciona_motor(tempo, true, motores[0]);
                step[0] += 4;

            } else if (x < CXmin && step[0] > min_coord[0]) {
                if (emergencia) return;
                aciona_motor(tempo, false, motores[0]);
                step[0] -= 4;
                aciona_motor(tempo, false, motores[0]);
                step[0] -= 4;
                aciona_motor(tempo, false, motores[0]);
                step[0] -= 4;
                aciona_motor(tempo, false, motores[0]);
                step[0] -= 4;
                aciona_motor(tempo, false, motores[0]);
                step[0] -= 4;
            } else {
                desliga_motor(motores[0]);
            }
            if (y > CYmax && step[1] < max_coord[1]) {
                if (emergencia) return;
                aciona_motor(tempo, true, motores[1]);
                step[1] += 4;
                aciona_motor(tempo, true, motores[1]);
                step[1] += 4;
                aciona_motor(tempo, true, motores[1]);
                step[1] += 4;
                aciona_motor(tempo, true, motores[1]);
                step[1] += 4;
                aciona_motor(tempo, true, motores[1]);
                step[1] += 4;
            } else if (y < CYmin && step[1] > min_coord[1]) {
                if (emergencia) return;
                aciona_motor(tempo, false, motores[1]);
                step[1] -= 4;
                aciona_motor(tempo, false, motores[1]);
                step[1] -= 4;
                aciona_motor(tempo, false, motores[1]);
                step[1] -= 4;
                aciona_motor(tempo, false, motores[1]);
                step[1] -= 4;
                aciona_motor(tempo, false, motores[1]);
                step[1] -= 4;

            } else {
                desliga_motor(motores[1]);
            }
            bool estado = z11;
            bool estado2 = z22;
            wait_ms(50);
            bool bateu_z1 = z11 && estado;
            bool bateu_z2 = z22 && estado2;
            if (!bateu_z1 && step[2] < max_coord[2]) {
                if (emergencia) return;
                aciona_motor(tempo, true, motores[2]);
                step[2] += 4;
                aciona_motor(tempo, true, motores[2]);
                step[2] += 4;
                aciona_motor(tempo, true, motores[2]);
                step[2] += 4;
                aciona_motor(tempo, true, motores[2]);
                step[2] += 4;
                aciona_motor(tempo, true, motores[2]);
                step[2] += 4;
            } else if (!bateu_z2 && step[2] > min_coord[2]) {
                if (emergencia) return;
                aciona_motor(tempo, false, motores[2]);
                step[2] -= 4;
                aciona_motor(tempo, false, motores[2]);
                step[2] -= 4;
                aciona_motor(tempo, false, motores[2]);
                step[2] -= 4;
                aciona_motor(tempo, false, motores[2]);
                step[2] -= 4;
                aciona_motor(tempo, false, motores[2]);
                step[2] -= 4;
            } else {
                desliga_motor(motores[2]);
            }
        }
    }

    void ponto_coleta() {
        pc.printf("determinando coleta\r\n");
        coleta[0] = step[0];
        coleta[1] = step[1];
        coleta[2] = step[2];
        determinar_ponto = false;
    }

    void ponto_solta(int volume_desejado) {
        if (soltas != numero_pontos_solta) {
            pc.printf("determinando solta\r\n");
            // soltas = numero_pontos_solta;
            solta[soltas].coord[0] = step[0];
            solta[soltas].coord[1] = step[1];
            solta[soltas].coord[2] = step[2];
            solta[soltas].volume_desejado = volume_desejado;
            soltas++;
        }
    }

    void ir_ponto(int destino[3]) {
        // ---- Levantando pipeta no maximo ----
        while (step[2] < max_coord[2] && estado_ref) {
            chave_fim_curso();
            pc.printf("Step x:%d Step y:%d Step z:%d levantando pipeta\r\n", step[0], step[1],
                      step[2]);
            if (emergencia) return;
            aciona_motor(tempo, true, motores[2]);
            step[2] += 4;
        }
        // ---- Arrumando eixo x e y ----
        for (int i = 0; i < 2; i++) {
            // ---- indo com eixo x e y para SAH - Sentido anti horario ----
            while (step[i] > destino[i] && estado_ref) {
                chave_fim_curso();
                pc.printf("Step x:%d Step y:%d Step z:%d subtraindo \r\n", step[0], step[1],
                          step[2]);
                if (emergencia) return;
                aciona_motor(tempo, false, motores[i]);
                step[i] -= 4;
            }
            // ---- indo com eixo x e y para SH - Sentido horario ----
            while (step[i] < destino[i] && estado_ref) {
                chave_fim_curso();
                pc.printf("Step x:%d Step y:%d Step z:%d somando\r\n", step[0], step[1], step[2]);
                if (emergencia) return;
                aciona_motor(tempo, true, motores[i]);
                step[i] += 4;
            }
        }
        // ---- Descendo pipeta para ponto desejado ----
        while (step[2] > destino[2] && estado_ref) {
            chave_fim_curso();
            pc.printf("Step x:%d Step y:%d Step z:%d descendo para ponto desejado\r\n", step[0],
                      step[1], step[2]);
            if (emergencia) return;
            aciona_motor(tempo, false, motores[2]);
            step[2] -= 4;
        }
    }
    // ---- Função para coletar ------
    void coletar() {
        if (!coleta_feita && estado_ref) {
            ir_ponto(coleta);
            if (estado_ref) {
                pipeta = true;
                wait(1);
                pipeta = false;
            }
        }
    }
    // ---- Função para soltar ------
    void soltar() {
        if (primeira_solta) {
            soltas = 0;
            solta[soltas].volume_atual = 0;
            primeira_solta = false;
            pc.printf("primeira solta\n\r");
        }
        pc.printf("volume atual: %d volume desejado: %dponto %d\n\r", solta[soltas].volume_atual,
                  solta[soltas].volume_desejado, soltas);
        ir_ponto(solta[soltas].coord);
        pipeta = true;
        wait(1);
        // soltas = numero_pontos_solta - 1;
        solta[soltas].volume_atual++;
        if (solta[soltas].volume_atual >= solta[soltas].volume_desejado) {
            soltas++;
            pc.printf("soltas:%d\r\n", soltas);
            if (soltas == numero_pontos_solta) {
                processo_concluido = true;
                funcionamento.stop();
            }
        }
        pipeta = false;
    }
    // ---- Tela e configurações para um novo processo ------
    void novo_processo() {
        bool esperando_novo_processo = true;
        while (esperando_novo_processo) {
            tft.setTextColor(BLUE); // SET "TEXTO 1"
            tft.setTextSize(3);
            tft.setCursor(25, 35);
            tft.println("Nova");
            tft.drawRoundRect(20, 20, 280, 80, 1, WHITE);
            tft.setTextColor(BLUE); // SET "TEXTO 2"
            tft.setTextSize(3);
            tft.setCursor(25, 65);
            tft.println("operacao?");
            tft.setTextColor(YELLOW); // SET "TEXTO 3"
            tft.setTextSize(2);
            tft.setCursor(45, 120);
            tft.println("Processo");
            tft.setTextColor(YELLOW); // SET "TEXTO 4"
            tft.setTextSize(2);
            tft.setCursor(45, 155);
            tft.println("finalizado em");
            tft.setTextColor(RED); // SET "TEXTO 5"
            tft.setTextSize(2);
            tft.setCursor(45, 192);
            tft.printf("%.2f segundos", funcionamento.read());
            // ------ Debounce ------
            bool estado_enter = enter;
            wait_ms(50);
            bool enter_deb = enter && estado_enter;
            // ----------------------
            if (!enter_deb) {
                esperando_novo_processo = false;
                inicializacao = true;
                enable = true; // 0 -> emergencia; -> 1 funcionamento normal;
                emergencia = false;
                soltas = 0;
                tempo = 3;
                det_vol = false;
                pontos_finalizados = false;
                coleta_feita = false;
                processo_concluido = false;
                numero_pontos_solta = 0; // numero de pontos de solta começa não definido
                determinar_ponto = true; // 1 -> ponto de coleta; 0 -> ponto de solta
                primeira_solta = true;
                estado_ref = false;
                for (int i = 0; i < 3; i++) {
                    ref_feito[i] = false;
                    coleta[i] = 0;
                    atual[i] = 0;
                    distancia_coleta_atual[i] = 0;
                    distancia_solta_coleta[i] = 0;
                    step[i] = 0;
                    step_rev[i] = 512;
                    solta[i].coord[i] = 0;
                    solta[i].volume_desejado = 0;
                    solta[i].volume_atual = 0;
                }
            }
        }
    }
};

/*                                                *\
|*    Chamando a estrutura - Funções adicionais    *|
\*                                                */
Controlador Controlador1;

void fail_safe() {
    Controlador1.emerg();
    tela_emergencia();
}

void sair_failsafe() {
    Controlador1.sair_emerg();
    Controlador1.variavel_default();
    referenciando = false;
    fail_safe_out = true;
    for (int i = 0; i < 3; i++) {
        Controlador1.ref_feito[i] = false;
        Controlador1.coleta[i] = 0;
        Controlador1.atual[i] = 0;
        Controlador1.distancia_coleta_atual[i] = 0;
        Controlador1.distancia_solta_coleta[i] = 0;
        Controlador1.step[i] = 0;
        Controlador1.step_rev[i] = 512;
        Controlador1.max_coord[i] = 0;
        Controlador1.min_coord[i] = 0;
    }
    Controlador1.inicializacao = true;
    Controlador1.enable = true;
}

void setup() {
    Controlador1.variavel_default(); // função que contem os valores das variaveis do controlador
    bot_emerg.mode(PullUp);          // definição do botão de emergencia como PullUp
    bot_emerg.fall(&fail_safe);
    bot_emerg.rise(&sair_failsafe);
    tft.reset();                  // Display
    tft.begin();                  // Display
    tft.setRotation(Orientation); // Display
    tft.fillScreen(BLACK);        // Display
    delay(200);
    pipeta = false; // Representação da pipeta
}

/*                       *\
|*    Rotina principal   *|
\*                       */

void loop() {
    x = Nunchuck.joyx();
    y = Nunchuck.joyy();
    pc.printf("Step x:%d Step y:%d Step z:%d\r\n", Controlador1.step[0],
              Controlador1.step[1], // loop
              Controlador1.step[2]);
    if (Controlador1.enable && !Controlador1.emergencia) {
        led_verde = true;
        led_vermelho = false;
        // ---------- inicialização da tela com mensagem de bem-vindo ----------
        if (Controlador1.inicializacao) {
            // ---------- RODAR UMA VEZ ----------
            apaga_tela();
            welcome();
            pc.printf("\rteladeboot\n");
            Controlador1.inicializacao = false;
            funcionamento.reset();
            referenciando = false;
            apaga_tela();
            Controlador1.tela_comecar_ref();
            fail_safe_out = false;
        }
        Controlador1.estado_ref =
            Controlador1.ref_feito[0] && Controlador1.ref_feito[1] && Controlador1.ref_feito[2];
        // ------------ Referenciamento - rotina + display ------------
        if (!Controlador1.estado_ref && !Controlador1.emergencia) {
            pc.printf("\rnao esta referenciao\n");
            // mostrar tela para iniciar ref caso nao esteja referenciado
            apaga_tela();
            Controlador1.tela_comecar_ref();
            if (Controlador1.emergencia) return;
            pc.printf("referenciando\r\n");
            apaga_tela();
            tela_ref_em_anda();
            referenciando = true;
            Controlador1.eixo_refere();
            apaga_tela();
            if (!referenciando && !fail_safe_out) {
                tela_ref_finalizado();
                Controlador1.estado_ref = true;
                apaga_tela();
            }
        }
        // --------------- Referenciamento feito ---------------
        if (Controlador1.estado_ref && !Controlador1.emergencia) {
            Controlador1.chave_fim_curso();
            if (Controlador1.emergencia) return;
            wait_ms(5);
            // --------------- Determinando o ponto de coleta ---------------
            if (Controlador1.determinar_ponto && !Controlador1.emergencia) {
                if (Controlador1.emergencia) return;
                bool estado_enter = enter;
                wait_ms(50);
                bool enter_deb = enter && estado_enter; // Debounce
                tela_def_coleta();
                Controlador1.motor_joystick(x, y, z1, z2);
                if (!enter_deb && !Controlador1.emergencia) {
                    if (Controlador1.emergencia) return;
                    led_amarelo = false;
                    apaga_tela();
                    Controlador1.ponto_coleta();
                    funcionamento.start();
                    Controlador1.tela_mostrar_ponto_coleta_def(
                        Controlador1.step[0], Controlador1.step[1], Controlador1.step[2]);
                    wait(3);
                }
            }

            // --------------- Determinado os pontos de solta ---------------
            if (!Controlador1.determinar_ponto && !Controlador1.emergencia) {
                Controlador1.chave_fim_curso();
                if (Controlador1.emergencia) return;
                while (Controlador1.numero_pontos_solta == 0 && !Controlador1.emergencia) {
                    Controlador1.chave_fim_curso();
                    if (Controlador1.emergencia) return;
                    num_pontos_solta = 1;
                    apaga_tela();
                    tela_recipientes();
                    funcao_touch_det_num_recip();
                    wait(1);
                    Controlador1.numero_pontos_solta = num_pontos_solta;
                    pc.printf("\rquantidade pontos de solta definidos: %d\n",
                              Controlador1.numero_pontos_solta);
                }

                int i = 0;
                // Sabendo os numero de pontos de solta que o usuario deseja será guardado as
                // coordenadas e o volume para cada ponto de solta
                while (i < Controlador1.numero_pontos_solta && !Controlador1.pontos_finalizados &&
                       !Controlador1.emergencia) {
                    Controlador1.chave_fim_curso();            // Emergencia chave fim de curso
                    if (Controlador1.emergencia) return;       // Emergencia
                    x = Nunchuck.joyx();                       // leitura do nunchuck
                    y = Nunchuck.joyy();                       // leitura do nunchuck
                    vol = 1;                                   // volume
                    Controlador1.motor_joystick(x, y, z1, z2); // Função do joystick
                    pc.printf("Step x:%d Step y:%d Step z:%d \r\n", Controlador1.step[0],
                              Controlador1.step[1],
                              Controlador1.step[2]);        // Print para saber os steps
                    bool estado_enter = enter;              // DEBOUNCE
                    wait_ms(50);                            // DEBOUNCE
                    bool enter_deb = enter && estado_enter; // DEBOUNCE
                    apaga_tela();
                    tela_def_solta();
                    // - SE O ENTER FOR ACIONADO E O ESTADO DE EMERGENCIA FOR FALSO - SERA
                    // DETERMINADO OS PONTOS DE SOLTA
                    if (!enter_deb && !Controlador1.emergencia) {
                        Controlador1.chave_fim_curso();
                        if (Controlador1.emergencia) return;
                        apaga_tela();
                        Controlador1.tela_mostrar_ponto_solta_def(Controlador1.step[0],
                                                                  Controlador1.step[1],
                                                                  Controlador1.step[2], i + 1);
                        pc.printf("determinando ponto de solta %d\r\n", i + 1);
                        wait(2);
                        apaga_tela();
                        tela_ref_det_vol();
                        wait(2);
                        //  ----- alterar o volume para o selecionado pela tela -----
                        Controlador1.ponto_solta(vol);
                        apaga_tela();

                        pc.printf("item %d coord %d %d %d soltas:%d vol:%d\r\n", i + 1,
                                  Controlador1.solta[i].coord[0], Controlador1.solta[i].coord[1],
                                  Controlador1.solta[i].coord[2], (Controlador1.soltas),
                                  Controlador1.solta[i].volume_desejado);
                        i++;
                        if (i == Controlador1.numero_pontos_solta && !Controlador1.emergencia) {
                            Controlador1.chave_fim_curso();
                            if (Controlador1.emergencia) return;
                            Controlador1.pontos_finalizados = true;
                            tela_iniciar_rotina();

                            apaga_tela();
                            tela_realizando_rotina();
                        }
                    }
                }
            }
            // Com os pontos de solta guardados será feito a coleta - irá ir ao ponto guardado de
            // coleta e coletar
            if (Controlador1.pontos_finalizados && !Controlador1.coleta_feita &&
                Controlador1.soltas >= 0 && !Controlador1.processo_concluido) {
                Controlador1.chave_fim_curso();
                if (Controlador1.emergencia) return;
                pc.printf("coletando\r\n");
                Controlador1.coletar();
                Controlador1.coleta_feita = true;
            }
            // Apos a coleta irá para o ponto de solta e irá ficar indo e voltando ate o volume
            // desejado = volume atual
            if (Controlador1.pontos_finalizados && Controlador1.coleta_feita &&
                !Controlador1.processo_concluido) {
                Controlador1.chave_fim_curso();
                if (Controlador1.emergencia) return;
                pc.printf("coleta_feita\r\n");
                Controlador1.soltar();
                Controlador1.coleta_feita = false;
                pc.printf("Solta feita\r\n");
            }
            // quando o processo concluido for satisfeito
            if (Controlador1.processo_concluido) {
                Controlador1.chave_fim_curso();
                if (Controlador1.emergencia) return;
                // Levantando pipeta no maximo de novo
                while (Controlador1.step[2] < Controlador1.max_coord[2] &&
                       Controlador1.processo_concluido) {
                    Controlador1.chave_fim_curso();
                    if (Controlador1.emergencia) return;
                    pc.printf("Step x:%d Step y:%d Step z:%d subindo pela ultima vez\r\n",
                              Controlador1.step[0], // processo quase concluido
                              Controlador1.step[1], Controlador1.step[2]);
                    aciona_motor(3, true, motores[2]);
                    Controlador1.step[2] += 4;
                }
                // verificando de novo
                if (Controlador1.processo_concluido) {
                    pc.printf("Processo concluido em %.2f segundos\r\n", funcionamento.read());
                    apaga_tela();
                    Controlador1.novo_processo();
                    apaga_tela();
                }
            }
        }
    } else {
        led_verde = false;   // ESTADO DA MAQUINA
        led_vermelho = true; // ESTADO DA MAQUINA - EMERGENCIA ACIONADA
    }
}
