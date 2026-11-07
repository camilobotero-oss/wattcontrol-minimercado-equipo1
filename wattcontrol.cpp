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

// =============================================================
// Medidor: parte del Negocio (COMPOSICION). Acumula kWh y costo
// =============================================================
class Medidor {
private:
    // Lo que gasta el negocio con el gestor
    float kwhConGestor, costoConGestor;
    // Lo que gastaria en esas mismas horas sin el gestor
    float kwhSinGestor, costoSinGestor;

public:
    Medidor() { reiniciar(); }

    void reiniciar() {
        kwhConGestor = 0; costoConGestor = 0;
        kwhSinGestor = 0; costoSinGestor = 0;
    }

    // Suma una hora: los kW encendidos durante 1 h son kWh; el costo es kWh x precio
    void registrarConGestor(float kwEncendidos, float precio) {
        kwhConGestor += kwEncendidos;
        costoConGestor += kwEncendidos * precio;
    }

    void registrarSinGestor(float kwTotales, float precio) {
        kwhSinGestor += kwTotales;
        costoSinGestor += kwTotales * precio;
    }

    float getKwhConGestor() { return kwhConGestor; }
    float getCostoConGestor() { return costoConGestor; }
    float getKwhSinGestor() { return kwhSinGestor; }
    float getCostoSinGestor() { return costoSinGestor; }
};

// =============================================================
// Mercado: da el precio de cada hora y el nivel de los embalses (ASOCIACION)
// =============================================================
class Mercado {
private:
    // % de los embalses del pais
    float nivelEmbalses;

public:
    Mercado(float nivelInicial) {
        nivelEmbalses = nivelInicial;
    }

    // Precio al azar dentro del rango real de la franja de esta hora
    float precioHora(int hora) {
        // Busca la primera franja que contiene la hora
        int franja = 0;
        while (hora > FRANJA_HORA_FIN[franja]) {
            franja++;
        }
        int minimo = FRANJA_PRECIO_MIN[franja];
        int maximo = FRANJA_PRECIO_MAX[franja];
        // rand() % (maximo - minimo + 1) da de 0 a la diferencia; sumado al minimo,
        // el precio queda entre minimo y maximo
        return minimo + rand() % (maximo - minimo + 1);
    }

    float getNivelEmbalses() { return nivelEmbalses; }

    // La opcion 1 actualiza el dato: asi se pueden simular dias de embalses bajos
    void setNivelEmbalses(float nivel) { nivelEmbalses = nivel; }
};

// =============================================================
// Negocio: reune todo y simula el dia
// =============================================================
class Negocio {
private:
    string nombre;
    // COMPOSICION: el medidor es un atributo objeto. Nace y muere con el negocio
    Medidor medidor;
    // AGREGACION: arreglo de punteros. Las cargas se crean afuera y aqui se guarda su direccion
    Carga* cargas[MAX_CARGAS];
    int numCargas;
    // ASOCIACION: el negocio no crea el mercado; solo lo conoce y le pide datos
    Mercado* mercado;

    // Resultados de la ultima simulacion, para el resumen (opcion 4)
    int horasSimuladas, fallas, alertasPotencia, turnosCedidos;
    bool emergencia;
    int horaEmergencia;

    // kW de todas las cargas: lo que consume el negocio sin gestor
    float potenciaInstalada() {
        float total = 0;
        for (int i = 0; i < numCargas; i++) {
            total += cargas[i]->getPotencia();
        }
        return total;
    }

    // kW de las cargas encendidas en este momento
    float potenciaEncendida() {
        float total = 0;
        for (int i = 0; i < numCargas; i++) {
            if (cargas[i]->estaEncendida()) total += cargas[i]->getPotencia();
        }
        return total;
    }

    // POLIMORFISMO: el mismo ciclo le pregunta a todas las cargas; cada una usa su regla
    void decidirCargas(int hora, float precio, float embalses) {
        for (int i = 0; i < numCargas; i++) {
            if (cargas[i]->debeEncender(hora, precio, embalses)) {
                cargas[i]->encender();
            } else {
                cargas[i]->apagar();
            }
        }
    }

    // continue: sin precio no se decide nada; solo sigue lo que no es aplazable
    void dejarSoloCritica() {
        for (int i = 0; i < numCargas; i++) {
            if (cargas[i]->esAplazable()) {
                cargas[i]->apagar();
            } else {
                cargas[i]->encender();
            }
        }
    }

    // Turno por horas minimas (decision del equipo, explicada en el PDF).
    // Las tres cargas suman mas que la potencia contratada, asi que las dos aplazables
    // nunca caben juntas. Si la regla 6 actuara sola, apagaria siempre la mayor y esa carga
    // nunca funcionaria. Por eso, antes de la regla 6, la aplazable que ya cumplio sus horas
    // cede el turno a la que todavia no. Devuelve la carga que cedio, o nullptr si ninguna
    Carga* cederTurno() {
        if (potenciaEncendida() <= POTENCIA_CONTRATADA) return nullptr;

        // Hay alguna aplazable encendida a la que le falten horas?
        bool hayPendiente = false;
        for (int i = 0; i < numCargas; i++) {
            if (cargas[i]->estaEncendida() && cargas[i]->faltanHoras()) hayPendiente = true;
        }
        if (!hayPendiente) return nullptr;

        for (int i = 0; i < numCargas; i++) {
            if (cargas[i]->esAplazable() && cargas[i]->estaEncendida() && !cargas[i]->faltanHoras()) {
                cargas[i]->apagar();
                return cargas[i];
            }
        }
        return nullptr;
    }

    // Regla 6: si se supera la potencia contratada, apaga la aplazable de mayor potencia.
    // Devuelve la carga que apago, o nullptr si no hizo falta
    Carga* aplicarRegla6() {
        if (potenciaEncendida() <= POTENCIA_CONTRATADA) return nullptr;

        Carga* mayor = nullptr;
        for (int i = 0; i < numCargas; i++) {
            if (cargas[i]->esAplazable() && cargas[i]->estaEncendida()) {
                if (mayor == nullptr || cargas[i]->getPotencia() > mayor->getPotencia()) {
                    mayor = cargas[i];
                }
            }
        }
        if (mayor != nullptr) mayor->apagar();
        return mayor;
    }

    // Cierra la hora: cuenta las horas de uso y el Medidor acumula con y sin gestor.
    // Devuelve el costo de esta hora con gestor
    float cerrarHora(float precio) {
        for (int i = 0; i < numCargas; i++) {
            cargas[i]->registrarHora();
        }
        float kwEncendidos = potenciaEncendida();
        medidor.registrarConGestor(kwEncendidos, precio);
        medidor.registrarSinGestor(potenciaInstalada(), precio);
        horasSimuladas++;
        return kwEncendidos * precio;
    }

    // Muestra la alerta de cada aplazable que no cumplio sus horas minimas
    void mostrarProduccionIncompleta() {
        for (int i = 0; i < numCargas; i++) {
            if (cargas[i]->faltanHoras()) {
                cout << "ALERTA: produccion incompleta - " << cargas[i]->getNombre() << ": "
                     << cargas[i]->getHorasUso() << " h de " << cargas[i]->getHorasMinimas()
                     << " h minimas" << endl;
            }
        }
    }

public:
    // ": medidor()" crea el medidor junto con el negocio (composicion)
    Negocio(string n, Mercado* m) : medidor() {
        nombre = n;
        mercado = m;
        numCargas = 0;
        horasSimuladas = 0; fallas = 0; alertasPotencia = 0; turnosCedidos = 0;
        emergencia = false;
        horaEmergencia = 0;
    }

    // Agrega una carga creada afuera (agregacion)
    void agregarCarga(Carga* c) {
        if (numCargas < MAX_CARGAS) {
            cargas[numCargas] = c;
            numCargas++;
        }
    }

    void mostrar() {
        cout << fixed << setprecision(1);
        cout << nombre << " | potencia contratada: " << POTENCIA_CONTRATADA << " kW" << endl;
        for (int i = 0; i < numCargas; i++) {
            cout << "  " << cargas[i]->getNombre() << " (" << cargas[i]->getPotencia() << " kW, "
                 << (cargas[i]->esAplazable() ? "aplazable" : "critica") << ")" << endl;
        }
    }

    // Opcion 2: que haria cada carga con una lectura (hora, precio, embalses)
    void evaluar(int hora, float precio, float embalses) {
        decidirCargas(hora, precio, embalses);

        // Lo que pidio cada carga, antes de revisar la potencia contratada
        for (int i = 0; i < numCargas; i++) {
            cout << "  " << setw(22) << left << cargas[i]->getNombre() << right
                 << (cargas[i]->estaEncendida() ? "ENCENDER" : "APLAZAR") << endl;
        }

        // El motivo de la decision
        if (embalses < NIVEL_ALERTA_EMBALSES) {
            cout << "ALERTA: riesgo de racionamiento: solo funciona la carga critica" << endl;
        }
        if (hora >= PICO_INICIO && hora <= PICO_FIN) {
            cout << "Hora pico: se aplazan las cargas aplazables" << endl;
        }
        if (precio > PRECIO_CARO) {
            cout << "Energia cara: se aplazan las cargas aplazables" << endl;
        }

        // Regla 6
        cout << fixed << setprecision(1);
        float potenciaPedida = potenciaEncendida();
        Carga* apagada = aplicarRegla6();
        if (apagada != nullptr) {
            cout << "ALERTA: " << potenciaPedida << " kW superan la potencia contratada ("
                 << POTENCIA_CONTRATADA << " kW). Se apaga " << apagada->getNombre() << endl;
        }
        cout << "Potencia encendida: " << potenciaEncendida() << " kW de "
             << POTENCIA_CONTRATADA << " kW contratados" << endl;
    }

    // Opcion 3: simula 24 horas con gestor y calcula el costo sin gestor
    void simularDia() {
        // Todo empieza de nuevo: asi el resumen cuenta solo esta simulacion
        medidor.reiniciar();
        for (int i = 0; i < numCargas; i++) {
            cargas[i]->reiniciarDia();
        }
        horasSimuladas = 0; fallas = 0; alertasPotencia = 0; turnosCedidos = 0;
        emergencia = false;

        cout << fixed << setprecision(0);
        if (mercado->getNivelEmbalses() < NIVEL_ALERTA_EMBALSES) {
            cout << "ALERTA: embalses al " << mercado->getNivelEmbalses()
                 << " %: riesgo de racionamiento, solo funciona la carga critica" << endl;
        }

        // Encabezado: una columna por carga, con su nombre recortado
        cout << setw(5) << "Hora" << setw(8) << "Precio" << setw(6) << "Emb%";
        for (int i = 0; i < numCargas; i++) {
            cout << setw(12) << cargas[i]->getNombre().substr(0, LETRAS_COLUMNA);
        }
        cout << setw(6) << "kW" << setw(8) << "Costo" << endl;

        for (int h = 0; h < HORAS_DIA; h++) {
            // ASOCIACION: el negocio le pide los datos de esta hora al mercado
            float precio = mercado->precioHora(h);
            float embalses = mercado->getNivelEmbalses();

            // continue: fallo la lectura del precio. El controlador no lo conoce, asi que
            // solo deja la carga critica; la factura de esa hora si se paga al precio real
            if (rand() % FALLA_CADA == 0) {
                dejarSoloCritica();
                cerrarHora(precio);
                fallas++;
                cout << setw(5) << h << setw(8) << "--" << setw(6) << embalses
                     << "  falla de lectura: solo la carga critica" << endl;
                continue;
            }

            // break: energia en escasez y embalses bajos a la vez. Termina la simulacion
            if (precio > PRECIO_EMERGENCIA && embalses < NIVEL_ALERTA_EMBALSES) {
                emergencia = true;
                horaEmergencia = h;
                cout << setw(5) << h << setw(8) << precio << setw(6) << embalses
                     << "  EMERGENCIA DE RACIONAMIENTO: termina la simulacion" << endl;
                break;
            }

            // Cada carga decide, y despues se revisa la potencia contratada
            decidirCargas(h, precio, embalses);
            Carga* cedio = cederTurno();
            if (cedio != nullptr) turnosCedidos++;
            Carga* apagada = aplicarRegla6();
            if (apagada != nullptr) alertasPotencia++;

            float kwEncendidos = potenciaEncendida();
            float costoHora = cerrarHora(precio);

            cout << setw(5) << h << setw(8) << precio << setw(6) << embalses;
            for (int i = 0; i < numCargas; i++) {
                cout << setw(12) << (cargas[i]->estaEncendida() ? "ON" : "off");
            }
            cout << setw(6) << setprecision(1) << kwEncendidos
                 << setw(8) << setprecision(0) << costoHora;
            if (apagada != nullptr) cout << "  ALERTA potencia: apaga " << apagada->getNombre();
            if (cedio != nullptr) cout << "  turno: cede " << cedio->getNombre();
            cout << endl;
        }

        cout << "Simulacion terminada. Vea el resumen con la opcion 4." << endl;
        mostrarProduccionIncompleta();
    }

    // Opcion 4: resumen y ahorro de la ultima simulacion
    void resumen() {
        cout << fixed << setprecision(1);
        cout << nombre << endl;
        cout << "Horas simuladas: " << horasSimuladas << " de " << HORAS_DIA
             << " | fallas de lectura: " << fallas << endl;
        cout << "Alertas de potencia: " << alertasPotencia
             << " | turnos cedidos: " << turnosCedidos << endl;
        if (emergencia) {
            cout << "EMERGENCIA DE RACIONAMIENTO a las " << horaEmergencia << " h" << endl;
        }

        for (int i = 0; i < numCargas; i++) {
            cout << "  " << setw(22) << left << cargas[i]->getNombre() << right
                 << setw(3) << cargas[i]->getHorasUso() << " h" << setw(8) << cargas[i]->kwhUsados() << " kWh";
            if (cargas[i]->esAplazable()) {
                cout << "  (minimo " << cargas[i]->getHorasMinimas() << " h)";
            }
            cout << endl;
        }
        cout << "Energia con gestor: " << medidor.getKwhConGestor() << " kWh" << endl;
        cout << "Energia sin gestor: " << medidor.getKwhSinGestor() << " kWh" << endl;

        float costoCon = medidor.getCostoConGestor();
        float costoSin = medidor.getCostoSinGestor();
        cout << setprecision(0);
        cout << "Costo con gestor: $" << costoCon << endl;
        cout << "Costo sin gestor: $" << costoSin << endl;
        cout << "Ahorro:           $" << costoSin - costoCon;
        // Solo si costoSin > 0, para no dividir por 0
        if (costoSin > 0) {
            cout << " (" << setprecision(1) << (costoSin - costoCon) / costoSin * 100 << " %)";
        }
        cout << endl;
        mostrarProduccionIncompleta();
    }
};

// Revisa la lectura que acaba de hacer cin y descarta el resto de la linea.
// Devuelve false si se escribieron letras o sobra algo despues del numero (12abc, 7.5 en la hora)
bool lecturaCorrecta() {
    bool correcta = !cin.fail() && (cin.peek() == '\n' || cin.eof());
    // Se termino la entrada: no hay nada mas que leer
    if (cin.fail() && cin.eof()) {
        cout << endl << "Fin de la entrada." << endl;
        exit(0);
    }
    // clear() quita el error de cin; ignore() bota lo que quedo escrito en esa linea
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    return correcta;
}

// main: el programa empieza a ejecutarse aqui
int main() {
    srand(SEMILLA);

    // El mercado existe por su cuenta: no es parte del negocio
    Mercado mercado(EMBALSES_INICIAL);

    // Las cargas se crean afuera del negocio
    CargaCritica refrigeradores("Refrigeradores", POTENCIA_REFRIGERADORES);
    CargaAplazable aire("Aire acondicionado", POTENCIA_AIRE, HORAS_MIN_AIRE);
    CargaAplazable calentador("Calentador de agua", POTENCIA_CALENTADOR, HORAS_MIN_CALENTADOR);

    // Le pasa la direccion del mercado (asociacion) y le agrega las cargas (agregacion)
    Negocio negocio("Minimercado El Ahorro", &mercado);
    negocio.agregarCarga(&refrigeradores);
    negocio.agregarCarga(&aire);
    negocio.agregarCarga(&calentador);
    negocio.mostrar();

    // Lectura manual: empieza con valores de ejemplo
    int hora = HORA_EJEMPLO;
    float precio = PRECIO_EJEMPLO;
    float embalses = EMBALSES_INICIAL;
    bool haySimulacion = false;
    int opcion;
    // true cuando el dato que acaba de escribir el usuario es valido
    bool valido;

    do {
        cout << endl << "===== WattControl v2 - Minimercado =====" << endl;
        cout << "1. Ingresar lectura manual (hora, precio, embalses)" << endl;
        cout << "2. Evaluar decision de cada carga" << endl;
        cout << "3. Simular un dia completo (24 h)" << endl;
        cout << "4. Ver resumen de la ultima simulacion" << endl;
        cout << "0. Salir" << endl;
        cout << "Opcion: ";
        cin >> opcion;
        // Si escribio letras, la opcion queda en un valor que cae en el default
        if (!lecturaCorrecta()) opcion = -1;

        switch (opcion) {
        case 1:
            do {
                cout << "Hora (" << HORA_MINIMA << " a " << HORA_MAXIMA << "): ";
                cin >> hora;
                valido = lecturaCorrecta() && hora >= HORA_MINIMA && hora <= HORA_MAXIMA;
                if (!valido) cout << "  Hora invalida." << endl;
            } while (!valido);
            do {
                cout << "Precio de bolsa (COP/kWh): ";
                cin >> precio;
                valido = lecturaCorrecta() && precio > 0;
                if (!valido) cout << "  El precio debe ser mayor que 0." << endl;
            } while (!valido);
            do {
                cout << fixed << setprecision(0);
                cout << "Nivel de los embalses (" << EMBALSES_MINIMO << " a " << EMBALSES_MAXIMO << " %): ";
                cin >> embalses;
                valido = lecturaCorrecta() && embalses >= EMBALSES_MINIMO && embalses <= EMBALSES_MAXIMO;
                if (!valido) cout << "  Nivel de embalses invalido." << endl;
            } while (!valido);
            // El mercado guarda el dato: la opcion 3 simula con este nivel de embalses
            mercado.setNivelEmbalses(embalses);
            cout << "Lectura guardada." << endl;
            break;
        case 2:
            cout << fixed << setprecision(0);
            cout << "Hora " << hora << " | precio " << precio
                 << " | embalses " << embalses << " %" << endl;
            negocio.evaluar(hora, precio, embalses);
            break;
        case 3:
            negocio.simularDia();
            haySimulacion = true;
            break;
        case 4:
            if (haySimulacion) negocio.resumen();
            else cout << "Primero simule un dia con la opcion 3." << endl;
            break;
        case 0:
            cout << "Hasta luego." << endl;
            break;
        default:
            cout << "Opcion invalida." << endl;
        }

        // Pausa para leer el resultado antes de volver a mostrar el menu
        if (opcion != 0) {
            cout << endl << "Presione Enter para continuar...";
            cin.get();
        }
    } while (opcion != 0);

    return 0;
}
