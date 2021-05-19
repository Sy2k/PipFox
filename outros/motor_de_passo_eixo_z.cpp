#include "mbed.h"
#define velocidade 0.002

BusOut MP(D3,D4,D5,D6);
DigitalIn botao1(PA_7);
DigitalIn botao2(PA_6);

int main()
{
    botao1.mode(PullUp);
    botao2.mode(PullUp);
    while(1)
    {
        if (botao1==0)
        {
            //Deslocamento LSB >MSB
            for (int i=0;i<4;i++)
            {
                MP = 1<<i;
                wait(velocidade);
            }
        }
        if (botao1==1)
        {
            MP=0;
        }
        if (botao2==0)
        {
            //Deslocamento LSB>MSB
            for (int i=3;i>-1;i--)
            {
                MP=1<<i;
                wait(velocidade);
            }
        }
        if (botao2==1)
        {
            MP=0;
        }
    }
}
            

