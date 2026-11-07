// Ejecuta wattcontrol.exe con las entradas de cada prueba, dato por dato,
// y guarda en esta carpeta lo que se vio en pantalla (salida + lo que se escribio).
// Uso, desde la carpeta del proyecto:
//   g++ -std=c++17 -Wall -Wextra wattcontrol.cpp -o wattcontrol.exe
//   node pruebas/ejecutar_pruebas.js
const { spawn } = require('child_process');
const fs = require('fs');
const path = require('path');

const exe = path.join(__dirname, '..', 'wattcontrol.exe');

// Cada linea del arreglo es lo que el usuario escribe ('' = solo Enter)
const pruebas = {
    p01_invalidos: ['1', '25', '-1', 'abc', '7.5', '14', '-5', '0', 'abc', '500', '150', '-1', '78', '', '0'],
    p02_pico: ['1', '19', '500', '78', '', '2', '', '0'],
    p03_embalses: ['1', '10', '500', '35', '', '2', '', '0'],
    p04_potencia: ['1', '10', '500', '78', '', '2', '', '0'],
    p05_precio_caro: ['1', '10', '801', '78', '', '2', '', '1', '10', '800', '78', '', '2', '', '0'],
    p06_simulacion: ['3', '', '4', '', '0'],
    p07_continue: ['3', '', '3', '', '4', '', '0'],
    p08_break: ['1', '12', '500', '35', '', '3', '', '3', '', '4', '', '0'],
    p09_menu: ['4', '', '9', '', 'x', '', '0'],
};

function ejecutar(nombre, entradas) {
    return new Promise((resolve) => {
        const programa = spawn(exe);
        let pantalla = '';
        let espera = null;
        let i = 0;
        programa.stdout.on('data', (datos) => {
            pantalla += datos.toString();
            clearTimeout(espera);
            // Cuando el programa deja de escribir, esta esperando un dato
            espera = setTimeout(() => {
                if (i < entradas.length) {
                    pantalla += entradas[i] + '\n';
                    programa.stdin.write(entradas[i] + '\n');
                    i++;
                }
            }, 40);
        });
        programa.on('close', (codigo) => {
            clearTimeout(espera);
            fs.writeFileSync(path.join(__dirname, nombre + '.txt'), pantalla);
            console.log(nombre, 'codigo de salida', codigo);
            resolve();
        });
    });
}

(async () => {
    for (const nombre of Object.keys(pruebas)) {
        await ejecutar(nombre, pruebas[nombre]);
    }
})();
