# WattControl: minimercado

Gestor de energía en C++ para un minimercado ante El Niño 2026. Decide, hora por hora, qué cargas encender o aplazar según la hora, el precio de bolsa y el nivel de los embalses, y compara el costo con gestor y sin gestor.

Evidencia 2 de Programación Orientada a Objetos (PREICA2602B020043), IU Digital de Antioquia, 2026-2.

- **Integrante:** Camilo Botero Ortiz
- **Negocio:** minimercado (último dígito del documento: 8)

## Compilar y ejecutar

Con g++ de MSYS2, desde la terminal de VS Code:

```bash
g++ -std=c++17 -Wall -Wextra wattcontrol.cpp -o wattcontrol.exe
./wattcontrol.exe
```

## Datos del negocio

| Carga | Tipo | Potencia | Horas mínimas |
|---|---|---|---|
| Refrigeradores | Crítica | 2,5 kW | Todo el día |
| Aire acondicionado | Aplazable | 3,5 kW | 8 h |
| Calentador de agua | Aplazable | 2,0 kW | 3 h |

Potencia contratada: 7 kW. Precio caro: más de 800 COP/kWh.

## Clases y relaciones

| Clase | Relación |
|---|---|
| `Carga` | Clase base abstracta, con `debeEncender()` virtual puro |
| `CargaCritica`, `CargaAplazable` | Herencia de `Carga` y polimorfismo |
| `Medidor` | Composición: es parte del `Negocio` |
| `Mercado` | Asociación: el `Negocio` lo conoce y le pide datos |
| `Negocio` | Agregación con `Carga`: las cargas se crean afuera |

El diagrama está en [docs/diagrama_clases.png](docs/diagrama_clases.png).

## Regla 6 y turno por horas mínimas

Las tres cargas suman 8 kW, más que los 7 kW contratados, y las dos aplazables siempre piden encender en las mismas horas. Si la regla 6 actuara sola, el aire nunca funcionaría. Por eso `Negocio::cederTurno()` actúa antes: la aplazable que ya cumplió sus horas mínimas cede el turno a la que no las ha cumplido.

## Pruebas

```bash
node pruebas/ejecutar_pruebas.js
```

El script ejecuta las 9 pruebas y guarda la salida de cada una en `pruebas/`. Con la semilla 2026, la primera simulación da $68.791 con gestor, $153.552 sin gestor y un ahorro de $84.761 (55,2 %).

El informe de la entrega es [Botero_Camilo_EA2_WattControl.pdf](Botero_Camilo_EA2_WattControl.pdf).
