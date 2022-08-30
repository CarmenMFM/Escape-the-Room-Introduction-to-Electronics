// Carmen Maria Fuentes Mendez 

// 27/07/2022

#include "msp430g2533.h"
#include "FrecuenciaNotas.h"
#include <stdint.h>

#define EVENTO_Sx_PULSAR            BIT0
#define EVENTO_Sx_SOLTAR            BIT1
#define EVENTO_TIEMPO               BIT3


#define SET(ev) (eventos |= (ev))
#define RESET(ev)   (eventos &= ~(ev))
#define TEST(ev)    (eventos & (ev))

const int ArrayFrec[26] = {_N__la,_N__la,_N__la,_N__fa,_N__doH,_N__la,_N__fa,_N__doH,_N__la,0,
                                _N__miH,_N__miH,_N__miH,_N__faH,_N__doH,_N__solb,_N__fa,_N__doH,_N__la,0};
unsigned int i = 0,j= 0;
int32_t division;
const uint32_t DelayNotas[26] = {68750, 68750, 68750, 48125, 20625, 68750, 48125, 20625, 89375,
                                 68750, 68750, 68750, 68750, 48125, 18750, 68750, 48125,
                                 20625, 89375, 68750 };
uint8_t Repeticion = 0;
volatile unsigned char eventos;
volatile unsigned char pulsador,nuevo_pulsador;
volatile char *Contrasena;
volatile unsigned char nota_anterior,nota_tocada, nota_cancion, cont_nota,cont_tiempo;
volatile char cont_fallo, cont_la, cont_d, cont_s, fin,subir_escala, sonar, cancion,cont_D,cont_l,cont_z, fin2;

void reiniciar(void);
void sonar_cancion(void);

//static unsigned char Contraseña[11];

void main(void) {
volatile    unsigned char estado, notas;

    WDTCTL = WDTPW + WDTHOLD;
    DCOCTL = 0;
    BCSCTL1 = CALBC1_1MHZ; // Frecuencia de reloj
    DCOCTL = CALDCO_1MHZ;

    P1DIR &= ~(BIT4+BIT3); // P1.3 Pulsador1; P1.4 Pulsador3
    P1REN |= (BIT4+BIT3);
    P1OUT |= (BIT4+BIT3);
    P1IES |= (BIT4+BIT3);
    P1IFG &= ~(BIT4+BIT3);
    P1IE |= (BIT4+BIT3);

    P1DIR |= (BIT6+BIT0);
    P1OUT &= ~(BIT6+BIT0);

    P2DIR &= ~(BIT3+BIT2+BIT1); //P2.1 Pulsador4; P2.2 Pulsador5; P2.3 Pulsador6 // 0 - Se configura como entrada
    P2REN |= (BIT3+BIT2+BIT1);  // Si hay resistencias o no (1- Si hay)
    P2OUT |= (BIT3+BIT2+BIT1);  // 1 - pullup
    P2IES |= (BIT3+BIT2+BIT1);  // 1 - Flanco de bajada
    P2IFG &= ~(BIT3+BIT2+BIT1); // 1 - Flag
    P2IE |= (BIT3+BIT2+BIT1);   // 1 - Activa interrupciones

    P2DIR |= (BIT7+BIT6+BIT5+BIT4);  // LEDs de los pulsadores
    P2OUT &= ~(BIT7+BIT6+BIT5+BIT4); // 0 - pull down
    P2SEL &= ~(BIT7+BIT6);           // 0 - Desactiva la funcionalidad para la cual esta definida

    P2DIR |= (BIT0); // Altavoz
    P2SEL |= (BIT0); //0 - Desactiva la funcionalidad para la cual esta definida

    TA1CTL = TASSEL_2 ; //  Timer1 a la frecuencia de SMCLK
    TA1CCTL0 = 0;

    TA0CTL = TASSEL_2 + ID_3 + MC_2; //Timer0 A la frecuencia de SMCLK Divide entre 8 en modo continuo

    P1SEL |= (BIT2+BIT1);
    P1SEL2 |= (BIT2+BIT1);
    int32_t Delay = 0;

    WDTCTL = WDT_MDLY_32; // Rebotes 32ms
    IFG1 &= ~(WDTIFG);
    while (IFG1 & WDTIFG == 0);
    WDTCTL = WDTPW + WDTHOLD;
    IFG1 &= ~(WDTIFG);

    nota_cancion = 'l';
    nota_tocada = 'x';
    nota_anterior = 'x';
    sonar = 0;
    cancion = 0;
    fin = 0;
    eventos = 0;
    cont_la = 0;
    cont_d = 0;
    cont_s = 0;
    cont_nota = 0;
    while (1) {
        __low_power_mode_0();

        while (eventos) {

            if (TEST(EVENTO_Sx_PULSAR)) {
                RESET(EVENTO_Sx_PULSAR);

                switch (pulsador) {
                    case (3): {
                        nota_tocada = 'd';
                        P2OUT |= (BIT4); // Enciende led

                        TA1CTL |= (MC_1 + TACLR);// Timer 1 entra en modo cuenta hasta que dejo de pulsar y lo limpia
                        TA1CCTL0 = OUTMOD_4;
                        nuevo_pulsador = 3;
                        break;
                    } // case (3)

                    case (4): {
                        nota_tocada = 'l';
                        P2OUT |= (BIT5);

                        TA1CTL |= (MC_1 + TACLR);// Timer 1 entra en modo cuenta hasta que dejo de pulsar y lo limpia
                        TA1CCTL0 = OUTMOD_4;
                        nuevo_pulsador = 4;
                        break;
                    } // case (4)

                    case (5): {
                        nota_tocada = 'r';
                        P2OUT |= (BIT6);

                        TA1CTL |= (MC_1 + TACLR);
                        TA1CCTL0 = OUTMOD_4;
                        nuevo_pulsador = 5;
                        break;
                    } // case (5)

                    case (6): {
                        nota_tocada = 'f';
                        P2OUT |= (BIT7);

                         TA1CTL |= (MC_1 + TACLR);// Timer 1 entra en modo cuenta hasta que dejo de pulsar y lo limpia
                         TA1CCTL0 = OUTMOD_4;
                         nuevo_pulsador = 6;
                        break;
                    } // case (6)

                } // switch (pulsador)

            } // if (TEST(EVENTO_Sx_PULSAR))

            if (TEST(EVENTO_Sx_SOLTAR)) {
                RESET(EVENTO_Sx_SOLTAR);

                switch (pulsador) {
                    case (3): {
                        P2OUT &= ~(BIT4); // Apaga el led
                        TA1CTL &= ~(MC_1); // Desactiva el modo cuenta
                        TA1CCTL0 = 0; // Timer1 se pone a 0
                        break;
                    } // case (3)
                    case (4): {
                        P2OUT &= ~(BIT5);
                        TA1CTL &= ~(MC_1);
                        TA1CCTL0 = 0;
                        break;
                    } // case (4)
                    case (5): {
                        P2OUT &= ~(BIT6);
                        TA1CTL &= ~(MC_1);
                        TA1CCTL0 = 0;
                        break;
                    } // case (5)
                    case (6): {
                        P2OUT &= ~(BIT7);
                        TA1CTL &= ~(MC_1);
                        TA1CCTL0 = 0;
                        break;
                    } // case (6)

                } // switch (pulsador)

            } // if (TEST(EVENTO_Sx_SOLTAR))
            if (TEST(EVENTO_TIEMPO)) {
                RESET(EVENTO_TIEMPO);
                    TA1CTL &= ~(MC_1);
                    TA1CCTL0 = 0;

            } // if (TEST(EVENTO_TIEMPO))

        } // while (eventos)

        switch (sonar) {
             case(0):
                switch (nuevo_pulsador) {
                    case(3):
                        if(nota_tocada == nota_cancion){
                               TA1CCR0 = 1912-1; // Suena la nota
                               if(nota_anterior == 'f' && cont_d == 1)
                               {
                                   nota_cancion = 'l';
                                   nota_anterior = nota_tocada;
                                   cont_s = 1;
                               }
                               else if(nota_anterior == 'f' && cont_d == 2)
                               {
                                   nota_cancion = 'l';
                                   nota_anterior = nota_tocada;
                                   cont_s = 2;
                               }
                               else{
                                   TA1CCR0 = 7000;
                                   reiniciar();
                               }
                        }
                        else {
                            TA1CCR0 = 7000;
                            reiniciar();
                        }
                    break;
                    case(4):
                        if(nota_tocada == nota_cancion){
                            TA1CCR0 = 2273-1;
                            if(nota_anterior == 'x') // Estamos empezando la cancion
                            {
                                nota_cancion = 'l'; //La siguiente nota que esperamos es un mi
                                nota_anterior = nota_tocada;
                            }
                            else if(nota_anterior == 'l' && cont_la == 0 )
                            {
                                nota_cancion = 'l'; //La siguiente nota que esperamos es un fa
                                nota_anterior = nota_tocada;
                                cont_la = 1;
                            }
                            else if(nota_anterior == 'l' && cont_la == 1)
                            {
                                nota_cancion = 'f';
                                nota_anterior = nota_tocada;
                                cont_la = 2;
                            }
                            else if(nota_anterior == 'd' && cont_s == 1)
                            {
                                nota_cancion = 'f';
                                nota_anterior = nota_tocada;
                                cont_la = 3;
                            }
                            else if(nota_anterior == 'd' && cont_s == 2)
                            {
                                nota_cancion = 'z';

                                nota_anterior = nota_tocada;
                                P2OUT |= (BIT7+BIT6+BIT5+BIT4);
                                TA0CCR2 = TA0R + 34375;
                                TA0CCTL2 &= ~(CCIFG);
                                TA0CCTL2 |= (CCIE);
                                fin = 1;
                                cont_la = 0;
                                cont_s = 0;
                                cont_d = 0;
                            }
                            else{
                                TA1CCR0 = 7000;
                                reiniciar();
                            }
                        }
                        else{
                            TA1CCR0 = 7000;
                            reiniciar();
                        }
                    break;
                    case(5):
                        TA1CCR0 = 7000;
                        reiniciar();
                    break;
                    case(6):
                        if(nota_tocada == nota_cancion){
                            TA1CCR0 = 2865-1;

                            if(nota_anterior == 'l' && cont_la == 2)
                            {
                                nota_cancion = 'd';
                                nota_anterior = nota_tocada;
                                cont_d = 1;
                            }
                            else if(nota_anterior == 'l' && cont_la == 3)
                            {
                                nota_cancion = 'd';
                                nota_anterior = nota_tocada;
                                cont_d = 2;
                            }
                            else{
                               TA1CCR0 = 7000;
                               reiniciar();
                            }
                        }
                        else{
                           TA1CCR0 = 7000;
                           reiniciar();
                        }
                    break;
                }
             break;
             case (1):
                TA1CCR0 = ArrayFrec[i];
                TA1CTL |= (MC_1 + TACLR);
                TA1CCTL0 = OUTMOD_4;
                Delay = DelayNotas[i];
                    Repeticion =  (Delay / 34375)+1;
                    for(j = 0; j < Repeticion;j++ )
                    {
                         TA0CCR1 = TA0R + Delay;
                         while (TA0R != TA0CCR1);
                         Delay = Delay - 34375;
                    }
                    if(Repeticion == 3)
                    {
                        TA1CCR0 = 0;
                        SET(EVENTO_TIEMPO);
                    }
                cancion=0;
                cont_nota = 0;
                if(cancion == 0)
                {
                    cancion=1;
                    SET(EVENTO_TIEMPO);
                    i++;
                    Delay=0;
                    if(i == 26)
                    {
                        TA1CTL &= ~(MC_1); // Desactiva el modo cuenta
                        TA0CCTL1 &= ~(CCIFG);
                        TA1CCTL0  &= ~ (CCIE);
                        sonar = 2;
                    }
                }

          break;
             case(2):
                 Contrasena = "ETR2022";

             break;
             } // switch
        nuevo_pulsador = 0;
    } // while (1)
} // void main(void)

void reiniciar(void)
{
    nota_cancion = 'l';
    nota_anterior = 'x';
    nota_tocada = 'x';
    cont_la = 0;
    cont_d = 0;
    cont_s = 0;
}

#pragma vector = PORT1_VECTOR
__interrupt void RTI_P1(void) {

    if ((P1IE & P1IFG & BIT4) != 0) {
        P1IFG &= ~(BIT4);
        P1IE &= ~(BIT4);
        WDTCTL = WDT_MDLY_32;
        IFG1 &= ~(WDTIFG);
        IE1 |= WDTIE;
        pulsador = 3;
        if ((P1IES & BIT4) != 0) {
            SET(EVENTO_Sx_PULSAR);
        } // if ((P1IES & BIT4) != 0)

        else {
            SET(EVENTO_Sx_SOLTAR);
        } // else: if ((P1IES & BIT4) != 0)

        P1IES ^= (BIT4);
    } // if ((P1IE & P1IFG & BIT4) != 0)

    if (eventos) {
        __low_power_mode_off_on_exit();
    } // if (eventos)

} // __interrupt void RTI_P1(void)

#pragma vector = PORT2_VECTOR
__interrupt void RTI_P2(void) {

    if ((P2IE & P2IFG & BIT1) != 0) {
        P2IFG &= ~(BIT1);
        P2IE &= ~(BIT1);
        WDTCTL = WDT_MDLY_32;
        IFG1 &= ~(WDTIFG);
        IE1 |= WDTIE;
        pulsador = 4;
        if ((P2IES & BIT1) != 0) {
            SET(EVENTO_Sx_PULSAR);
        } // if ((P2IES & BIT1) != 0)

        else {
            SET(EVENTO_Sx_SOLTAR);
        } // else: if ((P2IES & BIT1) != 0)

        P2IES ^= (BIT1);
    } // if ((P2IE & P2IFG & BIT1) != 0)

    if ((P2IE & P2IFG & BIT2) != 0) {
        P2IFG &= ~(BIT2);
        P2IE &= ~(BIT2);
        WDTCTL = WDT_MDLY_32;
        IFG1 &= ~(WDTIFG);
        IE1 |= WDTIE;
        pulsador = 5;

        if ((P2IES & BIT2) != 0) {
            SET(EVENTO_Sx_PULSAR);
        } // if ((P2IES & BIT2) != 0)

        else {
            SET(EVENTO_Sx_SOLTAR);
        } // else: if ((P2IES & BIT2) != 0)

        P2IES ^= (BIT2);
    } // if ((P2IE & P2IFG & BIT2) != 0)

    if ((P2IE & P2IFG & BIT3) != 0) {
        P2IFG &= ~(BIT3);
        P2IE &= ~(BIT3);
        WDTCTL = WDT_MDLY_32;
        IFG1 &= ~(WDTIFG);
        IE1 |= WDTIE;
        pulsador = 6;

        if ((P2IES & BIT3) != 0) {
            SET(EVENTO_Sx_PULSAR);
        } // if ((P2IES & BIT3) != 0)

        else {
            SET(EVENTO_Sx_SOLTAR);
        } // else: if ((P2IES & BIT3) != 0)

        P2IES ^= (BIT3);
    } // if ((P2IE & P2IFG & BIT3) != 0)

    if (eventos) {
        __low_power_mode_off_on_exit();
    } // if (eventos)

} // __interrupt void RTI_P2(void)

#pragma vector = WDT_VECTOR
__interrupt void RTI_WD(void) {
    WDTCTL = WDTPW + WDTHOLD;
    IE1 &= ~(WDTIE);
    P1IFG &= ~(BIT4+BIT3);
    P2IFG &= ~(BIT3+BIT2+BIT1);
    P1IE |= (BIT4+BIT3);
    P2IE |= (BIT3+BIT2+BIT1);
} // __interrupt void RTI_WD(void)


#pragma vector = TIMER0_A1_VECTOR
__interrupt void RTI_TA0CCR12(void) {

    switch (TA0IV) {
        case (TA0IV_TACCR1): {
            if(cancion)
            {
                cont_nota++;
                if (cont_nota == 2 ) {
                   SET(EVENTO_TIEMPO);
                   TA0CCTL1 &= ~(CCIE);
                   cancion = 0;

               } // if (cont_nota == 2)
            }

            break;
        } // case (TA0IV_TACCR1)
        case (TA0IV_TACCR2): {

            if (fin == 1)
            {
                cont_fallo ++;
                P2OUT ^= (BIT7+BIT6+BIT5+BIT4);
                TA0CCR2 += 34375;
                if(cont_fallo == 8)
                {
                    P2OUT &= ~ (BIT7+BIT6+BIT5+BIT4);
                    fin = 0;
                    sonar = 1;
                    cancion = 1;
                    TA0CCR1 += DelayNotas[i-1];
                    TA0CCTL1 &= ~(CCIFG);
                    TA0CCTL1 |= (CCIE);
                    P1IE  &= ~(BIT4+BIT3);
                    P2IE  &= ~ (BIT3+BIT2+BIT1);
                }
            }
            break;
        } // case (TA0IV_TACCR2)

        case (TA0IV_TAIFG): {
            break;
        } // case (TA0IV_TAIFG)

    } // switch (TA0IV)

    if (eventos) {
        __low_power_mode_off_on_exit();
    } // if (eventos)

} // __interrupt void RTI_TA0CCR12(void)



