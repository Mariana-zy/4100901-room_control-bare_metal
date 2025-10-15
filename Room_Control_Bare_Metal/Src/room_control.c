// room_control.c -----------------------------------------------------------------------------------------------

#include "room_control.h"

#include "gpio.h"    // Para controlar LEDs
#include "systick.h" // Para obtener ticks y manejar tiempos
#include "uart.h"    // Para enviar mensajes
#include "tim.h"     // Para controlar el PWM

// Definición de función GPIO 
void gpio_toggle_ld2(void);

// Estados de la sala
typedef enum {
    ROOM_IDLE,
    ROOM_OCCUPIED
} room_state_t;

// Variable de estado global
room_state_t current_state = ROOM_IDLE;

// variables estáticas para manejar tiempos y estados, solo se usan dentro de room_control.c
static uint32_t last_button_press_time = 0;    // Guarda el momento (en ms) en que se presionó el botón por última vez
static uint32_t led_on_time = 0;               // Guarda el momento (en ms) en que se encendió el LED 
static uint8_t pwm_duty = PWM_INITIAL_DUTY;    // Duty cycle actual del PWM (0-100) brillo del LED

// Variables para debounce
#define DEBOUNCE_DELAY_MS 200                  // 200 ms de filtro anti-rebote


void room_control_app_init(void)              
{
    led_off();                                 // Apaga el LED al iniciar
    current_state = ROOM_IDLE;                 // Estado inicial (IDLE)
    uart_send_string("Room Control Initialized\r\n");
}

void room_control_on_button_press(void)        // Maneja la pulsación del botón
{
    uint32_t now = systick_get_ms();           // ms han pasado desde que se inició el sistema

    if (now - last_button_press_time < DEBOUNCE_DELAY_MS)  // Ignora eventos muy seguidos (anti-rebote)
        return;

    last_button_press_time = now;              // Actualiza el tiempo de la última pulsación

    if (current_state == ROOM_IDLE)            // Si estaba en IDLE cambia a OCCUPIED
    {
        current_state = ROOM_OCCUPIED;
        led_on();
        uart_send_string("Estado: OCCUPIED\r\n");
    }
    else
    {
        current_state = ROOM_IDLE;  
        led_off();
        uart_send_string("Estado: IDLE\r\n");
    }
}

void room_control_on_uart_receive(char received_char)   // Procesa comandos recibidos por UART
{
    static uint8_t awaiting_pwm_digit = 0;              // Bandera para indicar que se espera un dígito después de 'B' o 'b'
    if (awaiting_pwm_digit && received_char >= '0' && received_char <= '9')    
    {
        uint8_t new_duty = (received_char - '0') * 10;  // Convierte el carácter recibido (‘0’–‘9’) en un valor numérico (0–9) y luego lo multiplica por 10
        tim3_ch1_pwm_set_duty_cycle(new_duty);          // Cambia la intensidad del LED al nuevo nivel
        pwm_duty = new_duty;                            // Guarda el nuevo valor de brillo
        awaiting_pwm_digit = 0;                         // Reinicia la bandera

        uart_send_string("PWM duty cycle: ");
        char msg[10];
        msg[0] = received_char;          // Copia el número recibido 
        msg[1] = '\0';                   // Termina la cadena
        uart_send_string(msg);           // Envía el número 
        uart_send_string("0%\r\n");      // Completa el mensaje con "0%"
        
        return;
    }

    switch (received_char)
    {
        case 'B':
        case 'b':
            uart_send_string("Comando B recibido. Envie un número (0-9) para el duty.\r\n");
            awaiting_pwm_digit = 1;
            break;

        case 'H':
        case 'h':
            tim3_ch1_pwm_set_duty_cycle(100);
            pwm_duty = 100;                       // Establece el PWM al 100% 
            uart_send_string("PWM al 100%\r\n");
            break;

        case 'L':
        case 'l':
            tim3_ch1_pwm_set_duty_cycle(0);
            pwm_duty = 0;                         // Establece el PWM al 0%
            uart_send_string("PWM al 0%\r\n");
            break;

        case 'O':
        case 'o':
            current_state = ROOM_OCCUPIED;          // Cambia el estado a ocupado 
            led_on();
            tim3_ch1_pwm_set_duty_cycle(100);
            uart_send_string("Sala ocupada\r\n"); 
            led_on_time = systick_get_ms();
            break;

        case 'I':
        case 'i':
            current_state = ROOM_IDLE;              // Cambia el estado a inactivo
            led_off();
            tim3_ch1_pwm_set_duty_cycle(0);
            uart_send_string("Sala vacía\r\n");    
            break;

        default:
            break;
    }
}

void room_control_update(void)                     
{                                                                                                 
    if (current_state == ROOM_OCCUPIED) {               // Comprueba si la sala está actualmente ocupada
        uint32_t now = systick_get_ms();                // Guarda el tiempo actual para poder calcular cuánto ha pasado desde que se encendió el LED
        if (now - led_on_time >= LED_TIMEOUT_MS) {      // Calcula cuánto tiempo ha pasado desde que se encendió el LED (si han pasado 3 seg
            current_state = ROOM_IDLE;                  // Cambia el estado de la sala a IDLE
            led_off();
            tim3_ch1_pwm_set_duty_cycle(0);
            uart_send_string("Timeout: sala vacia\r\n");
        }
    }
}