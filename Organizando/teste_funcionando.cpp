//----------------------- incluindo bibliotecas -----------------------
#include "mbed.h"

BusOut motor_x(PC_4, PB_13, PB_14, PB_1);
BusOut motor_y(PB_2, PB_11, PB_12, PA_11);
BusOut motor_z(PA_12, PC_5, PC_6, PC_8);

void aciona_motor(int tempo, bool sentido, bool on_off, BusOut &motor) {
    if (on_off) {
        if (!sentido) {
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
}

void desliga_motor(BusOut &motor) { motor = 0, 0, 0, 0; }

struct abc {
    BusOut motor[3];
    int fodase;
    void roda() { aciona_motor(3, 0, 1, this->motor[0]); }
};
int step[3] = {0, 0, 0}; // step_x, step_y, step_z
//----------------------- variáveis de referenciamento -----------------------
bool ref_x_feito = false;
bool ref_y_feito = false;
bool ref_z_feito = false;
void emerg(int *s, bool ref_x, bool ref_y) {
    s[0] = 0;
    s[1] = 0;
    s[2] = 0;
    !ref_x_feito;
    !ref_y_feito;
    !ref_z_feito;
}

int main() {
    int ligado = 1;
    abc a{{motor_x, motor_y, motor_z}};
    while (1) {
        //        step[0]+=1;
        //        aciona_motor(3,0,ligado,motor_x);
        a.roda();
    }
}
