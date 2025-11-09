#include "spin.h" // Include the motor control header

using namespace std; 

// --- FONCTIONS ---
// Set up configurations
//
int spinInit(){
 // --- Inicializar WiringPi usando la numeración BCM ----
    if (wiringPiSetupGpio() == -1) {
        cerr << "Error: No se pudo inicializar WiringPi" << endl;
        return 1;
    }
    cout << "WiringPi inicializado correctamente (usando numeración BCM)." << endl;

    // --- Configura pines como salidas y crea SoftPWM ----
    // Pines de dirección
    pinMode(IN1A_PIN, OUTPUT);
    pinMode(IN2A_PIN, OUTPUT);
    pinMode(IN1B_PIN, OUTPUT);
    pinMode(IN2B_PIN, OUTPUT);

    // Pines Enable (para SoftPWM)
    // softPwmCreate(pin, valor_inicial, rango_maximo)
    if (softPwmCreate(ENA_PIN, 0, PWM_RANGE) != 0) {
        cerr << "Error al crear SoftPWM para ENA_PIN " << ENA_PIN << endl;
        cleanup_gpio();
        return 1;
    }
    if (softPwmCreate(ENB_PIN, 0, PWM_RANGE) != 0) {
        cerr << "Error al crear SoftPWM para ENB_PIN " << ENB_PIN << endl;
        cleanup_gpio();
        return 1;
    }

    // Pin STBY
    pinMode(STBY_PIN, OUTPUT);
    cout << "GPIOs configurados correctamente." << endl;

    // --- Habilitar el módulo Puente H poniendo STBY en ALTO ---
    digitalWrite(STBY_PIN, HIGH);
    cout << "Módulo Puente H habilitado (STBY HIGH)." << endl;
    usleep(100000); // Pequeño retardo (100ms) para que el módulo se active
    return 0;
}

// CLEAN GPIO RESOURCES
//
void cleanup_gpio() {
    stop_motors(); // Asegurarse de que los motores estén detenidos

    // Deshabilitar el módulo Puente H
    digitalWrite(STBY_PIN, LOW);
    cout << "Módulo Puente H deshabilitado (STBY LOW)." << endl;

    cout << "GPIOs limpiados." << endl;
}

// STOP BOTH MOTORS
//
void stop_motors() {
    softPwmWrite(ENA_PIN, 0); // Establecer ciclo de trabajo a 0 para detener Motor A
    softPwmWrite(ENB_PIN, 0); // Establecer ciclo de trabajo a 0 para detener Motor B
    digitalWrite(IN1A_PIN, LOW);
    digitalWrite(IN2A_PIN, LOW);
    digitalWrite(IN1B_PIN, LOW);
    digitalWrite(IN2B_PIN, LOW);
    //cout << "Motores detenidos." << endl;
}

// MOVE IN CM
//
void move_cm(int cm, Direction direction) {
    if(direction == BACKWARDS){
       cout << "Moviendo el carro hacia atras" << endl;

       // Establecer dirección para Motor A (atras - derecho)
       digitalWrite(IN1A_PIN, HIGH);
       digitalWrite(IN2A_PIN, LOW);

       // Establecer dirección para Motor B (atras - izquierdo)
        digitalWrite(IN1B_PIN, LOW);
        digitalWrite(IN2B_PIN, HIGH);

       // Aplicar PWM para controlar la velocidad
       softPwmWrite(ENA_PIN, PWMA_DUTY_CYCLE_BACKWARDS);
       softPwmWrite(ENB_PIN, PWMB_DUTY_CYCLE_BACKWARDS);
    }
    if(direction == FOWARD){
       cout << "Moviendo el carro hacia adelante" << endl;

       // Establecer dirección para Motor A (adelante - derecho)
       digitalWrite(IN1A_PIN, LOW);
       digitalWrite(IN2A_PIN, HIGH);

       // Establecer dirección para Motor B (adelante - izquierdo)
       digitalWrite(IN1B_PIN, HIGH);
       digitalWrite(IN2B_PIN, LOW);
	
       // Aplicar PWM para controlar la velocidad
       softPwmWrite(ENA_PIN, PWMA_DUTY_CYCLE_FORWARD);
       softPwmWrite(ENB_PIN, PWMB_DUTY_CYCLE_FORWARD);
    }

    // Avanzar los cm indicados
    usleep(cm * ONE_CM_TO_MS * 1000); // 1 ms = 1000 us
    stop_motors();
    
}

// MOVE INDEFINITELY
//
void move(Direction direction, int PWMA, int PWMB){
    if(direction == BACKWARDS){
       cout << "Moviendo el carro hacia atras" << endl;

       // Establecer dirección para Motor A (atras - derecho)
       digitalWrite(IN1A_PIN, HIGH);
       digitalWrite(IN2A_PIN, LOW);

       // Establecer dirección para Motor B (atras - izquierdo)
        digitalWrite(IN1B_PIN, LOW);
        digitalWrite(IN2B_PIN, HIGH);

       // Aplicar PWM para controlar la velocidad
       softPwmWrite(ENA_PIN, PWMA_DUTY_CYCLE_BACKWARDS);
       softPwmWrite(ENB_PIN, PWMB_DUTY_CYCLE_BACKWARDS);
    } if(direction == FOWARD){
       cout << "Moviendo el carro hacia adelante" << endl;

       // Establecer dirección para Motor A (adelante - derecho)
       digitalWrite(IN1A_PIN, LOW);
       digitalWrite(IN2A_PIN, HIGH);

       // Establecer dirección para Motor B (adelante - izquierdo)
       digitalWrite(IN1B_PIN, HIGH);
       digitalWrite(IN2B_PIN, LOW);
	
       // Aplicar PWM para controlar la velocidad
       softPwmWrite(ENA_PIN, PWMA);
       softPwmWrite(ENB_PIN, PWMB);
    }

}

// TURN IN PX
//
void turn_px(int px, Turn direction){
    if(direction == LEFT){
       cout << "Moviendo el carro hacia la izquierda" << endl;

       // Establecer dirección para Motor A (adelante - derecho)
       digitalWrite(IN1A_PIN, LOW);
       digitalWrite(IN2A_PIN, HIGH);

       // Establecer dirección para Motor B (apagado - izquierdo)
        digitalWrite(IN1B_PIN, LOW);
        digitalWrite(IN2B_PIN, LOW);
    } if(direction == RIGHT){
       cout << "Moviendo el carro hacia la derecha" << endl;

       // Establecer dirección para Motor A (alto - derecho)
       digitalWrite(IN1A_PIN, LOW);
       digitalWrite(IN2A_PIN, LOW);

       // Establecer dirección para Motor B (adelante - izquierdo)
       digitalWrite(IN1B_PIN, HIGH);
       digitalWrite(IN2B_PIN, LOW);	
    }
    // Aplicar PWM para controlar la velocidad
    softPwmWrite(ENA_PIN, PWM_RANGE);
    softPwmWrite(ENB_PIN, PWM_RANGE);

    // Avanzar los px indicados
    usleep(px*PX_320_TO_MS*1000); // 1 ms = 1000 us
    stop_motors();
}

// TURN INDEFINITELY
//
void turn(Turn direction){
    if(direction == LEFT){
       cout << "Moviendo el carro hacia la izquierda" << endl;

       // Establecer dirección para Motor A (adelante - derecho)
       digitalWrite(IN1A_PIN, LOW);
       digitalWrite(IN2A_PIN, HIGH);

       // Establecer dirección para Motor B (apagado - izquierdo)
        digitalWrite(IN1B_PIN, LOW);
        digitalWrite(IN2B_PIN, LOW);
    } if(direction == RIGHT){
       cout << "Moviendo el carro hacia la derecha" << endl;

       // Establecer dirección para Motor A (alto - derecho)
       digitalWrite(IN1A_PIN, LOW);
       digitalWrite(IN2A_PIN, LOW);

       // Establecer dirección para Motor B (adelante - izquierdo)
       digitalWrite(IN1B_PIN, HIGH);
       digitalWrite(IN2B_PIN, LOW);	
    }
    // Aplicar PWM para controlar la velocidad
    softPwmWrite(ENA_PIN, PWMA_DUTY_CYCLE_FORWARD);
    softPwmWrite(ENB_PIN, PWMA_DUTY_CYCLE_FORWARD);
}
