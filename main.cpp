//Librerias
#include <iostream>
#include <fstream>
#include <windows.h>
#include <conio.h>

//Usar el estandar
using namespace std;

//Constantes
#define DIAMETRO_REFERENCIA 20.0
#define TOLERANCIA_MAXIMA_1 2.0
#define TOLERANCIA_MAXIMA_2 5.0
#define LIMITE_BUFFER 256

//Estructura para almacenar los datos formateados
struct DatosCinta {
    int codigo;
    char fabricante;
    float diametro;
};

//Funciones
void gestionHilos();
void lecturaSensor(char *cinta);
void moverBrazo(char * pieza);

//Variables
bool detenerCintas = false;
HANDLE semaforo =CreateSemaphore(NULL, 1, 1, NULL);

//Main
int main() {
    
    //Creamos los hilos
    HANDLE hiloPrincipal = CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)gestionHilos,NULL, 0, NULL);
    
    //Mensaje al usuario
    cout << "Presiona una tecla para detener los hilos... \n";
    
    //Esperar
    getch();
    
    //Detenerlos
    detenerCintas = true;
    
    //Esperar a que acaben
    WaitForSingleObject(hiloPrincipal, INFINITE);
    CloseHandle(hiloPrincipal);
    
    return 0;
}

//Funcion Hilo para dejar el main mas limpio, si necesitamos ampliar el programa nos sera mas sencillo
void gestionHilos() {
    
    //Hilo sensor 1
	CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)lecturaSensor, (LPVOID)"cinta1.txt",0,NULL);
    
    //Hilo sensor 2
    CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)lecturaSensor, (LPVOID)"cinta2.txt",0,NULL);
}

//Funcion que simula el sensor de la cinta, va leyendo de 1 en 1
void lecturaSensor(char * cinta){
    
    /*Vamos a crear un archivo temporal
    que nos ayudara a hacer esta funcion
    este sera su nombre*/
    char tempFileName[LIMITE_BUFFER];
    
    //Copiar el nombre original en tempFileName
    strcpy(tempFileName, cinta);  
    
    //Concatenar la extensión ".temp"
    strcat(tempFileName, ".temp"); 
    
    //Mientras no se detengan las cintas los sensores estan funcionando
    while (!detenerCintas) {
        
        //Procedemos a leer la cinta
        //Variable para guardar el resultado
        char *resultado = (char *)malloc(LIMITE_BUFFER * sizeof(char));
        
        if (resultado == NULL) {
            // Manejo de error de asignación de memoria
            cout << "Error al asignar memoria." << endl;
            break;
        }
        
        resultado[0] = '\0';
        
        //Procedemos a leer el primer elemento de la cinta y eliminarlo de la cinta
        //Abrir archivo
        ifstream inputFile(cinta);
        
        //Crear archivo temporal
        ofstream tempOutputFile(tempFileName);
        
        //Comprobar si esta abierto
        if (!inputFile.is_open() || !tempOutputFile.is_open()) {
            //cout << "Error al abrir el archivo." << endl;
            goto vacio;
        }
        
        //Guardar la primera linea
        char buffer[LIMITE_BUFFER];

        if (!inputFile.getline(buffer, LIMITE_BUFFER)) {
            //cout << "Archivo vacio o error al leer." << endl;
            goto vacio;
        }

        //Guardar la linea
        strcpy(resultado, buffer);

        //El resto de lineas se ponen en el temporal
        while (inputFile.getline(buffer, LIMITE_BUFFER)) {
            tempOutputFile << buffer << endl;
        }

        //Se cierran los archivos
        inputFile.close();
        tempOutputFile.close();

        //Renombrar el archivo temporal como el archivo original
        remove(cinta);
        rename(tempFileName, cinta);

        //Solo hacer un sleep si no hay nada en el resultado para no saturar el pc
        if(0){
            vacio:
                Sleep(1000);
        }

        //Anadir a la cola para mover el brazo
        CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)moverBrazo, (LPVOID)resultado,0,NULL);
    }
}

void moverBrazo(char * pieza){
    
    //Si no hay pieza que mover no se hace nada
    if (pieza[0] == '\0') {
        //cout << "La cadena esta vacia. No se realiza ninguna accion." << endl;
        free(pieza); 
        return;
    }

    //Datos de la cinta
    DatosCinta datos;

    // Leer los datos de la cadena y asignarlos a la estructura
    int numero;
    sscanf(pieza, "%i %c %i %f", &numero, &datos.fabricante, &datos.codigo, &datos.diametro);
    
    //Liberamos la memoria para la pieza
    free(pieza);
    
    // Preparar la cadena formateada
    char cadenaFormateada[100];
    
    // Formatear la cadena
    sprintf(cadenaFormateada, "%i,%c,%f", datos.codigo, datos.fabricante, datos.diametro);

    //Imprimir la cadena formateada (Esta solo para probar)
    //cout << "Cadena formateada: " << cadenaFormateada << endl;

    // Calcular la diferencia porcentual con respecto al diámetro de referencia
    float diferenciaPorcentual = ((datos.diametro - DIAMETRO_REFERENCIA) / DIAMETRO_REFERENCIA) * 100;
    
    //Verificar si el brazo mecanico esta libre
    WaitForSingleObject(semaforo, INFINITE);

    // Determinar en qué archivo escribir según la tolerancia del diámetro
    if (abs(diferenciaPorcentual) <= TOLERANCIA_MAXIMA_1) {
        // Escribir en Pallet-1.csv en modo append
        ofstream archivoPallet1Stream("pallet-1.csv", ios::app);
        archivoPallet1Stream << cadenaFormateada << endl;
    } else if (abs(diferenciaPorcentual) <= TOLERANCIA_MAXIMA_2) {
        // Escribir en Pallet-2.csv en modo append
        ofstream archivoPallet2Stream("pallet-2.csv",ios::app);
        archivoPallet2Stream << cadenaFormateada << endl;
    } else {
        // Escribir en Pallet-3.csv en modo append
        ofstream archivoPallet3Stream("pallet-3.csv", ios::app);
        archivoPallet3Stream << cadenaFormateada << endl;
    }

    //Cuando acaba deja el brazo mecanico libre (Libera el semaforo)
    ReleaseSemaphore(semaforo, 1, NULL);
}