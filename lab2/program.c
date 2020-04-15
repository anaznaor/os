/*
 * Jednostavni primjer za generiranje prostih brojeva kori�tenjem
   "The GNU Multiple Precision Arithmetic Library" (GMP)
 */

#include <stdio.h>
#include <time.h>
#include<stdlib.h>
#include <pthread.h>
#include <unistd.h>

#include "slucajni_prosti_broj.h"
#define KRAJ_RADA 1


uint64_t MS[5], ULAZ = 0, IZLAZ = 0, velicina_grupe;
int br=0, ulaz[3], Broj[3], Kraj=0;

void stavi_u_MS (uint64_t broj)
{
    MS[ULAZ] = broj;
    ULAZ = (ULAZ + 1) % 5;
    br++;
    if (br > 5) {
        br--;
        IZLAZ = (IZLAZ + 1) % 5;
    }
}

uint64_t uzmi_iz_MS()
{
    uint64_t broj = MS[IZLAZ];
    if (br > 0) {
        IZLAZ = (IZLAZ + 1) % 5;
        br--;
    }
    return broj;
}


uint64_t zbrckanost(uint64_t x)
{
    //gledamo broj jedinica i nula u blokovima od 6 bita,zatim apsolutnu vrijednost razlike broja jedinica i nula usporedimo sa sljedecom apsolutnom vrijednosti sljedeceg prozora
    //broj jedinica spremamo u brojac1 i brojac2 a broj nula spremamo u br3 i br4 
	//apsolutne vrijednosti njihovih razlika spremamo u varijable razlika 1 i razlika 2 koje nakon toga usporedujemo i ako su razlicite zbrckanost povecavamo za 1
	uint64_t prozor,razlika1=0,razlika2=0, brojac1 = 0, brojac2 = 0,br3=0,br4=0, zbrckanost = 0;

    while(x != 0)
	{
        prozor = x & 0xF;
        brojac1 = 0;
		br3=0;
		while(prozor != 0)
		{
			if(prozor & 1)
				brojac1++;
			prozor = prozor >> 1;
		}
        
        x = x >> 6;
        brojac2 = 0;
		br4=0;
        prozor = x & 0xF;
        while(prozor != 0)
		{
			if(prozor & 1)
				brojac2++;
			prozor = prozor >> 1;
		}
			br3=6-brojac1;
			br4=6-brojac2;
			razlika1=abs(brojac1-br3);
			razlika2=abs(brojac2-br4);

			if(razlika1!=razlika2)
            zbrckanost++;
    }
    return zbrckanost;
}

uint64_t generiraj_dobar_broj(uint64_t velicina_grupe, struct gmp_pomocno p)
{
	 
    uint64_t najbolja_zbrckanost = 0, z, i, najbolji_broj = 0, novi_broj;;
    
    

	for(i = 0; i < velicina_grupe; ++i)
	{
		novi_broj = daj_novi_slucajan_prosti_broj(&p);
		z = zbrckanost(novi_broj);

		if(z > najbolja_zbrckanost)
		{
			najbolja_zbrckanost = z;
			najbolji_broj = novi_broj;
		}
	}
	return najbolji_broj;
}

uint64_t procijeni_velicinu_grupe()
{	struct gmp_pomocno p;
    inicijaliziraj_generator(&p, 0);
	uint64_t M = 1000;
	uint64_t SEKUNDI = 5;
	time_t t = time(NULL);
	uint64_t k = 0, i,broj,brojeva_u_sekundi, velicina_grupe = 1;

	while(time(NULL) < t + SEKUNDI)
	{
		k++;

		for(i = 0; i < M; ++i)
		{
			broj = generiraj_dobar_broj(velicina_grupe, p);
			stavi_u_MS(broj);
		}
	}
	brojeva_u_sekundi = k * M / SEKUNDI;
	velicina_grupe = brojeva_u_sekundi / 2.5;
	obrisi_generator(&p);
    return velicina_grupe;
}

int maksimum(int a, int b, int c)
{
    int max = a;
    if(max < b)
        max = b;
    if(max < c)
        max = c;
    return max;
}

void udi_u_KO(int i) //Lamportov algoritam
{
    int j;
    ulaz[i] = 1;
    Broj[i] = maksimum(Broj[0], Broj[1], Broj[2]) + 1;
    ulaz[i] = 0;

    for (j = 0; j < 3; ++j) {
        while (ulaz[j] == 1)
            ;// Čeka se da dretva j dobije broj, ako je u postupku dobivanja
        while (Broj[j] != 0 && (Broj[j] < Broj[i] || (Broj[j] == Broj[i] && j < i)))
            ; // cekaj ako j ima prednost
    }
}

void izadi_iz_KO(int i)
{
    Broj[i] = 0;
}

void *rad_dretva (void *I_D)
{
    struct gmp_pomocno p;
    int *ID = I_D;
	inicijaliziraj_generator(&p,*ID);
    uint64_t broj;
    
    do {
        broj = generiraj_dobar_broj(velicina_grupe, p);

        udi_u_KO(*ID);

        stavi_u_MS(broj);
        printf("Stavila sam: %" PRIu64 "\n", broj);

        izadi_iz_KO(*ID);
    } while (Kraj != KRAJ_RADA);
    obrisi_generator(&p);

    return NULL;
}


void *nerad_dretva (void *I_D)
{
    int *ID = I_D;
    do {
        sleep(3);

        udi_u_KO(*ID);

        uint64_t broj = uzmi_iz_MS();
        printf("Uzela sam: %" PRIu64 "\n", broj);

        izadi_iz_KO(*ID);
    } while (Kraj != KRAJ_RADA);

    return NULL;
}


int main(int argc, char *argv[])
{
	int i, j;
	int ID_radne_dretve[3], ID_neradne_dretve[3];
    pthread_t rad_dretve[3], nerad_dretve[3];
    velicina_grupe = procijeni_velicinu_grupe();

    for (i = 0; i < 3; i++) {
		ID_radne_dretve[i] = i;
		if (pthread_create (&rad_dretve[i], NULL, &rad_dretva, &ID_radne_dretve[i])) {
			printf("Ne mogu stvoriti novu radnu dretvu!\n");
			exit(1);
		}
	}

    for (i = 0; i < 3; i++) {
		ID_neradne_dretve[i] = i;
		if (pthread_create (&nerad_dretve[i], NULL, &nerad_dretva, &ID_neradne_dretve[i])) {
			printf("Ne mogu stvoriti novu neradnu dretvu!\n");
			exit(1);
		}
	}

	sleep(20);
    Kraj = KRAJ_RADA;

	for (j = 0; j < 3; j++) {
        pthread_join (rad_dretve[j], NULL);
    }
    for (j = 0; j < 3; j++) {
        pthread_join (nerad_dretve[j], NULL);
    }
	return 0;
}

/*
  prevo�enje:
  - ru�no: gcc program.c slucajni_prosti_broj.c -lgmp -lm -o program
  - preko Makefile-a: make
  pokretanje:
  - ./program
  - ili: make pokreni
  nepotrebne datoteke (.o, .d, program) NE stavljati u repozitorij
  - obrisati ih ru�no ili s make obrisi
*/