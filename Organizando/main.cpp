#include "setup.h"

Controlador control{};
// control.emerg();
void emergencia() { control.emerg(); }
void setup(void) {
    BusOut motores[3] = {motor_x, motor_y, motor_z};
    // SETUP - Display
    tft.reset();
    tft.begin();
    tft.setRotation(Orientation);
    tft.fillScreen(BLACK);
    tft.setTextColor(GREEN);
    tft.setTextSize(3);
    tft.setCursor(0, 10); //  Orientação do texto X,Y
    // Motores desligados
    desliga_motor(motores[0]);
    desliga_motor(motores[1]);
    desliga_motor(motores[2]);
    // Valores
    vx = 3;
    vy = 3;
    vz = 3;
    // Botão de emergências
    bot_emerg.mode(PullUp);
    bot_emerg.fall(&emergencia);
    // Baud rate - comunicacao serial
    pc.baud(9600);
}

void loop(void) {
    x = Ax.read_u16(); // ou Ax.read*1000()
    y = Ay.read_u16(); // ou Ay.read*1000()
    if (control.enable) {
        control.eixo_refere();
    }
}