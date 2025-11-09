#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/videoio.hpp"
#include <iostream>
#include <cmath>
#include <chrono> 

#include "tof.h"
#include "spin.h"

using namespace std;
using namespace cv;

// DISTANCIAS FOCALES
const double FX = 300; // Focal distance in pixels (eje X)
const double ANCHO_REAL = 14.5; // Real: 1.7 | Celular: 6.1 | Tablet: 14.5

// THREASHOLDS
const float THRESHOLD_MOVE = 5;  //Real: 2 | Celular: 5 | Tablet: 5
const float DISTANCE_GOAL = 25; //Real: 6 | Celular: 15 | Tablet: 25
 
const int CENTER_GOAL = 160;             

double current_distance = 0.0; 
int distance_px = 0;

// PROPORTIONAL GAINS 
const float KP_TURN_LEFT = 0.19;
const float KP_TURN_RIGHT = 0.13;
const float KP_DISTANCE = 0.6; 
const int PWM_BASE = 50;

// CASCADE
CascadeClassifier cascade;
const double scale = 1.2;
const int minNeighbors = 10;  

// SEARCH MODE
const long long SEARCH_TIMEOUT_SECONDS = 20;
std::chrono::steady_clock::time_point last_detection_time; // Almacena el último momento de detección

// PREVENTS CRASHING
const int long STOP_DISTANCE = 50; //mm

// Función para detección y visualización
void detectAndDraw(Mat& img)
{
    vector<Rect> faces;
    const static Scalar colors[] = {
        Scalar(255,0,0), Scalar(255,128,0), Scalar(255,255,0),
        Scalar(0,255,0), Scalar(0,128,255), Scalar(0,255,255),
        Scalar(0,0,255), Scalar(255,0,255)
    };

    Mat gray, smallImg;
    double fx = 1.0 / scale;
    cvtColor(img, gray, COLOR_BGR2GRAY);
    resize(gray, smallImg, Size(), fx, fx, INTER_LINEAR_EXACT);
    equalizeHist(smallImg, smallImg);

    cascade.detectMultiScale(smallImg, faces, scale, minNeighbors, 0 | CASCADE_SCALE_IMAGE, Size(20, 20));
	
    for (size_t i = 0; i < faces.size(); i++)
    {
        Rect r = faces[i];
        Scalar color = colors[i % 8];
        double aspect_ratio = (double)r.width / r.height;
       
        // --- CÁLCULO DE LA DISTANCIA DEL ROSTRO ---
        if (r.width > 0) { // Avoids divisions by zero
            current_distance = (ANCHO_REAL* FX) / r.width;   // Distancia = (Tamaño_Real_del_Objeto * Distancia_Focal) / Tamaño_del_Objeto_en_Píxeles

            // Mostrar la distancia en la imagen
            string text = "Distancia: " + to_string(static_cast<int>(current_distance)) + " cm"; // Cast in int for less decimals
            putText(img, text, Point(r.x, r.y - 10), FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 1);
        }

	// -------- CALCULO DEL CENTROIDE ------
	Point centroid(cvRound((r.x + r.width * 0.5) * scale), cvRound((r.y + r.height * 0.5) * scale));
	//circle(img, centroid, 5, Scalar(0, 0, 255), -1);
	distance_px = centroid.x;
        putText(img, to_string(centroid.x), centroid, FONT_HERSHEY_SIMPLEX, 0.3, Scalar(0, 0, 255), 1);


        if (0.75 < aspect_ratio && aspect_ratio < 1.3)
        {
            Point center(cvRound((r.x + r.width * 0.5) * scale), cvRound((r.y + r.height * 0.5) * scale));
            int radius = cvRound((r.width + r.height) * 0.25 * scale);
            circle(img, center, radius, color, 3, 8, 0);
        }
        else
        {
            rectangle(img,
                Point(cvRound(r.x * scale), cvRound(r.y * scale)),
                Point(cvRound((r.x + r.width - 1) * scale), cvRound((r.y + r.height - 1) * scale)),
                color, 3, 8, 0);
        }
    }
    imshow("Carrito", img);
}

int main()
{
    // Carga del clasificador
    string cascadeName = "/home/MyJ/codigos/18_carrito/ojoMike.xml";
    if (!cascade.load(cascadeName))
    {
        cerr << "ERROR: Could not load classifier cascade" << endl;
        return -1;
    }

    // Iniciar el video
    VideoCapture capture(0);
    if (!capture.isOpened())
    {
        cerr << "ERROR: Could not open camera" << endl;
        return -1;
    }

    capture.set(CAP_PROP_FRAME_WIDTH, 320);
    capture.set(CAP_PROP_FRAME_HEIGHT, 240);
    //capture.set(CAP_PROP_FPS, 30);

    cout << "Video capturing has been started ..." << endl;
    namedWindow("Carrito", WINDOW_AUTOSIZE);

    // Initialize ToF and Spin (motor control)
    // tofInit: (I2C bus, I2C address, long range mode enable)
    tofInit(1, 0x29, 1);
    cout << "Sensor ToF VL53L0X inicializado correctamente." << endl;
  
    spinInit(); // Initialize motor control system
    int pwma, pwmb;
    int error_turn,  turn_adjustment, speed_adjustment;
    float error_distance;
    int distance = 0;
    bool measure_dist = true;

    last_detection_time = std::chrono::steady_clock::now();
    
    for (;;)
    {

	if(measure_dist == true){
            // ANTI-CRASHING
            distance = tofReadDistance();   
            cout << distance << "cm" << endl;
            if(distance < STOP_DISTANCE){
                cout << "Obstaculo detectado ";
                move_cm(5, BACKWARDS); // aprox 10 
	        last_detection_time = std::chrono::steady_clock::now(); // Reinicia el temporizador
	    }
        }
	
        Mat frame;
        capture >> frame;
        if (frame.empty()) break;

	current_distance = 0;
	distance_px = 0;

        detectAndDraw(frame);

        // Control logic for the car based on Mike's movement
        if (current_distance > 0.0) { // Check if a face was detected and distance updated
	    
	    // Reinicia el temporizador de no detección y sale del modo de búsqueda
            last_detection_time = std::chrono::steady_clock::now();

	    if (current_distance > (DISTANCE_GOAL + THRESHOLD_MOVE)) {   
		
                // --- Proportional Turning Control ---
                error_turn = CENTER_GOAL - distance_px;    // Positive if object is to the left, negative if to the right
                
                if (error_turn > 0) { // Objeto a la izquierda del centro, necesita girar a la izquierda
                    turn_adjustment = static_cast<int>(error_turn * KP_TURN_LEFT);
                } else { // Objeto a la derecha del centro, necesita girar a la derecha
                    turn_adjustment = static_cast<int>(error_turn * KP_TURN_RIGHT);
                }

                // Apply turning adjustment
                pwma = PWM_BASE + turn_adjustment; // Increase right motor speed for left turn, decrease for right turn
                pwmb = PWM_BASE - turn_adjustment; // Decrease left motor speed for left turn, increase for right turn

                // Clamp PWM values to valid range
                pwma = max(0, min(PWM_RANGE, pwma));
                pwmb = max(0, min(PWM_RANGE, pwmb));

                error_distance = current_distance - DISTANCE_GOAL;
                speed_adjustment = static_cast<int>(error_distance * KP_DISTANCE);

                // Increase speed based on how far away the object is
                pwma = min(PWM_RANGE, pwma + speed_adjustment);
                pwmb = min(PWM_RANGE, pwmb + speed_adjustment);

		cout << "Speed adjustment: " << speed_adjustment << " | Turn adjustment: " << turn_adjustment << " | Pwma : " << pwma << " | Pwmb: " << pwmb << endl;
           
		move(FOWARD, pwma, pwmb);
 	    }
            else if(current_distance < (DISTANCE_GOAL - THRESHOLD_MOVE)) {  // MUY CERCA
                move(BACKWARDS, PWMA_DUTY_CYCLE_BACKWARDS, PWMB_DUTY_CYCLE_BACKWARDS);
            } else {
                // Good distance
  	        stop_motors();
            }
        } else { // No se detectó ningún rostro
            auto current_time = std::chrono::steady_clock::now();
            auto elapsed_seconds = std::chrono::duration_cast<std::chrono::seconds>(current_time - last_detection_time).count();

            if (elapsed_seconds >= SEARCH_TIMEOUT_SECONDS) {  // Si ha pasado el tiempo de espera
                turn(RIGHT);
		usleep(3000*1000); // 1 ms = 1000 us
                stop_motors();
                last_detection_time = std::chrono::steady_clock::now(); // Reinicia el temporizador después del giro
            } else { 
                stop_motors();
                cout << "Tiempo sin detección: " << elapsed_seconds << "s" << endl;
            }
        }        
        char c = (char)waitKey(10);
        if (c == 27 || c == 'e') break;  // salir con ESC o e
        if (c == 'y') measure_dist = true;
        if (c == 'n') measure_dist = false;
    }

    cleanup_gpio();
    return 0;
}
