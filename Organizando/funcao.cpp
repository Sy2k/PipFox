// --------------------------- Funcoes ------------------------------------------
// -------------- Importando o arquivo de setup --------------
#include "setup.h"

// --------------------- Display ---------------------------
void print_lcd(int step_x, int step_y, int step_z)
{
        tft.setTextColor(GREEN);
        tft.setTextSize(3);
        tft.setCursor(0,10); //  Orientação do texto X,Y
        int distancia_x = step_x*passo_linear_x/steps_por_revol_x;//calculo da distancia percorrida em x
        int distancia_y = step_y*passo_linear_y/steps_por_revol_y;//calculo da distancia percorrida em y
        int distancia_z = step_z*passo_linear_z/steps_por_revol_z;//calculo da distancia percorrida em z
        tft.printf("distancia_x=%.2f\ndistancia_y=%.2f\ndistancia_z=%.f\npassos_x=%.0f\npassos_y=%.0f", distancia_x, distancia_y, distancia_z,step_x,step_y);
        tft.printf("\npassos_z=%.0f",step_z);
}

// --------------------- Motores ---------------------------
void motor(int tempo, int sentido,int motor){
    if(sentido==0){
        for(int i = 0; i <4;i++){
            motor = 1 << i;       //verificar se o estado do sistema eh 0 ou 1
            wait_ms(tempo);
        }
    }
    else if(sentido==1){
        for(int i= 3;i>-1;i--){
            motor = 1 << i;
            wait_ms(tempo);
        }
    }
}

void Mx_off(){
    motor_x = 0,0,0,0; 
}

void My_off(){
    motor_y = 0,0,0,0;
} 
void Mz_off(){
    motor_z = 0,0,0,0;
} 

// --------------------- ref ---------------------------
void ref(){
    if (movendo_em_x==1)
    {
        pc.printf("\r referenciou em x\n");
        // lcd.printf("\r referenciou em x\n");
        movendo_em_x = 0;
        ref_x_feito=1;
        Mx_off();
        debounce_endstop.start();
    }

    else if(movendo_em_y==1 && debounce_endstop.read_ms()>500)
    {
        pc.printf("\r referenciou em y\n");
        // lcd.printf("\r referenciou em y\n");
        movendo_em_y = 0;
        ref_y_feito=1;
        My_off();
        debounce_endstop.reset();
        debounce_endstop.start();
    }

    else if(movendo_em_z==1 && debounce_endstop.read_ms()>500)
    {
        pc.printf("\r referenciou em z\n");
        // lcd.printf("\r referenciou em z\n");
        movendo_em_z = 0;
        ref_z_feito=1;
        Mz_off();
        debounce_endstop.reset();
        step_x=0;
        step_y=0;
        step_z=0;
    }
}


void be(){
    estado_sis = 0;//entrou em estado de emergencia
    step_x = 0;//pensar se precisa checar antes de zerar o valor 
    step_y = 0;
    step_z = 0;
    ref_x_feito=0;//zerar o referenciamento de todos os eixos
    ref_y_feito=0;
    ref_z_feito=0;
    determinar_ponto=0;
    print_valor_pos=0;
    char solta[3]={0,0,0};
    char coleta[3]={0,0,0};
    debounce_emer.start();//iniciar timer para debounce
    pc.printf("\r estado de emergencia\n");
    // lcd.printf("\r estado de emergencia\n");
    //entrada no estado de emergencia e perdendo o referenciamento com botao de emergencia
}

void sair_emer(){
    if(debounce_emer.read_ms()>15){//CHECAR
        estado_sis=1;
        ref_x_feito=0;//zerar o referenciamento de todos os eixos
        ref_y_feito=0;
        ref_z_feito=0;
        debounce_emer.reset();//resetar timer de reset
        pc.printf("\r saindo do estado de emergência\n");
        // lcd.printf("\r saindo do estado de emergência\n");
    }
}
