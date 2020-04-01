/*
 * Jednostavni primjer za generiranje prostih brojeva kori�tenjem
   "The GNU Multiple Precision Arithmetic Library" (GMP)
 */

#include <stdio.h>
#include <time.h>
#include<stdlib.h>
#include "slucajni_prosti_broj.h"

struct gmp_pomocno p;

uint64_t MS[5], ULAZ = 0, IZLAZ = 0;

void stavi_u_MS(uint64_t broj)
{
    MS[ULAZ] = broj;
    ULAZ = (ULAZ + 1) % 5;
}

uint64_t uzmi_iz_MS()
{
    uint64_t broj = MS[IZLAZ];
    IZLAZ = (IZLAZ + 1) % 5;

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

uint64_t generiraj_dobar_broj()
{
	uint64_t najbolji_broj = 0, novi_broj;
    uint64_t najbolja_zbrckanost = 0, z;
    uint64_t i, velicina_grupe = 1000;
    //sto je velicina grupe veca, to dulje traje izvrsavanje programa

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
{
	uint64_t M = 1000;
	uint64_t SEKUNDI = 10;
	time_t t = time(NULL);
	uint64_t k = 0, i,broj,brojeva_u_sekundi, velicina_grupe = 1;

	while(time(NULL) < t + SEKUNDI)
	{
		k++;

		for(i = 0; i < M; ++i)
		{
			broj = generiraj_dobar_broj();
			stavi_u_MS(broj);
		}
	}
	brojeva_u_sekundi = k * M / SEKUNDI;
	velicina_grupe = brojeva_u_sekundi * 2.5;

    return velicina_grupe;
}

int main(int argc, char *argv[])
{
	inicijaliziraj_generator (&p, 0);
    uint64_t broj, broj_ispisa = 0;
    time_t t = time(NULL);
    while(broj_ispisa < 10)
	{
        broj = generiraj_dobar_broj();

        stavi_u_MS(broj);

        if(time(NULL) != t)
		{
            broj = uzmi_iz_MS();
            printf("%" PRIu64 "\n", broj);
            broj_ispisa++;
            t = time(NULL);
        }
    }

	obrisi_generator (&p);

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