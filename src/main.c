/*
 * main.c
 *
 *  Created on: 21 jul. 2022
 *      Author: Tana
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "../includes/finance.h"

#define MAX 12


typedef struct {
	char tipo[4];
	char *numero;
	char monto[12];
	char codigo[3];
}REQUEST_MESSAGE;

typedef struct {
	char tipo[4];
	char codigo[2];
}RESPONSE_MESSAGE;

char APR_COD[]="00";

int main(){
	float monto;
	char aux_tarjeta[8];

	REQUEST_MESSAGE rqMess;
	RESPONSE_MESSAGE rpMess;

	const char ip[]="192.0.10.1";
	unsigned short port=1;

	int maxTimeout=100000;

	FILE *ptrRangesFile;
	FILE *ptrCardFile;
	range_t range;
	card_t card;
	int id;

	puts("Importe:");
	scanf( "%f", &monto);
	ftoa(monto, rqMess.monto, 2);


	puts("Numero de tarjeta:");
	scanf( "%s", rqMess.numero);

	int len=strlen(rqMess.numero);
	char snum[2];

	itoa(len, snum, 10);
	rqMess.numero=strcat(snum,rqMess.numero);


	ptrRangesFile=fopen ("ranges.dat", "r");
	if(ptrRangesFile==NULL){puts("ranges.dat error");}
	else
	{
		/*“TARJETA NO SOPORTADA” en pantalla y abortar la operación, de lo contrario mostrar el label de la tarjeta en pantalla y pasar al siguiente paso.*/
		while(fread(&range, sizeof(range_t), 1, ptrRangesFile))
		{
			if(range.len==len)
			{
				for(unsigned int i =0; i<8;i++){
					aux_tarjeta[i]=rqMess.numero[i+2];
				}
				if(atoi(range.rangeLow)<= atoi(aux_tarjeta) && atoi(aux_tarjeta)<=atoi(range.rangeHigh))
				{
					id=range.id;
					ptrCardFile=fopen ("cards.dat", "r");

					if(ptrCardFile==NULL) puts("cards.dat error");
					else
					{
						while(fread(&card, sizeof(card_t), 1, ptrCardFile))
						{
							if(card.id==id){
								printf("Label : %s",card.label);
								break;
							}
						}
						fclose (ptrCardFile);
					}
					break;
				}
			}
		}
		if(!ptrRangesFile){
			puts("TARJETA NO SOPORTADA");
			fclose (ptrRangesFile);
			return 0;
		}

		// close file
		fclose (ptrRangesFile);
	}


	puts("Codigo de seguridad:");
	scanf( "%3s", rqMess.codigo);

	/*Armar el “request message” con los datos de la transacción (ver “Formato de los mensajes”) y enviarlo al host.*/

	int handle = socketCreate();
	if(handle!=-1){
		if(!socketConnect(handle, ip, port)){
			if(socketWrite(handle, rqMess)==sizeof(rqMess)){
				if(socketRead(handle, rpMess, maxTimeout)>=0)
				{
					(!strcmp(rpMess.codigo,APR_COD))? puts("APROBADA"):puts("RECHAZADA");
				}
				else puts("ERROR DE COMUNICACION");

			}
			else puts("ERROR DE COMUNICACION");
		}
		else puts("ERROR DE COMUNICACION");
		(!socketClose(handle))? puts("\n"):puts("ERROR DE COMUNICACION");
	}
	else puts("ERROR DE COMUNICACION");

	return 0;
}
