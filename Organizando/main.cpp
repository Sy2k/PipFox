#include "setup.h"

// Controlador control{};
// control.emerg();
void setup(void) {
    tft.reset();
    tft.begin();
    tft.setRotation(Orientation);
    tft.fillScreen(BLACK);             // Fundo do Display
    print_lcd(step_x, step_y, step_z); // PRA MIM ISSO NAO FAZ SENTIDO ESTAR AQUI DENTRO
    delay(1000);                       // tava 1000 antes, ACHEI DEMAIS

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
    botao_emergencia.mode(
        PullUp);   // definição do botão de emergencia como PullUp -> ISSO DAQUI TA ESTRANHO
    pc.baud(9600); // definição do baud rate da comunicação serial usb
    botao_emergencia.fall(&be);
    botao_emergencia.rise(&sair_emer);
    endstops.fall(&ref);
}

void loop(void) {
    x = Ax.read_u16(); // ou Ax.read*1000()
    y = Ay.read_u16(); // ou Ay.read*1000()
    switch (estado_sis) { case 1: }
}