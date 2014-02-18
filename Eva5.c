/*El objetivo de esta práctica es familiarizarse con el uso de las primitivas de sincronización (semáforos) 
y los problemas de concurrencia que pueden presentarse (condiciones de carrera, deadlocks) al construir 
aplicaciones multihiladas. También se pretende explorar herramientas de control de versiones y de trabajo en
equipo.

Implemente un programa, en C, considerando

1. El programa se ejecuta así:
$./nombre_programa numeroDeHilos cantidad_tiempo_a_correr numero_cuentas valor_inicial

2. El hilo princial debe crear tantas cuentas como indique el parámetro numero_cuentas. 
El valor inicial de las cuentas estará especificado por valor_inicial.

3. En una cuenta se pueden realizar depósitos, retiros y transferencias.

4. El hilo principal creará la cantidad de hilos definida en numeroDeHilos. 
Cada hilo seleccionará de manera aleatoria dos cuentas y realizará transferencias de una cantidad o monto 
igualmente aleatorio. Cada hilo repetirá esta operación durante la cantidad de tiempo especificada en cantidad_tiempo_a_correr.

5. Al finalizar cantidad_tiempo_a_correr, el hilo principal deberá imprimir el valor de cada una de las cuentas y el balance total
de la sumatoria de todas las cuentas.


NOTAS:

1. Escriba la estrategia de solución INICIAL, explicando la estructura del programa y las ideas para resolver la práctica.
Explique cómo piensa utilizar las primitivas ofrecidas por el sistemas operativo para evitar condiciones de carrera y posibles interbloqueos.

2. Describa los problemas que encontró durante la solución de la práctica mostrando cómo se manifestaban, cómo los detectó y cómo los solucionó.

3. Describa como fue la solución final, explicando la estructura del programa y las ideas finalmente implementadas y funcionales.

4. Compare la solución inicial y la solución final.

5. EL TRABAJO DEBERÁ SER REALIZADO EN PAREJAS!

6. Suba el trabajo realizado a GitHub, incluyendo el código del programa, el programa de prueba y archivos de prueba.

7. TODA LA DOCUMENTACIÓN SOLICITADA deberá estar también en github, utilizando una página de presentación o la wiki.

8. NO SE RECIBIRÁ NADA POR CORREO ELECTRÓNICO.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <wait.h>
#include <string.h>
#include <time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>


sem_t semaforos[10000];
int numHilos;
int tRecorrer;
int numCuentas;
int valorInicial;
int cuentas[10000];


void transferencia()
{
int contador = 0;
int aCuenta1;
int aCuenta2;
int semilla = 2;
//se controla que cada hilo haga la cantidad de tranferencias indicada por el usuario.
	while(contador!=tRecorrer)
	{
		int bandera=0;
		int bandera2=0;
//se controla que las cuentas seleccionadas no sean la misma.
		while(bandera2==0)
		{
			srand(time(NULL)*(int)getpid()*semilla);
			aCuenta1=(int)rand()%(numCuentas);
			aCuenta2=(int)rand()%(numCuentas);
			if(aCuenta1!=aCuenta2)
			{
				bandera2=1;
			}
		}
		contador++;
//se accede a los semaforos de las cuentas y se hace la transferencia.
		while(bandera==0)
		{
			int intento1= sem_trywait(&semaforos[aCuenta1]);
			int intento2= sem_trywait(&semaforos[aCuenta2]);
//se verifica que ambos semaforos fueron tomados, si esto no ocurre se libera alguno de ellos (si lo tomó)
//y se espera por otra oportunidad
			if((intento1==-1)||(intento2==-1))
			{
				if(intento1==0)
				{
					sem_post(&semaforos[aCuenta1]);
				}
				if(intento2==0)
				{
					sem_post(&semaforos[aCuenta2]);
				}
				semilla++;
				sleep(0.01);
			}
			else
			{
//se puede acceder a ambas cuentas y se procede a realizar la transferencia.
				srand(time(NULL)*(int)getpid()*semilla);
				int valorTrans =(int)rand()%(cuentas[aCuenta1]);
				cuentas[aCuenta1]=((int)cuentas[aCuenta1])-valorTrans;
				cuentas[aCuenta2]=((int)cuentas[aCuenta2])+valorTrans;
				bandera=1;
				semilla++;
//se liberan los semaforos
				sem_post(&semaforos[aCuenta1]);
				sem_post(&semaforos[aCuenta2]);
				sleep(0.01);
			}
			semilla++;
		}
	}
}


int main (int argc, char*argv[])
{
//se leen los parametros ingresados	
char *par1=argv[1];
numHilos= atoi(par1);
char *par2=argv[2];
tRecorrer=atoi(par2);
char *par3=argv[3];
numCuentas=atoi(par3);
char *par4=argv[4];
valorInicial=atoi(par4);
int balanceInicial=0;

int i=0;
//se inicializan las cuentas con el valor indicado por el usuario.
for(i=0;i<numCuentas;i++)
{
	cuentas[i]=valorInicial;
	balanceInicial=balanceInicial+valorInicial;
}
//se inicializan los semaros asociados a cada cuenta.
for(i=0;i<numCuentas;i++)
{
	sem_init(&semaforos[i],0,1);
}

pthread_t hilos[numHilos];
//se crean los hilos y se envian a la función transferencia
for(i=0;i<numHilos;i++)
{
	pthread_create(&hilos[i],NULL,(void *)transferencia,NULL);
}
//se espera por cada uno de los hilos.
for(i=0;i<numHilos;i++)
{
	pthread_join(hilos[i],NULL);
}
//se procede a comparar la sumatoria del saldo final de cada cuenta
	int z=0;
	int balanceFinal=0;
	for(z=0;z<numCuentas;z++)
	{
		balanceFinal=balanceFinal+((int)cuentas[z]);
	}

	if(balanceFinal==balanceInicial)
	{
		
		printf("*************** PASÓ ****************\n\n");
		printf("Sumatoria valores iniciales de las cuentas: %d\n\n",balanceInicial);
		printf("Sumatoria valores finales de las cuentas: %d\n\n",balanceFinal);
		
	}
	else
	{
		printf("************** NO PASÓ ***************");
		printf("Sumatoria valores iniciales de las cuentas: %d\n\n",balanceInicial);
		printf("Sumatoria valores finales de las cuentas: %d\n\n",balanceFinal);
	}
return 0;
}

