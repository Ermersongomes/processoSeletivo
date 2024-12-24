//biblioteca utilizadas
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"

//Definição dos pinos
#define BUZZER_PIN 28
#define BOTAO_PEDESTRE 17
#define LED_LARANJA_BOTAO 28
#define LED_VERMELHO_SEMAFARO 2
#define LED_AMARELO_SEMAFARO 3
#define LED_VERDE_SEMAFARO 4
#define LED_VERMELHO_PEDESTRE 18
#define LED_VERDE_PEDESTRE 19

//DEfinição das frequencias e duração
#define FREQUENCIA_BAIXA 440
#define FREQUENCIA_ALTA 660
#define DURACAO_NOTA 850

//Função para configuração do Buzzer
void configurar_buzzer(int pin)
{
    gpio_set_function(pin, GPIO_FUNC_PWM); // Aqui eu escolho o pino do buzzer
    uint slice_num = pwm_gpio_to_slice_num(pin);
    pwm_config configuracao = pwm_get_default_config();
    pwm_config_set_clkdiv(&configuracao, 1.0);
    pwm_init(slice_num, &configuracao, true);
    pwm_set_gpio_level(pin, 0);
}

//Função para justar frequencia usada
void ajustar_frequencia(int pin, int freq)
{
    uint slice_num = pwm_gpio_to_slice_num(pin);
    uint32_t clock_div = clock_get_hz(clk_sys) / (freq * 5000);
    pwm_set_wrap(slice_num, 5000 - 1);
    pwm_set_clkdiv(slice_num, clock_div);
}

//Função responsavel por emitir o som no pino escolhido
void emitir_som(int pin, int freq, int duracao_ms)
{
    ajustar_frequencia(pin, freq);
    pwm_set_gpio_level(pin, 2500);
    sleep_ms(duracao_ms);
    pwm_set_gpio_level(pin, 0);
}

//Aqui inicializo todos os pinos que irei usar
void inicializar_pinos()
{  
    //inicialização do buzzer
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    //inicialização do puhs botton
    gpio_init(BOTAO_PEDESTRE);
    gpio_set_dir(BOTAO_PEDESTRE, GPIO_IN);
    gpio_pull_up(BOTAO_PEDESTRE); // AQUI ATIVA O RESITOR "VER DEPOIS O PORQUE"

    //inicalização leds do semafaro
    gpio_init(LED_VERMELHO_SEMAFARO);
    gpio_set_dir(LED_VERMELHO_SEMAFARO, GPIO_OUT);
    gpio_init(LED_AMARELO_SEMAFARO);
    gpio_set_dir(LED_AMARELO_SEMAFARO, GPIO_OUT);
    gpio_init(LED_VERDE_SEMAFARO);
    gpio_set_dir(LED_VERDE_SEMAFARO, GPIO_OUT);

    //inicialização leds pedeste
    gpio_init(LED_VERMELHO_PEDESTRE);
    gpio_set_dir(LED_VERMELHO_PEDESTRE, GPIO_OUT);
    gpio_init(LED_VERDE_PEDESTRE);
    gpio_set_dir(LED_VERDE_PEDESTRE, GPIO_OUT);
}

// Aqui a função verifica se o  botão conectado ao pino botão_pedestre foi precionados e retorna TRUE OU FALSE
bool botao_pressionado()
{
    return gpio_get(BOTAO_PEDESTRE) == 0;
}


//Função que controla o semafaros para os carros
void controle_semafaro()
{
    // Verde para carros
    gpio_put(LED_VERMELHO_PEDESTRE, 1); // o codigo sempre inicia com verde
    gpio_put(LED_VERDE_PEDESTRE, 0);
    gpio_put(LED_VERDE_SEMAFARO, 1);

    for (int i = 0; i < 800; i++) // contador que espera 8 segundos no verde e verificar se o botão foi presionado nesse periodo.
    {
        sleep_ms(10);
        if (botao_pressionado())
            return; // Interrompe se o botão for pressionado
    }

    gpio_put(LED_VERDE_SEMAFARO, 0);

    // Amarelo carros
    gpio_put(LED_AMARELO_SEMAFARO, 1); // contador que espera 2 segundos no amarelo e verificar se o botão foi presionado nesse periodo.
    for (int i = 0; i < 200; i++)
    {
        sleep_ms(10);
        if (botao_pressionado())
            return;
    }

    gpio_put(LED_AMARELO_SEMAFARO, 0);

    // Vermelho carros
    gpio_put(LED_VERMELHO_SEMAFARO, 1); 
    for (int i = 0; i < 1000; i++)// contador que espera 10 segundos no vermelho e verificar se o botão foi presionado nesse periodo.
    {
        sleep_ms(10);
        if (botao_pressionado())
            return;
    }

    gpio_put(LED_VERMELHO_SEMAFARO, 0);
}

//Função para o controle de travesia do pedestre
void controle_pedestre()
{
    if (gpio_get(LED_VERDE_SEMAFARO)) // verifica se o sefamaro esta no verde
    {
        gpio_put(LED_VERDE_SEMAFARO, 0);

        gpio_put(LED_AMARELO_SEMAFARO, 1);
        sleep_ms(5000);
        gpio_put(LED_AMARELO_SEMAFARO, 0);
        gpio_put(LED_VERMELHO_SEMAFARO, 1); // O sinal dos carros fica vermelho
        gpio_put(LED_VERMELHO_PEDESTRE, 0);
        gpio_put(LED_VERDE_PEDESTRE, 1); // Liberado para pedestre

        if (gpio_get(LED_VERDE_PEDESTRE)) // se o controle de pedestre for verde, emite o aviso sonoro
        {
            // Loop para manter o aviso sonoro e luminoso por 15 segundos 
            for (int i = 0; i < 9; i++) // <-- GAMBIARRA NÃO MEXER
            {                
                emitir_som(BUZZER_PIN, FREQUENCIA_BAIXA, DURACAO_NOTA);
                emitir_som(BUZZER_PIN, FREQUENCIA_ALTA, DURACAO_NOTA);
            }

            // Após o loop, desliga os LEDs e termina a parte do pedestre
            gpio_put(LED_LARANJA_BOTAO, 0);
            gpio_put(LED_VERMELHO_SEMAFARO, 0);
            gpio_put(LED_VERDE_PEDESTRE, 0);
            gpio_put(LED_VERMELHO_PEDESTRE, 1);
        }
    }

    if (gpio_get(LED_AMARELO_SEMAFARO)) // verifica se o led amrelo esta ligado atraves do true ou high
    {
        sleep_ms(5000);
        gpio_put(LED_AMARELO_SEMAFARO, 0);

        gpio_put(LED_VERMELHO_SEMAFARO, 1);
        gpio_put(LED_VERMELHO_PEDESTRE, 0);
        gpio_put(LED_VERDE_PEDESTRE, 1);
        if (gpio_get(LED_VERDE_PEDESTRE))
        {
            // Loop para manter o aviso sonoro e luminoso por 15 segundos 
            for (int i = 0; i < 9; i++) // <-- GAMBIARRA NÃO MEXER
            {
                emitir_som(BUZZER_PIN, FREQUENCIA_BAIXA, DURACAO_NOTA);
                emitir_som(BUZZER_PIN, FREQUENCIA_ALTA, DURACAO_NOTA);
            }

            // Após o loop, desliga os LEDs e termina a parte do pedestre
            gpio_put(LED_VERMELHO_SEMAFARO, 0);
            gpio_put(LED_VERDE_PEDESTRE, 0);
            gpio_put(LED_VERMELHO_PEDESTRE, 1);
        }
    }

    if (gpio_get(LED_VERMELHO_SEMAFARO)) // verifica se o led esta no vermelho
    {
        gpio_put(LED_VERMELHO_PEDESTRE, 0);
        gpio_put(LED_VERDE_PEDESTRE, 1);
        if (gpio_get(LED_VERDE_PEDESTRE))
        {
            // Loop para manter o aviso sonoro e luminoso por 15 segundos
            for (int i = 0; i < 9; i++) // <-- GAMBIARRA NÃO MEXER
            {
                emitir_som(BUZZER_PIN, FREQUENCIA_BAIXA, DURACAO_NOTA);
                emitir_som(BUZZER_PIN, FREQUENCIA_ALTA, DURACAO_NOTA);
            }

            // Após o loop, desliga os LEDs e termina a parte do pedestre
            gpio_put(LED_VERMELHO_SEMAFARO, 0);
            gpio_put(LED_VERDE_PEDESTRE, 0);
            gpio_put(LED_VERMELHO_PEDESTRE, 1);
        }
    }
}

// Função principal
int main()
{
    stdio_init_all();
    inicializar_pinos();
    configurar_buzzer(BUZZER_PIN);

    while (true) //cria um loop infinito
    {
        if (botao_pressionado()) // verifica se o botão foi precionado, se sim aciona a função do controle de pedeste.
        {
            controle_pedestre();
            
        }
        else // se a primeria condição não foi satisfeita, aciona a função do controle do semafaro.
        {
            controle_semafaro();
        }
    }

    return 0;
}


//Para fazer esse exercicios usei como inspiração alguns exemplos da mentoria e do prof. Ninho do workwi 
//link: https://wokwi.com/projects/417289794886393857
//link: https://wokwi.com/projects/417299852817197057