// ------ Funcoes --------
// -------------- Importando o arquivo de setup --------------
#include "setup.h"

// --------------------- acionamento motor ---------------------------
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

void Controlador::variavel_default() {
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
    enable = true; // 0 -> emergencia; -> 1 funcionamento normal; 2-> fim do processo
    emergencia = false;
    soltas = 0;
    tempo = 3;
}
void Controlador::emerg() {
    for (int i = 0; i < 3; i++) {
        step[i] = 0;
        ref_feito[i] = false;
    }
    enable = false;
    emergencia = true;
}

void Controlador::display() {
    int dist_x = step[0] * passo[0] / step_rev[0];
    int dist_y = step[1] * passo[1] / step_rev[1];
    int dist_z = step[2] * passo[2] / step_rev[2];
    tft.printf("distancia_x=%.2f\ndistancia_y=%.2f\ndistancia_z=%.f\npassos_x=%."
               "0f\npassos_y=%.0f",
               dist_x, dist_y, dist_z, step[0], step[1]);
    tft.printf("\npassos_z=%.0f", step[2]);
}

void Controlador::eixo_refere() {
    BusOut motores[3] = {motor_x, motor_y, motor_z};
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
                    max_coord[i] = step[i];
                    if (emergencia) return;
                    aciona_motor(10, true, motores[i]);
                }
            } else {
                if (emergencia) return;
                aciona_motor(tempo, false, motores[i]);
                wait_ms(3);
                bool bateu = first_read && endstops.read();
                if (bateu) {
                    ref_feito[i] = true;
                    min_coord[i] = step[i];
                    if (emergencia) return;
                    aciona_motor(10, false, motores[i]);
                }
            }
        }
    }
}
void Controlador::motor_joystick(int x, int y, bool z1, bool z2) {
    BusOut motores[3] = {motor_x, motor_y, motor_z};
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

void Controlador::ponto_coleta() {
    pc.printf("determinando coleta\r\n");
    coleta[0] = step[0];
    coleta[1] = step[1];
    coleta[2] = step[2];
}

void Controlador::ponto_solta(int volume_desejado) {
    pc.printf("determinando solta\r\n");
    solta[soltas].coord[0] = step[0];
    solta[soltas].coord[1] = step[1];
    solta[soltas].coord[2] = step[2];
    solta[soltas].volume_desejado = volume_desejado;
}

void Controlador::ir_ponto(int destino[3]) {
    BusOut motores[3] = {motor_x, motor_y, motor_z};
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

void Controlador::coletar() {
    ir_ponto(coleta);
    led_azul = true;
}

void Controlador::soltar() {
    ir_ponto(solta[soltas].coord);
    led_azul = false;
    solta[soltas].volume_atual++;
    if (solta[soltas].volume_atual == solta[soltas].volume_desejado) {
        soltas++;
    }
}
