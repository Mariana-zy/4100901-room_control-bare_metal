// main.c -----------------------------------------------------------------------------------------------

#include "gpio.h"          // Para controlar LEDs y botones
#include "systick.h"       // Para manejar retardos y tiempos
#include "rcc.h"           // Para inicializar el sistema de reloj
#include "uart.h"          // Para comunicación serial
#include "nvic.h"          // Para configurar interrupciones
#include "tim.h"           // Para PWM
#include "room_control.h"  // Lógica de control de la sala

// Flags para eventos
volatile uint8_t button_event = 0; // Indica que se presionó el botón
volatile char uart_event_char = 0; // Indica que se recibió un carácter por UART
                                 
// Función local para inicializar periféricos
static void peripherals_init(void)
{   
    // Inicialización del sistema
    rcc_init();  
    
    // Configuración de GPIOs
    gpio_init_pin(GPIOA, 5, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE);
    gpio_init_pin(GPIOB, 3, GPIO_MODE_OUTPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_NONE);
    gpio_init_pin(GPIOC, 13, GPIO_MODE_INPUT, GPIO_OTYPE_PP, GPIO_OSPEED_LOW, GPIO_PUPD_PU);   // Botón con pull-up

    // Inicialización de periféricos
    init_systick();
    init_uart();                    // Asumiendo función unificada
    nvic_exti_pc13_button_enable();
    nvic_usart2_irq_enable();
    tim3_ch1_pwm_init(1000);        // 1 kHz PWM
    init_gpio_uart();
}

int main(void)
{
    peripherals_init(); 
    room_control_app_init();
    uart_send_string("Sistema de Control de Sala Inicializado!\r\n"); 

    // Bucle principal: procesa eventos
    while (1) {
        if (button_event) {
            button_event = 0;
            room_control_on_button_press();
        }         
        if (uart_event_char) {
            char c = uart_event_char;
            uart_event_char = 0;
            room_control_on_uart_receive(c);
        }
        // Llamar a la función de actualización periódica
        room_control_update();
    }
}

// Manejadores de interrupciones
void EXTI15_10_IRQHandler(void)
{
    // Limpiar flag de interrupción
    if (EXTI->PR1 & (1 << 13)) {
        EXTI->PR1 |= (1 << 13);  // Clear pending
        button_event = 1;
    }
}

void USART2_IRQHandler(void)
{
    // Verificar si es recepción
    if (USART2->ISR & (1 << 5)) {  // RXNE
        uart_event_char = (char)(USART2->RDR & 0xFF);
    }
}