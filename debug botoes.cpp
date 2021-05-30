#include "mbed.h"


Timer debounce_emer;


Serial pc(USBTX,USBRX);
InterruptIn botao_emergencia(PC_13);

void be(){
    debounce_emer.start();
    pc.printf("\r estado de emergencia\n");
    //entrada no estado de emergencia e perdendo o referenciamento com botao de emergencia
}

InterruptIn endstop_z(PA_15); //pc14

void sair_emer(){
    // if(debounce_emer.read_ms()>15){
        // debounce_emer.reset();
        pc.printf("\r saindo do estado de emergencia\n");
    // }
}

void Mx_ref(){
        pc.printf("\r ref x feito\n");
}
void My_ref(){
        pc.printf("\r ref y feito\n");
} 
void Mz_ref(){
        pc.printf("\r ref z feito\n");
} 

int main(){    
    botao_emergencia.mode(PullUp);
    pc.baud(9600);
    botao_emergencia.fall(&be);
    botao_emergencia.rise(&sair_emer);
    endstop_z.fall(&Mz_ref);
    // endstop_y.fall(&My_ref);
    // endstop_x.fall(&Mx_ref);
}