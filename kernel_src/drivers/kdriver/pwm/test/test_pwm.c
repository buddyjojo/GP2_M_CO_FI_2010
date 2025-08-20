
#include <fcntl.h>
#include <signal.h>
#include <stropts.h>
#include <sys/ioctl.h>
#include <asm-mips/types.h>

#include "madp_pwm.h"

int main()
{
    unsigned short int index;
    unsigned int period;
    unsigned int duty;
    unsigned int unit_div;
    unsigned int div;
    unsigned int oen;
    unsigned int polarity;
    unsigned int vdben;
    unsigned int reset_en;
    unsigned int dben;

    MAdp_PWM_Init();

    printf("Start to Run PWM Testing Program...\n");
    do
    {
        printf("Enter the PWM number you want to test (Press 9 to quit):");
        scanf("%d",&index);

        if (index == 9)
        {
            printf("Thank you for test PWM, Bye~\n:");
            break;
        }

        printf("Please enter the Oen of PWM[%d] (1=enable,0=disable)...",index);
        scanf("%d",&oen);


        if(oen == 1)
        {
            MAdp_PWM_Oen(index, oen);

            printf("Please enter the Unit Div of all PWM (integer number)...");
            scanf("%d",&unit_div);
            MAdp_PWM_Unit_Div(unit_div);

            printf("Please enter the Period of PWM[%d] (integer number)...",index);
            scanf("%d",&period);
            MAdp_PWM_Period(index, period);

            printf("Please enter the DutyCycle of PWM[%d] (integer number)...",index);
            scanf("%d",&duty);
            MAdp_PWM_DutyCycle(index, duty);


            printf("Please enter the Div of PWM[%d] (integer number)...",index);
            scanf("%d",&div);
            MAdp_PWM_Div(index, div);

            printf("Please enter the Polarity of PWM[%d] (1=enable,0=disable)...",index);
            scanf("%d",&polarity);
            MAdp_PWM_Polarity(index, polarity);

            printf("Please enter the Vsync double enable of PWM[%d] (1=enable,0=disable)...",index);
            scanf("%d",&vdben);
            MAdp_PWM_Vdben(index, vdben);

            printf("Please enter the Vsync reset enable of PWM[%d] (1=enable,0=disable)...",index);
            scanf("%d",&reset_en);
            MAdp_PWM_Reset_En(index, reset_en);

            printf("Please enter the double enable of PWM[%d] (1=enable,0=disable)...",index);
            scanf("%d",&dben);
            MAdp_PWM_Dben(index, dben);
        }

    } while(index != 9);

}




