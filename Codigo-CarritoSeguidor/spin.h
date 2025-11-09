#ifndef _SPIN_H_
#define _SPIN_H_

#include <iostream>   // For std::cout and std::cerr
#include <unistd.h>   // For sleep and usleep
#include <wiringPi.h> // For WiringPi library
#include <softPwm.h>  // For SoftPWM from WiringPi
#include <chrono> 


typedef enum direction{
	FOWARD = 1,
	BACKWARDS = -1
} Direction;

typedef enum turn{
	RIGHT = 1,
	LEFT = -1
} Turn;

// ---- PINES ------
// Pines para el Motor A (Motor Derecho)
#define ENA_PIN 8    // Pin Enable/PWM para el Motor A (Conectar a PWMA del HW-166)
#define IN1A_PIN 1   // Pin de direcci�n 1 para el Motor A
#define IN2A_PIN 7   // Pin de direcci�n 2 para el Motor A

// Pines para el Motor B (Motor Izquierdo,)
#define ENB_PIN 12    // Pin Enable/PWM para el Motor B (Conectar a ENB del HW-166)
#define IN1B_PIN 20   // Pin de direcci�n 1 para el Motor B
#define IN2B_PIN 16   // Pin de direcci�n 2 para el Motor B

// Pin Standby para todo el m�dulo Puente H
#define STBY_PIN 21

// --- Par�metros de Control de Motor ---
#define PWM_RANGE 100              
#define PWMA_DUTY_CYCLE_FORWARD 57
#define PWMB_DUTY_CYCLE_FORWARD 63
#define PWMA_DUTY_CYCLE_BACKWARDS 57
#define PWMB_DUTY_CYCLE_BACKWARDS 57
#define ONE_CM_TO_MS 150
#define PX_320_TO_MS 2   


// --- FONCTIONS ---
// CLEAN GPIO RESOURCES
//
void cleanup_gpio();

// STOP BOTH MOTORS
//
void stop_motors();

// SETUP CONFIGURATIONS
//
int spinInit();

// MOVE IN CM
//
void move_cm(int cm, Direction direction);

// MOVE INDEFINITELY
//
void move(Direction dir, int PWMA, int PAMB);


// TURN IN PX
//
void turn_px(int px, Turn direction);

// TURN INDEFINITELY
//
void turn(Turn direction);

#endif // _SPIN_H
