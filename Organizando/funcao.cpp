// --------------------------- Funcoes ------------------------------------------

void print_lcd(int step_x, int step_y, int step_z)
{
        lcd.cls();//limpar a tela
    //  float distancia_x = step_x*passo_linear_x/steps_por_revol_x;//calculo da distancia percorrida em x
    //  float distancia_y = step_y*passo_linear_y/steps_por_revol_y;//calculo da distancia percorrida em y
    //  float distancia_z = step_z*passo_linear_z/steps_por_revol_z;//calculo da distancia percorrida em z
    //  lcd.printf("dx=%.0f dy=%.0f\ndz=%.0f", distancia_x, distancia_y, distancia_z);
    //  wait(1);//COLOCAR NA MESMA POSICAO
        lcd.cls();
        lcd.printf("Px=%2d Py=%2d\nPz=%2d", step_x, step_y, step_z);
    //  wait(1);
}

void motor_x_sentido_1(int tempo){
    for(int i = 0; i <4;i++){
        motor_x = 1 << i;       //verificar se o estado do sistema eh 0 ou 1
        wait_ms(tempo);
    }
}

void motor_x_sentido_2(int tempo){
    for(int i= 3;i>-1;i--){
        motor_x = 1 << i;
        wait_ms(tempo);
    }
}
    
void motor_y_sentido_1(int tempo){
    for(int i = 0; i <4;i++){
        motor_y = 1 << i;
        wait_ms(tempo);
    }
}


void motor_y_sentido_2(int tempo){
    for(int i= 3;i>-1;i--){
        motor_y = 1 << i;
        wait_ms(tempo);
    }
}
    
void motor_z_sentido_1(int tempo){
    for(int z = 0; z <4;z++){
        motor_z = 1 << z;
        wait_ms(tempo);
    }
}

void motor_z_sentido_2(int tempo){
    for(int z = 3; z > -1;z--){
        motor_z = 1 << z;
        wait_ms(tempo);
    }
}

void Mx_off(){
    motor_x=0,0,0,0;//mao desligar o motor - somente cortar a alimentacao 
}
void My_off(){
    motor_y=0,0,0,0;
} 
void Mz_off(){
    motor_z=0,0,0,0;
} 

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
// void endstop_crash(){
//     estado_sis=0;
//     ref_x_feito=0;
//     ref_y_feito=0;
//     ref_z_feito=0;
//     pc.printf("\r estado de emergencia devido a batida \n");
// }