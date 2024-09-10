#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>

#define N_ATOMS_INIT 10
#define duration 5
#define energy_explode 100000
#define min_split_atomic 10
#define min_atomic_number 50
#define max_atomic_number 100

//================================================================
void generateAtoms();
void printReport();
void masterFunc();
void atomProcessFunc(void * );
void activatorFunc();
void feederFunc();
void manageNewAtom(void *);
//================================================================
int atomes[N_ATOMS_INIT];
int wasteAtomes[N_ATOMS_INIT];
_Bool finish = false;
sem_t permit, split, start, stop, semWait;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
int waste = 0;
int counter = 0;
int demandEnergy = 0;
int energy = 50000;
int energyGenerateStep[duration], energyDemandStep[duration];
//================================================================
void generateAtoms() {
    for (int i = 0; i < N_ATOMS_INIT; ++i) {
        int random_atom = rand() % (max_atomic_number - min_atomic_number + 1) + min_atomic_number;
        sem_wait(&semWait);
        atomes[i] = random_atom;
        sem_post(&semWait);
    }
}
//================================================================
void printReport() {
    int index = 0;
    while (!finish) {
        sleep(1);
        counter++;
        energyGenerateStep[index] = energy;
        energyDemandStep[index] = demandEnergy;
        for (int j = 0; j < N_ATOMS_INIT; j++)
            waste += wasteAtomes[j];
        if (counter == duration) {
            printf("\nTotal energy generated: %d Energy generated in time%d: %d",
                   energy, index + 1, abs(energyGenerateStep[index] - energyGenerateStep[index - 1]));
            printf("\nTotal energy Demanded: %d Energy Demanded in time%d: %d",
                   demandEnergy, index + 1, abs(energyDemandStep[index] - energyGenerateStep[index - 1]));
            printf("\nWaste = %d", waste);
            finish = true;
            printf("\nTime Over ...");
            exit(0);
        }
        if (index == 0) {
            printf("\nEnergy generated in time%d: %d", index + 1, energyGenerateStep[index]);
            printf("\nEnergy Demanded in time%d: %d", index + 1, energyDemandStep[index]);
            printf("\nWaste = %d", waste);
        } else {
            printf("\nTotal energy generated: %d Energy generated in time%d: %d",
                   energy, index + 1, abs(energyGenerateStep[index] - energyGenerateStep[index - 1]));
            printf("\nTotal energy Demanded: %d Energy Demanded in time%d: %d",
                   demandEnergy, index + 1, abs(energyDemandStep[index] - energyDemandStep[index - 1]));
            printf("\nWaste = %d", waste);
        }
        index++;
    }
    return;
}
//================================================================
void masterFunc() {
    generateAtoms();

    pthread_t reportThread, activatorThread, feederThread;
    pthread_t atomProcessThread[N_ATOMS_INIT];

    sem_init(&permit, 0, 0);
    sem_init(&split, 0, 0);
    sem_init(&start, 0, 0);
    sem_init(&stop, 0, 0);
    sem_init(&semWait, 0, 1);

    pthread_create(&reportThread, NULL, (void *)printReport, NULL);
    pthread_create(&activatorThread, NULL, (void *)activatorFunc, NULL);
    pthread_create(&feederThread, NULL, (void *)feederFunc, NULL);

    while (!finish) {
        usleep(100000);
        pthread_mutex_lock(&m);
        if (atomes[0] == 0) {
            pthread_cond_signal(&cv);
            pthread_mutex_unlock(&m);
            sem_wait(&stop);
        } else {
            pthread_mutex_unlock(&m);
        }

        int i = 0;
        for (; i < N_ATOMS_INIT; ++i) {
            if (atomes[i] != 0) {
                int *temp= malloc(sizeof (int));
                *temp=atomes[i];
                pthread_create(&atomProcessThread[i], NULL, (void *)atomProcessFunc, temp);
                atomes[i] = 0;
            } else {
                break;
            }
        }

        for (int j = 0; j < i; ++j) {
            pthread_join(atomProcessThread[j], NULL);
        }
    }

    pthread_join(reportThread, NULL);
    pthread_join(activatorThread, NULL);
    pthread_join(feederThread, NULL);
    return;
}
//================================================================
void atomProcessFunc(void *input) {
    int atom =*((int *) input);
    sem_wait(&semWait);
    demandEnergy += 1000;// 1000 for normal execution and times up, 900 for explosion , 1500 for lack of energy
    energy -= 1000;
    if (finish) {
        sem_post(&semWait);
        return;
    }
    if (energy <= 0) {
        finish = true;
        printf("\nLack of sufficient energy ...(%d)", energy);
        exit(0);
    }
    sem_post(&semWait);

    sem_post(&permit);
    sem_wait(&split);

    int x = atom / 2;
    int y = atom - x;

    sem_wait(&semWait);
    energy += ((x * y) - ((x > y) ? x : y));
    if (energy > energy_explode) {
        finish = true;
        printf("\nGenerated too much energy caused explosion ...(%d)", energy);
        exit(0);
    }
    sem_post(&semWait);

    pthread_t child1, child2;
    int *xx= malloc(sizeof(int));
    *xx=x;
    int *yy= malloc(sizeof(int));
    *yy=y;
    pthread_create(&child1, NULL, (void *)manageNewAtom, xx);
    pthread_create(&child2, NULL, (void *)manageNewAtom, yy);

    pthread_join(child1, NULL);
    pthread_join(child2, NULL);
}
//================================================================
void manageNewAtom(void *input) {
    int atom =*((int *) input);
    sem_wait(&semWait);
    if (atom > min_split_atomic)
        atomes[0] = atom;
    else
        wasteAtomes[0] = atom;
    sem_post(&semWait);
}
//================================================================
void activatorFunc() {
    while (!finish) {
        sem_wait(&permit);
        //do some operations then make a decision
        sem_post(&split);
        // emit the permission
    }
}
//================================================================
void feederFunc() {
    while (!finish) {
        pthread_mutex_lock(&m);
        while (atomes[0] != 0) {
            pthread_cond_wait(&cv, &m);
        }
        pthread_mutex_unlock(&m);

        generateAtoms();
        printf("\nFeeder generated new atoms ...");
        sem_post(&stop);
    }
}
//================================================================
int main(int argc, char *argv[]) {
    pthread_t mainThread;
    pthread_create(&mainThread, NULL, (void *)masterFunc, NULL);
    pthread_join(mainThread, NULL);
    return 0;
}
