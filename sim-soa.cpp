#include <iostream>
#include <vector>
#include <math.h>
#include <algorithm>
#include <fstream>
#include <string>
#include <random>

using namespace std;

struct objetos
{
  vector<double> p[3];
  vector<double> v[3];
  vector<double> m;
} ;

void fuerzaGravitatoria(double x1, double y1, double z1, double m1, double x2, double y2, double z2, double m2, &vector<double> resultado)
{
    //Si ambos objetos son los mismos entonces devolver 0
    if((x1==x2)&&(y1==y2)&&(z1==z2)&&(m1==m2))
        return 0;
    //Constante de gravitacion universal
    double G = 6.674e-11;
    //Calculo de la parte escalar (G*m1*m2)/||pj-pi||^3
    double pEscalar = (G*m1*m2)/pow(pow(x2-x1, 2.0) + pow(y2-y1, 2.0) + pow(z2-z1, 2.0), 3.0/2.0);
    //Calculo del vector resultado
    resultado.push_back((x2-x1)*pEscalar);
    resultado.push_back((y2-y1)*pEscalar);
    resultado.push_back((z2-z1)*pEscalar);
}

void vectorAceleracion(double x1, double y1, double z1, double m1, &struct objetos O, &vector<double> resultado)
{
    //bucle for homologo al sumatorio de la formula
    for(int i = 0; i < O.p[0].size(); i++){
        //Se calcula la fuerza que ejerce uno sobre el otro en forma vectorial
        vector<double> fuerza;
        fuerzaGravitatoria(x1, y1, z1, m1, O.p[0][i], O.p[1][i], O.p[2][i], O.m[i], fuerza)
        //Se añade al resultado final, como indica el sumatorio
        transform(resultado.begin(), resultado.end(), fuerza.begin(), resultado.begin(), plus<double>())
    }
    //Se reescala por 1/m1
    double k = 1.0/m1
    transform(resultado.begin(), resultado.end(), resultado.begin(), [k](double &c){ return c/k; });
}

void colisiones(&struct objetos universo)
{
    for (int j = 0; j < universo.p[0].size(); j++){
        for (int k = 0; k < universo.p[0].size(); k++){
            if((j != k)&&((pow(universo.p[0][k]-universo.p[0][j], 2.0) + pow(universo.p[1][k]-universo.p[1][j], 2.0) + pow(universo.p[2][k]-universo.p[2][j], 2.0)) < 1)){//Si 2 objetos son distintos y colisionan
                universo.m[j] += universo.m[j]; //Se suman sus masas
                //Se suman las velocidades y se destruye el segundo objeto
                for(int l = 0; l < 3; l++){
                    //Suma de velocidades
                    universo.v[l][j] += universo.v[l][j];
                    //destruccion del segundo objeto
                    universo.p[l].erase(universo.p[l].begin()+k);
                    universo.v[l].erase(universo.v[l].begin()+k);
                    universo.m.erase(universo.m.begin()+k);
                }
            }
        }
    }
}

void guardarDatos(&struct objetos datos, string archivo, double size_enclosure, double time_step, int num_objects)
{
    ofstream Resultados(archivo);

    //Definicion de num decimales a 3
    Resultados << fixed << setprecision(3);

    //Cabecera
    Resultados << size_enclosure << " " << time_step << " " << num_objects << endl;

    //Cuerpo
    for (int i = 0; i < datos.p[0].size(); i++){
        //Posiciones
        for(int j = 0; j < 3; j++){
            Resultados << datos.p[j][i] << " ";
        }
        //Velocidades
        for(int j = 0; j < 3; j++){
            Resultados << datos.v[j][i] << " ";
        }
        //Masa
        Resultados << datos.m[i] << endl;
    }

    //se cierra el archivo
    Resultados.close();
}

void simulation(int num_objects, int num_iterations, int random_seed, double size_enclosure, double time_step)
{
    //Se inicializan los objetos de la simulation
    //random_device rd; //Se usa para obtener una semilla aleatoria con la que inicializar gen64
    mt19937_64 gen64(random_seed); //Generador de numeros pseudoaleatorios
    uniform_real_distribution<> disPos(0.0, size_enclosure); //Distribucion para generar las posiciones
    normal_distribution<> disMas{10e21, 10e15}; //Distribucion para generar las velocidades

    //elementos
    struct objetos universo;

    /*
    //Cambiar la semilla
    uint64_t newSeed = ;
    gen64.seed(newSeed);
    */

    //se generan de forma aleatoria los elementos
    for(int i = 0; i < num_objects; i++){
        //posiciones y velocidades
        for(int j = 0; j < 3; j++){
            universo.p[j].push_back(disPos(gen64));//Se inicializan las posiciones de forma aleatoria
            universo.v[j].push_back(0.0);//Se inicializan a 0 las velocidades
        }
        universo.m.push_back(disMas(gen64));//Se inicializan las masas de forma aleatoria
    }

    //comprobar si hay colisiones
    colisiones(universo);

    //Almacenar configuracion inicial
    guardarDatos(universo, "init_config.txt", size_enclosure, time_step, num_objects);

    //bucle de la simulation
    for(int i = 0; i < num_iterations; i++){
        //se crea la siguiente iteracion
        struct objetos newIt;
        for (int j = 0; j < universo.p[0].size(); j++){
            //Calculo del vector aceleracion
            vector<double> aceleracion(0.0, 0.0, 0.0);
            vectorAceleracion(universo.p[0][j], universo.p[1][j], universo.p[2][j], universo.m[j], universo, aceleracion);
            //Calculo de las nuevas velocidades y posiciones en los 3 ejes
            for(int k = 0; k < 3; k++){
                newIt.v[k].push_back(universo.v[k][j] + aceleracion[k]*time_step);
                newIt.p[k].push_back(universo.p[k][j] + newIt.v[k][j]*time_step);
                //Calculo de rebotes
                if(newIt.p[k][j] < 0.0){
                    newIt.p[k][j] = 0.0;
                    newIt.v[k][j] *= -1.0;
                }
                if(newIt.p[k][j] > size_enclosure){
                    newIt.p[k][j] = size_enclosure;
                    newIt.v[k][j] *= -1.0;
                }
            }
            //Cargar las masas en la nueva iteracion
            newIt.m[j] = universo.m[j];
        }
        //Calculo de colisiones
        colisiones(newIt);

        //Actualizacion del universo
        universo = newIt;
    }
    //Almacenar configuracion final
    guardarDatos(universo, "final_config.txt", size_enclosure, time_step, num_objects);
}

int main(int argc, char** argv)
{
    cout << "sim-soa invoked with " << argc << "parameters." << endl;

    string parametros[5] = ["num_objects", "num_iterations", "random_seed", "size_enclosure", "time_step"];

    for(int i = 0; i < 5; i++){
        cout << parametros[i] << ": ";
        if(i < argc)
            cout << argv[i] << endl;
        else
            cout << "?" << endl;
    }

    if(argc != 5){
        perror("Error: Numero de comandos incorrecto");
        return -1;
    }

    if(argv[0] <= 0 && argv[1] <= 0 && argv[3] <= 0 && argv[4] <= 0){
        perror("Error: Algun argumento invalido");
        return -2;
    }

    simulation(argv[0], argv[1], argv[2], argv[3], argv[4]);

    return 0;
}
