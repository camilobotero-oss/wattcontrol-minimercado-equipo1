// =============================================================
// WattControl v2 - Gestor de energia de un minimercado (El Nino 2026)
// Evidencia 2 - Programacion Orientada a Objetos - IU Digital de Antioquia
// Negocio: minimercado (ultimo digito del documento: 8)
// =============================================================
// Compilar: g++ -std=c++17 -Wall -Wextra wattcontrol.cpp -o wattcontrol.exe

// --- Librerias -------------------------------------------------------------
// iostream: cout y cin
#include <iostream>
// iomanip: setw, setprecision y fixed, para alinear tablas y fijar decimales
#include <iomanip>
// string: el tipo string
#include <string>
// cstdlib: rand(), srand() y exit()
#include <cstdlib>
// limits: numeric_limits, para descartar toda una linea mal escrita
#include <limits>

using namespace std;

// --- Directivas del preprocesador ------------------------------------------
// Horas de la simulacion: un dia
#define HORAS_DIA 24
// Cuantas cargas puede tener el negocio (tamano del arreglo de punteros Carga*)
#define MAX_CARGAS 5
// Semilla de rand(): con la misma semilla se repiten los mismos precios
#define SEMILLA 2026
// Franjas horarias del precio de bolsa: madrugada, manana, tarde, pico y noche
#define NUM_FRANJAS 5

// --- Constantes del negocio (tabla de la seccion 3 del proyecto de aula) ---
// kW de la carga critica: los refrigeradores nunca se apagan
const float POTENCIA_REFRIGERADORES = 2.5;
// kW de la aplazable 1
const float POTENCIA_AIRE = 3.5;
// kW de la aplazable 2
const float POTENCIA_CALENTADOR = 2.0;
// kW contratados con el comercializador de energia (regla 6)
const float POTENCIA_CONTRATADA = 7.0;
// COP/kWh: por encima de este precio la energia esta "cara" (regla 4)
const float PRECIO_CARO = 800.0;

// --- Constantes de las reglas ----------------------------------------------
// La hora pico va de las 18 h...
const int PICO_INICIO = 18;
// ...a las 20 h, incluida (regla 3)
const int PICO_FIN = 20;
// %: con los embalses por debajo hay riesgo de racionamiento (regla 2)
const float NIVEL_ALERTA_EMBALSES = 40.0;
// COP/kWh: precio que, con embalses bajos, declara la emergencia (break)
const float PRECIO_EMERGENCIA = 1500.0;
// Horas minimas al dia del aire acondicionado (justificadas en el PDF)
const int HORAS_MIN_AIRE = 8;
// Horas minimas al dia del calentador de agua (justificadas en el PDF)
const int HORAS_MIN_CALENTADOR = 3;
// 1 de cada 20 lecturas del precio falla (continue)
const int FALLA_CADA = 20;

// --- Constantes de las lecturas --------------------------------------------
// % real de los embalses en septiembre de 2026 (proyecto de aula, seccion 1)
const float EMBALSES_INICIAL = 78.0;
// Valores que la opcion 1 acepta
const int HORA_MINIMA = 0;
const int HORA_MAXIMA = 23;
const float EMBALSES_MINIMO = 0.0;
const float EMBALSES_MAXIMO = 100.0;
// Lectura de ejemplo, antes de que el usuario escriba la suya
const int HORA_EJEMPLO = 12;
const float PRECIO_EJEMPLO = 500.0;

// --- Precios reales de bolsa por franja (XM, 21 al 27 de septiembre de 2026) -
// Madrugada, manana y mediodia, tarde, hora pico y noche
// Ultima hora de cada franja
const int FRANJA_HORA_FIN[NUM_FRANJAS] = {6, 14, 17, 20, 23};
// Precio minimo de cada franja (COP/kWh)
const int FRANJA_PRECIO_MIN[NUM_FRANJAS] = {398, 456, 596, 705, 565};
// Precio maximo de cada franja (COP/kWh)
const int FRANJA_PRECIO_MAX[NUM_FRANJAS] = {640, 970, 1040, 1612, 970};

// --- Constantes de presentacion --------------------------------------------
// Letras del nombre de cada carga que caben en una columna de la tabla
const int LETRAS_COLUMNA = 10;

// =============================================================
// Carga: clase base ABSTRACTA con lo comun a todas las cargas
// =============================================================
class Carga {
// protected: lo ven esta clase y sus hijas, pero no el resto del programa
protected:
    string nombre;
    float potenciaKW;
    bool encendida;
    int horasUso;

public:
    Carga(string n, float p) {
        nombre = n;
        potenciaKW = p;
        encendida = false;
        horasUso = 0;
    }

    // Destructor virtual: necesario en una clase base con metodos virtuales
    virtual ~Carga() {}

    // POLIMORFISMO: metodo virtual puro. Carga no lo implementa; cada hija pone su regla.
    // Por el "= 0", Carga es abstracta: no se pueden crear objetos Carga
    virtual bool debeEncender(int hora, float precio, float embalses) = 0;

    // Por defecto una carga no se aplaza ni tiene horas minimas.
    // CargaAplazable reemplaza estos dos metodos
    virtual bool esAplazable() { return false; }
    virtual int getHorasMinimas() { return 0; }

    void encender() { encendida = true; }
    void apagar() { encendida = false; }

    // Cierra la hora: si la carga termino encendida, suma una hora de uso.
    // Se llama despues de las reglas, porque la regla 6 puede apagarla a ultimo momento
    void registrarHora() {
        if (encendida) horasUso++;
    }

    // Empieza un dia nuevo: asi cada simulacion cuenta solo sus horas
    void reiniciarDia() {
        encendida = false;
        horasUso = 0;
    }

    // true si todavia no completa sus horas minimas del dia
    bool faltanHoras() { return horasUso < getHorasMinimas(); }

    // Energia gastada en el dia: kW x horas = kWh
    float kwhUsados() { return potenciaKW * horasUso; }

    // Getters: leen los atributos protegidos sin poder cambiarlos
    string getNombre() { return nombre; }
    float getPotencia() { return potenciaKW; }
    bool estaEncendida() { return encendida; }
    int getHorasUso() { return horasUso; }
};

// =============================================================
// HERENCIA: CargaCritica ES UNA Carga. Nunca se apaga
// =============================================================
class CargaCritica : public Carga {
public:
    CargaCritica(string n, float p) : Carga(n, p) {}

    // Regla 1: siempre encendida. Recibe los datos aunque no los use,
    // porque debe tener la misma forma que debeEncender de Carga
    bool debeEncender(int hora, float precio, float embalses) override {
        (void)hora; (void)precio; (void)embalses;
        return true;
    }
};

// =============================================================
// HERENCIA: CargaAplazable ES UNA Carga. Espera a que la energia convenga
// =============================================================
class CargaAplazable : public Carga {
private:
    // Horas que debe funcionar cada dia para cumplir con la operacion
    int horasMinimas;

public:
    CargaAplazable(string n, float p, int minimas) : Carga(n, p) {
        horasMinimas = minimas;
    }

    // Reglas 2 a 5 en UNA condicion compuesta: se aplaza si los embalses estan bajos (2),
    // si es hora pico (3) o si el precio supera el precio caro (4).
    // ! niega todo: si no hay razon para aplazar, se enciende (5)
    bool debeEncender(int hora, float precio, float embalses) override {
        return !(embalses < NIVEL_ALERTA_EMBALSES ||
                 (hora >= PICO_INICIO && hora <= PICO_FIN) ||
                 precio > PRECIO_CARO);
    }

    bool esAplazable() override { return true; }
    int getHorasMinimas() override { return horasMinimas; }
};

// Prueba de las cargas: la misma pregunta, dos respuestas distintas
int main() {
    CargaCritica refrigeradores("Refrigeradores", POTENCIA_REFRIGERADORES);
    CargaAplazable aire("Aire acondicionado", POTENCIA_AIRE, HORAS_MIN_AIRE);
    CargaAplazable calentador("Calentador de agua", POTENCIA_CALENTADOR, HORAS_MIN_CALENTADOR);
    Carga* cargas[] = {&refrigeradores, &aire, &calentador};

    for (Carga* carga : cargas) {
        cout << carga->getNombre() << " a las 19 h: " << carga->debeEncender(19, 500, EMBALSES_INICIAL) << endl;
    }
    return 0;
}
