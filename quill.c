/*
AUTOR:pereira suarez, jose antonio:infjps00
AUTOR:perez-roca fernandez, jesus angel:infjps00
*/

#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <signal.h>

int main(int argc,char *argv[]) {
	
	int i,senhal;
	char *senhal_aux;

	if (argc!=4) printf("Numero de argumentos invalido. \nejemplo: quill num_veces pid senhal\n");
		else{
			if (atoi(argv[1])<-1) printf ("Numero de veces invalido. \n");
				else if (atoi(argv[1])==-1) {
								senhal_aux=(char *) malloc ((strlen(argv[3])-2)*sizeof(char));
								for (i=0;i<strlen(argv[3])-2;i++) senhal_aux[i]=argv[3][i+3];
								str2sig(senhal_aux,&senhal);
								while (1) {if (kill(atoi(argv[2]),senhal)==-1){
														 printf("Error al enviar la señal.\n");
														 exit(0);
													       } 
									  }
							    }								
					else if (atoi(argv[1])>-1) {
									senhal_aux=(char *) malloc ((strlen(argv[3])-2)*sizeof(char));
									for (i=0;i<strlen(argv[3])-2;i++) senhal_aux[i]=argv[3][i+3];
								     	str2sig(senhal_aux,&senhal);
								     	for (i=atoi(argv[1]);i>0;i--) if (kill(atoi(argv[2]),senhal)==-1) {
														printf("Error al enviar la señal.\n");
														exit(0);
													}
								    }
						else printf("Argumentos invalidos. \nEjemplo: quill num_veces pid senhal\n");
		    }
}
