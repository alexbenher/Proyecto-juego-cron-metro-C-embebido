#include <msp430.h>

void config_ACLK_to_32KHz_crystal() {
    PJSEL1 &= ~BIT4;
    PJSEL0 |= BIT4;
    CSCTL0 = CSKEY;
    do {
        CSCTL5 &= ~LFXTOFFG;
        SFRIFG1 &= ~OFIFG;
    } while((CSCTL5 & LFXTOFFG) != 0);
    CSCTL0_H = 0;
}

void Initialize_LCD() {
    PJSEL0 = BIT4 | BIT5;
    LCDCPCTL0 = 0xFFFF;
    LCDCPCTL1 = 0xFC3F;
    LCDCPCTL2 = 0x0FFF;
    CSCTL0_H = CSKEY >> 8;
    CSCTL4 &= ~LFXTOFF;
    do {
        CSCTL5 &= ~LFXTOFFG;
        SFRIFG1 &= ~OFIFG;
    } while (SFRIFG1 & OFIFG);
    CSCTL0_H = 0;
    LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;
    LCDCVCTL = VLCD_1 | VLCDREF_0 | LCDCPEN;
    LCDCCPCTL = LCDCPCLKSYNC;
    LCDCMEMCTL = LCDCLRM;
    LCDCCTL0 |= LCDON;
}

const char LCD_Num[10] = {
    0xFC, 0x60, 0xDB, 0xF3, 0x67,
    0xB7, 0xBF, 0xE0, 0xFF, 0xF7
};

const char LCD_Char[26] = {
    0xEF, 0x3F, 0x9C, 0x7B, 0x9F,
    0x8F, 0xBD, 0x6F, 0x60, 0x78,
    0x0F, 0x1C, 0xEC, 0x2B, 0xFC,
    0xCF, 0xE7, 0x0B, 0xB7, 0x1F,
    0x7C, 0x7C, 0x6C, 0x6F, 0x77,
    0xDB
};

void display_num_lcd(unsigned int n) {
    int i = 0;
    do {
        unsigned int digit = n % 10;
        switch(i) {
            case 0: LCDM8  = LCD_Num[digit]; break;
            case 1: LCDM15 = LCD_Num[digit]; break;
            case 2: LCDM19 = LCD_Num[digit]; break;
            case 3: LCDM4  = LCD_Num[digit]; break;
            case 4: LCDM6  = LCD_Num[digit]; break;
        }
        n /= 10;
        i++;
    } while(i < 5);
}

void display_str_lcd(char* str) {
    int i;
    for (i = 0; i < 5; i++) {
        char c = str[i];
        char seg = 0x00;

        if (c != '\0') {
            if      (c >= '0' && c <= '9') seg = LCD_Num[c - '0'];
            else if (c >= 'A' && c <= 'Z') seg = LCD_Char[c - 'A'];
            else if (c >= 'a' && c <= 'z') seg = LCD_Char[c - 'a'];
        }

        switch(i) {
            case 0: LCDM6  = seg; break;  // posición 1 (izquierda)
            case 1: LCDM4  = seg; break;  // posición 2
            case 2: LCDM19 = seg; break;  // posición 3
            case 3: LCDM15 = seg; break;  // posición 4
            case 4: LCDM8  = seg; break;  // posición 5 (derecha)
        }
    }
}

#define ESTADO_ESPERANDO    0
#define ESTADO_CUENTA_ATRAS 1
#define ESTADO_JUGANDO      2
#define ESTADO_RESOLUCION   3

volatile int estado_actual = ESTADO_ESPERANDO;

volatile unsigned long ticks_cronometro = 0;
volatile unsigned long objetivo_ticks   = 0;
volatile unsigned long tiempo_j1        = 0;
volatile unsigned long tiempo_j2        = 0;
volatile int j1_ha_pulsado = 0;
volatile int j2_ha_pulsado = 0;

volatile int segundos_cuenta_atras = 3;
volatile int ticks_cuenta_atras    = 0;

volatile unsigned int numero_tecleado = 0;

void enviar_cadena_uart(char* texto) {
    while (*texto) {
        while (!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = *texto;
        texto++;
    }
}

int main(void) {
    WDTCTL  = WDTPW | WDTHOLD;
    PM5CTL0 &= ~LOCKLPM5;

    config_ACLK_to_32KHz_crystal();
    Initialize_LCD();
    display_num_lcd(0);

    P1DIR |= BIT0;
    P1OUT &= ~BIT0;
    P9DIR |= BIT7;
    P9OUT &= ~BIT7;

    P1DIR &= ~(BIT1 | BIT2);
    P1REN |=  (BIT1 | BIT2);
    P1OUT |=  (BIT1 | BIT2);
    P1IE  |=  (BIT1 | BIT2);
    P1IES |=  (BIT1 | BIT2);
    P1IFG &= ~(BIT1 | BIT2);

    P3SEL0 |=  (BIT4 | BIT5);
    P3SEL1 &= ~(BIT4 | BIT5);

    CSCTL0_H = CSKEY >> 8;
    CSCTL1   = DCOFSEL_3 | DCORSEL;
    CSCTL2   = SELA__LFXTCLK | SELS__DCOCLK | SELM__DCOCLK;
    CSCTL3   = DIVA__1 | DIVS__1 | DIVM__1;
    CSCTL0_H = 0;

    UCA1CTLW0  = UCSWRST;
    UCA1CTLW0 |= UCSSEL__SMCLK;
    UCA1BR0    = 52;
    UCA1BR1    = 0x00;
    UCA1MCTLW |= UCOS16 | UCBRF_1 | 0x4900;
    UCA1CTLW0 &= ~UCSWRST;
    UCA1IE    |= UCRXIE;

    TA1CCR0  = 10000;
    TA1CCTL0 |= CCIE;
    TA1CTL   = TASSEL__SMCLK | ID__8 | MC__UP | TACLR;

    __enable_interrupt();

    enviar_cadena_uart("\r\n--- NUEVO JUEGO ---\r\nIntroduce los segundos y pulsa Enter: ");

    while(1) {
        switch(estado_actual) {

            case ESTADO_ESPERANDO:
                break;

            case ESTADO_CUENTA_ATRAS:
                break;

            case ESTADO_JUGANDO:
                if (j1_ha_pulsado && j2_ha_pulsado) {
                    estado_actual = ESTADO_RESOLUCION;
                }
                break;

            case ESTADO_RESOLUCION: {
                unsigned long errorj1, errorj2;

                errorj1 = (tiempo_j1 > objetivo_ticks) ? tiempo_j1 - objetivo_ticks : objetivo_ticks - tiempo_j1;
                errorj2 = (tiempo_j2 > objetivo_ticks) ? tiempo_j2 - objetivo_ticks : objetivo_ticks - tiempo_j2;

                enviar_cadena_uart("\r\n\r\n--- RESULTADOS ---\r\n");

                if (errorj1 < errorj2) {
                    enviar_cadena_uart("GANA EL JUGADOR 1\r\n");
                    display_str_lcd("JUG1 ");
                } else if (errorj2 < errorj1) {
                    enviar_cadena_uart("GANA EL JUGADOR 2\r\n");
                    display_str_lcd("JUG2 ");
                } else {
                    enviar_cadena_uart("EMPATE\r\n");
                    display_str_lcd("EMPAT");
                }

                j1_ha_pulsado  = 0;
                j2_ha_pulsado  = 0;
                P1OUT         &= ~BIT0;
                P9OUT         &= ~BIT7;
                numero_tecleado = 0;

                enviar_cadena_uart("\r\nArbitro, introduce los segundos y pulsa Enter: ");
                estado_actual = ESTADO_ESPERANDO;
                break;
            }
        }
    }
}

#pragma vector = USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void) {
    if (UCA1IFG & UCRXIFG) {
        char c = UCA1RXBUF;

        if (estado_actual == ESTADO_ESPERANDO) {
            while (!(UCA1IFG & UCTXIFG));
            UCA1TXBUF = c;

            if (c == '\r') {
                objetivo_ticks         = (unsigned long)numero_tecleado * 100;
                segundos_cuenta_atras  = 3;
                ticks_cuenta_atras     = 0;
                display_num_lcd(segundos_cuenta_atras);
                estado_actual          = ESTADO_CUENTA_ATRAS;
            } else if (c >= '0' && c <= '9') {
                numero_tecleado = (numero_tecleado * 10) + (c - '0');
            }
        }
        UCA1IFG &= ~UCRXIFG;
    }
}

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void) {
    if (estado_actual == ESTADO_CUENTA_ATRAS) {
        ticks_cuenta_atras++;
        if (ticks_cuenta_atras >= 100) {
            ticks_cuenta_atras = 0;
            segundos_cuenta_atras--;
            if (segundos_cuenta_atras > 0) {
                display_num_lcd(segundos_cuenta_atras);
            } else {
                display_num_lcd(8888);
                ticks_cronometro = 0;
                estado_actual    = ESTADO_JUGANDO;
            }
        }
    } else if (estado_actual == ESTADO_JUGANDO) {
        ticks_cronometro++;
    }
    TA1CCTL0 &= ~CCIFG;
}

#pragma vector = PORT1_VECTOR
__interrupt void ISR_Puerto1(void) {
    if (estado_actual == ESTADO_JUGANDO) {
        if ((P1IFG & BIT1) && !j1_ha_pulsado) {
            tiempo_j1    = ticks_cronometro;
            j1_ha_pulsado = 1;
            P1OUT        |= BIT0;
        }
        if ((P1IFG & BIT2) && !j2_ha_pulsado) {
            tiempo_j2    = ticks_cronometro;
            j2_ha_pulsado = 1;
            P9OUT        |= BIT7;
        }
    }
    P1IFG &= ~(BIT1 | BIT2);
}


