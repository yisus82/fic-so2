/*
AUTOR:pereira suarez, jose antonio:infjps00
AUTOR:perez-roca fernandez, jesus angel:infjps00
*/


#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <wait.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
#include <signal.h>

#define MAX_ARGS 10
#define MAX_DIRS 30
#define MAX_PROMPT 20
#define MAX_COMANDO 50
#define MAX_PATH 60
#define MAX 100
#define MAX_ENTORNO 100
#define MAX_PROCS 50



struct proceso{
	pid_t pid;
	char tiempo[21];
	char est[25];
	char l_comando[MAX_PATH+20];
	int vacio;
 }espera[MAX_PROCS];



char * donde(char p[][MAX_PATH],char *ejec)
 
{

    struct stat s;
    static char aux[MAX_PATH];
    int i=0,encontrado=0;
    
   while((!encontrado) && (strcmp(p[i],""))){
      
        strcpy(aux,p[i]);
        strcat(aux,"/");
        strcat(aux,ejec);

        if (stat(aux,&s)!=-1) encontrado=1;
          else i++;
    }

    if (encontrado) return(aux);
      else return(NULL);
 }


void actualizar(struct proceso espera[])
{
	char aux[10];
	int estado;
	int i;
	for (i=2;i<MAX_PROCS;i++){
	    if (espera[i].vacio==0){
		if (espera[i].pid==waitpid(espera[i].pid,&estado,WNOHANG|WUNTRACED|WCONTINUED)){	
			if (WIFSTOPPED(estado)) {
				strcpy(espera[i].est,"Parado (SIG");
				sig2str(WSTOPSIG(estado),aux);
				strcat(espera[i].est,aux);
				strcat(espera[i].est,")");
			  }
			if (WIFSIGNALED(estado)) {
				strcpy(espera[i].est,"Terminado (SIG");
			 	sig2str(WTERMSIG(estado),aux);
				strcat(espera[i].est,aux);
				strcat(espera[i].est,")");
			  }
			if (WIFEXITED(estado)) {
				strcpy(espera[i].est,"Term. Normal (");
				sprintf(aux,"%d",WEXITSTATUS(estado));
				strcat(espera[i].est,aux);
				strcat(espera[i].est,")");
			   }
			if (WIFCONTINUED(estado)) strcpy(espera[i].est,"Activo");
	 
		  }
		}
	}
}


int ejecutar(char *params[],int backgrd, char path[][MAX_PATH],struct proceso espera[])
{
  	char *ejec;
	int pid;
	int estado;
	int i,j;

	if ((params[0][0]=='/')||(params[0][0]=='.')) ejec=params[0];
	   else ejec=donde(path,params[0]);
	if (ejec==NULL) {
			 printf("Comando no valido\n");
			 return (-1);
			}
	if ((pid=fork())==0){
				if ((execv(ejec,params))==-1) return(pid);
					else return(-1);
			    }
	   else {   
     		if (!backgrd) waitpid(pid,&estado,0);
			else {
				i=2;
				while (i<MAX_PROCS)
				 {
				   if (espera[i++].vacio) break;
				 }
				if (espera[i-1].vacio) {
							espera[i-1].pid=pid;
							instante(espera[i-1].tiempo);
							strcpy(espera[i-1].est,"Activo");
							strcpy(espera[i-1].l_comando,ejec);
							j=1;
							while(params[j]!=NULL){
										strcat(espera[i-1].l_comando," ");
							 			strcat(espera[i-1].l_comando,params[j]);
										j++;
									      }
							espera[i-1].vacio=0;
						       }
					else printf("No se pueden poner mas procesos en la lista");
				}
     		}  
        	
}
 

int instante(char t[])
{
	int i;
	struct timeval tv;
	char temporal[25];
	char *auxiliar[5];
	gettimeofday(&tv,(void *)NULL);
	strcpy(temporal,ctime(&tv.tv_sec));
	auxiliar[0]=strtok(temporal," ");
	for (i=1;i<5;i++) auxiliar[i]=strtok(NULL," ");
	strcpy(t,auxiliar[2]);
	strcat(t," ");
	if (!(strcmp(auxiliar[1],"Jan"))) strcpy(auxiliar[1],"Ene");
		else if (!(strcmp(auxiliar[1],"Apr"))) strcpy(auxiliar[1],"Abr");
			else if (!(strcmp(auxiliar[1],"Aug"))) strcpy(auxiliar[1],"Aug");
				else if (!(strcmp(auxiliar[1],"Dec"))) strcpy(auxiliar[1],"Dic");
	strcat(t,auxiliar[1]);
	strcat(t," ");
	auxiliar[0]=(char *) malloc(5*sizeof(char));
	for (i=0;i<4;i++) auxiliar[0][i]=auxiliar[4][i];
	strcat(t,auxiliar[0]);
	strcat(t," ");
	strcat(t,auxiliar[3]);
}



int main(int argc,char *argv[],char *entorno[]){

int i,j=0,k,l;
int estado,pid;
char prompt[MAX_PROMPT]="trash-1.01=>";
char comando[MAX_COMANDO];
char *args[MAX_ARGS]; 
char path[MAX_DIRS][MAX_PATH];
char *ejecutable;
char *aux;
char *temp=NULL;
char *ruta[MAX_DIRS];
char *dir_actual;
	
printf("\n");
instante(espera[1].tiempo);
printf(espera[1].tiempo);
printf("\n\n");
espera[0].pid=getppid();
strcpy(espera[0].est,"Activo");
espera[0].vacio=0;
espera[1].pid=getpid();
strcpy(espera[1].est,"Activo");
strcpy(espera[1].l_comando,argv[0]);
for (i=2;i<MAX_PROCS;i++) espera[i].vacio=1; 

while (1) {

  int encontrado=0;

  printf("%s",prompt);
  gets(comando);
  args[0] = strtok(comando," ");
  i=1;
  while(args[i++]=strtok(NULL," "));
 
  if (args[0]!=NULL)
{

  if (!strcmp(args[0],"autores")){  

                   printf("\nPractica 1 de SO2\n");
                   printf("Realizada por:\n"); 
                   printf("\nJose Antonio Pereira Suarez (infjps00)\n");
                   printf("Jesus Angel Perez-Roca Fernandez (infjpf02)\n\n");
                   continue;
   }


  if (!(strcmp(args[0],"prompt"))){

	if (args[1]!=NULL) if ((strlen(args[1]))<MAX_PROMPT+1) strcpy(prompt,args[1]);
				else printf("El prompt no puede tener mas de %d caracteres\n",MAX_PROMPT);
               else printf("Numero de argumentos insuficiente\nEjemplo: prompt cadena\n");
	 continue;
  }
    
  
  if (!(strcmp(args[0],"path"))) {

      if (args[1]!=NULL) {
	for (i=0;i<j;i++) if (!(strcmp(path[i],args[1]))) {
				encontrado=1;
				printf("El directorio ya estaba en la ruta de busqueda\n");
				}
	if (j<MAX_DIRS) {if (!encontrado) strcpy(path[j++],args[1]);}
			else printf("No se pueden poner mas directorios en la ruta de busqueda\n");
				
			  	
	}		  
	else { 
		for (i=0;i<j;i++) printf ("%s\n",path[i]);
		if (j==0) printf ("La ruta de busqueda no contiene ningun directorio\n");
	     }
	continue;
   }

  
  if (!(strcmp(args[0],"delpath"))){ 

      if (args[1]!=NULL) {
	for (i=0;i<j;i++) if (!(strcmp(path[i],args[1]))) {
							    encontrado=1;
							    for (i++;i<j;i++) strcpy(path[i-1],path[i]);
							    strcpy(path[i],"");
							    j--;
							  }
	if (!encontrado) printf ("El directorio no esta en la ruta de busqueda\n");
			   	
	}		  
	else {
		for (i=0;i<j;i++) strcpy(path[i],"");
		j=0;
	     }
	continue;
  }
  
  
   if (!(strcmp(args[0],"getpath"))) {
					
	aux=getenv("PATH");
	temp=(char *) malloc((strlen(aux)*sizeof(aux)));
	strcpy(temp,aux);
	ruta[0]=strtok(temp,":");
	encontrado=0;
	for (i=0;i<j;i++) if (!(strcmp(path[i],ruta[0]))) encontrado=1;
	if (j<MAX_DIRS) if (!encontrado) strcpy(path[j++],ruta[0]);
	l=1;
	while (ruta[l++]=strtok(NULL,":"))
		{
    			encontrado=0;
		        for (k=0;k<j;k++) if (!(strcmp(path[k],ruta[l-1]))) encontrado=1;
			if (j<MAX_DIRS) {if (!encontrado) strcpy(path[j++],ruta[l-1]);}
				else {
					printf("No se pueden poner mas directorios en la ruta de busqueda\n");
					break;
				     }
		         }
			temp=NULL;
			continue;
  }
  

  if (!(strcmp(args[0],"whereis"))){

	if (args[1]!=NULL){
          	ejecutable= donde(path,args[1]);
                if (ejecutable!=NULL) printf("%s\n",ejecutable);
                  else printf("No se encuentra el ejecutable\n");            
           } 
 	   else  printf("Numero de argumentos insuficiente\nEjemplo: whereis fichero\n");
         continue;  
	
  }


  if (!(strcmp(args[0],"proc"))){
	if (args[1]!=NULL){
		actualizar(espera);
		if (!(strcmp(args[1],"-l"))) for (i=2;i<MAX_PROCS;i++){
 			if (!(espera[i].vacio)) if (!(strncmp(espera[i].est,"Term",4))) espera[i].vacio=1;
		  }
		  else printf("Opcion invalida\nEjemplo: proc [-l]\n");
		}
	  else {
		actualizar(espera);
		printf("PID\t\tINSTANTE\t\tESTADO\t\t\tCOMANDO\n");
		printf("%d\t\t?? ??? ???? ????????\tActivo\t\t\t????(Padre del shell)\n",espera[0].pid);
		printf("%d\t\t%s\tActivo\t\t\t%s(Shell actual)\n",espera[1].pid,espera[1].tiempo,espera[1].l_comando);
		for (i=2;i<MAX_PROCS;i++) if (!(espera[i].vacio)){ 
			if(!(strcmp(espera[i].est,"Activo"))) printf("%d\t\t%s\t%s\t\t\t%s\n",espera[i].pid,espera[i].tiempo,espera[i].est,espera[i].l_comando);
				else printf("%d\t\t%s\t%s\t%s\n",espera[i].pid,espera[i].tiempo,espera[i].est,espera[i].l_comando);
		   }
		}
	continue;
  }


  if (!(strcmp(args[0],"chdir"))){

		dir_actual=getcwd(NULL,MAX_PATH);
		if (args[1]!=NULL) chdir(args[1]);
		   else printf ("%s\n",dir_actual);
	
		continue;
  }

 
  if(!(strcmp(args[0],"exec"))) {
		if (args[1]!=NULL){
				  for (k=0;k<i-1;k++) args[k]=args[k+1];
				  if ((args[0][0]=='/')||(args[0][0]=='.')) ejecutable=args[0];
	   				else ejecutable=donde(path,args[0]);
				  if (ejecutable==NULL)  printf("No se ha encontrado el ejecutable en la ruta de busqueda\n");
					else execv(ejecutable,args);
				}
			else printf("Numero de argumentos insuficiente\nEjemplo: exec cadena\n");
  		continue;
  }
	

  if (!(strcmp(args[0],"fork"))) {
	if ((pid=fork())==0){
				espera[0].pid=espera[1].pid;
				espera[1].pid=getpid();
				instante(espera[1].tiempo);
			     	for (i=2;i<MAX_PROCS;i++) espera[i].vacio=1;
			     }
		else waitpid(pid,&estado,0);
	continue;
  }
				
  
  if (!(strcmp(args[0],"pid"))) {

		printf("El pid del shell es %d\n",getpid());
		printf("El pid del proceso padre es %d\n",getppid());
		continue;
  } 


  if (!(strcmp(args[0],"quit"))) break;


  if (!(strcmp(args[i-2],"&"))) {
				  args[i-2]=NULL;
				  ejecutar(args,1,path,espera);
				  continue;
		    	       	  }

  ejecutar(args,0,path,espera);

  	 

}

}

return(0);
}
