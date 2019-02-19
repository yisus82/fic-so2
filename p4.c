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
#include <sys/priocntl.h>
#include <sys/rtpriocntl.h>
#include <sys/tspriocntl.h>
#include <pwd.h>


#define MAX_ARGS 10
#define MAX_DIRS 30
#define MAX_PROMPT 20
#define MAX_COMANDO 50
#define MAX_PATH 60
#define MAX 100
#define MAX_ENTORNO 100
#define MAX_PROCS 50


extern char **environ;

struct proceso{
	pid_t pid;
	char tiempo[20];
	char est[25];
	char l_comando[MAX_PATH+20];
	int vacio;
 }espera[MAX_PROCS];



pri_t obtenerprioridad(pid_t pid)
{
 pcparms_t p;
 tsparms_t *t;
 p.pc_cid=PC_CLNULL;
 if (priocntl(P_PID,pid,PC_GETPARMS,(caddr_t)&p)==-1) return(1);
 t=(tsparms_t *)p.pc_clparms;
 return(t->ts_upri);
}


pri_t obtenerlimite(pid_t pid)
{
 pcparms_t p;
 tsparms_t *t;
 p.pc_cid=PC_CLNULL;
 if (priocntl(P_PID,pid,PC_GETPARMS,(caddr_t)&p)==-1) return(1);
 t=(tsparms_t *)p.pc_clparms;
 return(t->ts_uprilim);
}


int establecerprioridad(pid_t pid,pri_t nuevapri)
{
  tsparms_t *t;
  pcparms_t p;
  p.pc_cid=PC_CLNULL;
  priocntl(P_PID,pid,PC_GETPARMS,(caddr_t)&p);
  t=(tsparms_t *)p.pc_clparms;
  t->ts_upri=nuevapri;
  return(priocntl(P_PID,pid,PC_SETPARMS,(caddr_t)&p));
} 


int establecerlimite(pid_t pid,pri_t nuevolim)
{
  tsparms_t *t;
  pcparms_t p;
  p.pc_cid=PC_CLNULL;
  priocntl(P_PID,pid,PC_GETPARMS,(caddr_t)&p);
  t=(tsparms_t *)p.pc_clparms;
  t->ts_uprilim=nuevolim;
  return(priocntl(P_PID,pid,PC_SETPARMS,(caddr_t)&p)); 
} 


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


int ejecutar(char *params[],int backgrd, char path[][MAX_PATH],struct proceso espera[],int modo,char **env_aux,int prio)
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
				 if (modo==0) {if ((execv(ejec,params))==-1) exit(0);}
				 	else if (modo==1) {if ((execve(ejec,params,env_aux))==-1) exit(0);}
						else {if (modo==2) {if ((execv(ejec,params))==-1) exit(0);}
							}
			    }
	   else {   
		if (modo==2) establecerprioridad(pid,prio);
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
	strncat(t,auxiliar[3],5);
}


int buscavariable(char *entorno[],char *variable)
{
        int l,i=0;
        char *aux;   
        int encontrado=0;
   	aux=(char *) malloc ((strlen(variable)+2)*sizeof(char));
        strcpy(aux,variable);
        strcat(aux,"=");
        l=strlen(aux);
        while (((!encontrado) && (entorno[i]!=NULL)))
        {
           if (strncmp(entorno[i],aux,l)==0) encontrado=1;
            else i++;
        }   
        if (encontrado) return(i);
         else return(-1);
}      


void rec(int n)
{
	char aut[MAX];
	static char est[MAX];
	printf("Direccion variable automatica: %x\n",aut);
	printf("Direccion variable estatica: %x\n",est);
	if (n>0) rec(n-1);
}





int main(int argc,char *argv[],char *entorno[]){

int i,j=0,k,l,n;
int estado,pos1,pos2;
pid_t pid;
pri_t pri,lim;
uid_t uid;
struct passwd * pwd;
char prompt[MAX_PROMPT]="trash-1.04=>";
char comando[MAX_COMANDO];
char *args[MAX_ARGS];
char path[MAX_DIRS][MAX_PATH];
char *ejecutable;
char *aux;
char *temp=NULL;
char *ruta[MAX_DIRS];
char *dir_actual;
char *env_aux[MAX_ARGS-2];
char *args_aux[MAX_ARGS];
	
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
  int x=0;
	
  printf("%s",prompt);
  gets(comando);
  args[0] = strtok(comando," ");
  i=1;
  while(args[i++]=strtok(NULL," "));
 
  if (args[0]!=NULL)
{

  if (!strcmp(args[0],"autores")){  
	if ((i-2)==0){
                   printf("\nPractica 1 de SO2\n");
                   printf("Realizada por:\n"); 
                   printf("\nJose Antonio Pereira Suarez (infjps00)\n");
                   printf("Jesus Angel Perez-Roca Fernandez (infjpf02)\n\n");
            }
	 else printf("Numero de argumentos invalido.\nEjemplo: autores\n");
  	continue; 
  }


  if (!(strcmp(args[0],"prompt"))){
     if ((i-2)==1){
	if ((strlen(args[1]))<MAX_PROMPT+1) strcpy(prompt,args[1]);
		else printf("El prompt no puede tener mas de %d caracteres\n",MAX_PROMPT);
	}     
       else printf("Numero de argumentos invalido.\nEjemplo: prompt cadena\n");
     continue;
  }
    
  
  if (!(strcmp(args[0],"path"))) {
     if ((i-2)>1){ 
		  printf("Numero de argumentos invalido.\nEjemplo: path [cadena]\n");
		  continue;
		 }
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
      if ((i-2)>1){
		   printf("Numero de argumentos invalido.\nEjemplo: delpath [cadena]\n");
		   continue;
		  }
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
	if((i-2)>0) {
			printf("Numero de argumentos invalido.\nEjemplo: getpath \n");
			continue;
		    }
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
	if ((i-2)>1) {
			printf("Numero de argumentos invalido.\nEjemplo: whereis fichero\n");
			continue;
		     }
	if (args[1]!=NULL){
          	ejecutable= donde(path,args[1]);
                if (ejecutable!=NULL) printf("%s\n",ejecutable);
                  else printf("No se encuentra el ejecutable\n");            
           } 
 	   else  printf("Numero de argumentos insuficiente\nEjemplo: whereis fichero\n");
         continue;  
	
  }


  if (!(strcmp(args[0],"proc"))){
	if ((i-2)>1) {
			printf("Numero de argumentos invalido.\nEjemplo: proc [-l]\n");
			continue;
		     }
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
		printf("%d\t\t?? ??? ???? ?????\tActivo\t\t\t????(Padre del shell)\n",espera[0].pid);
		printf("%d\t\t%s\tActivo\t\t\t%s(Shell actual)\n",espera[1].pid,espera[1].tiempo,espera[1].l_comando);
		for (i=2;i<MAX_PROCS;i++) if (!(espera[i].vacio)){ 
			if(!(strcmp(espera[i].est,"Activo"))) printf("%d\t\t%s\t%s\t\t\t%s\n",espera[i].pid,espera[i].tiempo,espera[i].est,espera[i].l_comando);
				else printf("%d\t\t%s\t%s\t%s\n",espera[i].pid,espera[i].tiempo,espera[i].est,espera[i].l_comando);
		   }
		}
	continue;
  }


  if (!(strcmp(args[0],"chdir"))){
		if ((i-2)>1) {
				printf("Numero de argumentos invalido.\nEjemplo: chdir [directorio]\n");
				continue;
			     }	
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
	if ((i-2)>0){
			printf("Numero de argumentos invalido.\nEjemplo: fork\n");
			continue;
		    }
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
		if((i-2)>0){
				printf("Numero de argumentos insuficiente.\nEjemplo: path [cadena]\n");
				continue;
			   }

		printf("El pid del shell es %d\n",getpid());
		printf("El pid del proceso padre es %d\n",getppid());
		continue;
  } 


  if (!(strcmp(args[0],"quit"))){ 
		if((i-2)>0){
				printf("Numero de argumentos invalido.\nEjemplo: quit\n");
				continue;
			}
		break;
  }

  
  if (!(strcmp(args[0],"entorno"))) {
	if ((i-2)>1) {
			printf("Numero de argumentos invalido.\nEjemplo: entorno -a|-e\n");
			continue;
		     }	
        if (args[1]!=NULL)
           if (!(strcmp(args[1],"-a"))){
	               k=0;
        	       while (entorno[k++]!=NULL) printf("%x->entorno[%d]=(%x) %s\n",&entorno[k-1],k-1,entorno[k-1],entorno[k-1]);
              	       printf("%x->environ=%x | %x->entorno=%x\n",&environ,environ,&entorno,entorno);
 		}
	       else if (!(strcmp(args[1],"-e"))){
	               k=0;
        	       while (environ[k++]!=NULL) printf("%x->environ[%d]=(%x) %s\n",&environ[k-1],k-1,environ[k-1],environ[k-1]);
              	       printf("%x->environ=%x | %x->entorno=%x\n",&environ,environ,&entorno,entorno);
		}
			else printf("Opcion invalida.\nEjemplo: entorno -a|-e\n");
	else printf("Numero de argumentos insuficiente.\nEjemplo: entorno -a|-e\n");
	continue; 
  }


  if (!(strcmp(args[0],"setenv"))) {
	if ((i-2)>3){
			printf("Numero de argumentos invalido.\nEjemplo: setenv -a|-e val var\n");
			continue;
		    }
	if ((i-2)==3){
		if(!(strcmp(args[1],"-a"))) {
			pos1=buscavariable(entorno,args[3]);
			if (pos1!=-1) {
				aux=(char *) malloc((strlen(args[3])+strlen(args[2])+2)*sizeof(char));
				strcpy(aux,args[3]);
				strcat(aux,"=");
				strcat(aux,args[2]);
				entorno[pos1]=(char *) malloc ((strlen(aux)+1)*sizeof(char));
				entorno[pos1]=aux;
			   }
			   else printf("No existe la variable %s en el entorno\n",args[3]);
		   }
		   else if (!(strcmp(args[1],"-e"))) {
				pos2=buscavariable(environ,args[3]);
				if (pos2!=-1) {
					aux=(char *) malloc((strlen(args[3])+strlen(args[2])+2)*sizeof(char));
					strcpy(aux,args[3]);
					strcat(aux,"=");
					strcat(aux,args[2]);
					environ[pos2]=(char *) malloc ((strlen(aux)+1)*sizeof(char));
					environ[pos2]=aux;
				}
			   	  else printf("No existe la variable %s en el entorno\n",args[3]);
			   }
			  else printf("Argumento(s) invalido(s).\nEjemplo: setenv -a|-e val var\n");
	  }
	  else printf("Numero de argumentos insuficiente.\nEjemplo: setenv -a|-e val var\n");
	continue;
  }
  
  if (!(strcmp(args[0],"putenv"))) {
	if ((i-2)>2){
			printf("Numero de argumentos invalido.\nEjemplo: putenv var val\n");
			continue;
		    }
	if ((i-2)==2){
		aux=(char *)malloc((strlen(args[1])+strlen(args[2])+2)*sizeof(char)); 
                strcpy(aux,args[1]);
                strcat(aux,"=");
                strcat(aux,args[2]);
                putenv(aux);
	 }
	 else printf("Numero de argumentos insuficiente.\nEjemplo: putenv var val\n");
	continue;
  }
               
  
  if (!(strcmp(args[0],"get"))) {
     if ((i-2)>1){
			printf("Numero de argumentos invalido.\nEjemplo: get var\n");
			continue;
		}
     if (args[1]!=NULL){
	pos1=buscavariable(entorno,args[1]);
	pos2=buscavariable(environ,args[1]);
	if ((pos1==-1)||(pos2==-1)) printf("No existe la variable %s en el entorno\n",args[1]);
		else{
			printf("Con el tercer argumento del main:\n");
			printf("(%x) %s\n",entorno[pos1],entorno[pos1]);
			printf("Con la variable environ:\n");
			printf("(%x) %s\n",environ[pos2],environ[pos2]);
        		printf("Con getenv():\n");
        		aux=getenv(args[1]);
			temp=(char *) malloc((strlen(args[1])+strlen(aux)+2)*sizeof(char));
			strcpy(temp,args[1]);
			strcat(temp,"=");
			strcat(temp,aux);
			printf("(%x) %s\n",aux-(strlen(args[1])+1),temp);
		   }
      }
 
      else printf("Numero de argumentos insuficiente.\nEjemplo: get var\n");
      continue;

  }

  
  if(!(strcmp(args[0],"xenv"))) {
	if ((i-2)>0){
		env_aux[0]=NULL;
		k=1;
		while ((pos1=buscavariable(environ,args[k]))!=-1) {
			env_aux[k-1]=environ[pos1];
			k++;
	  	}
		if (!(strcmp(args[i-2],"&"))){ 
			args[i-2]=NULL;
			i--;
			x=1;
	  	}
		l=0;
		while ((args_aux[l++]=args[k++])!=NULL);
		if ((args_aux[0])!=NULL) ejecutar(args_aux,x,path,espera,1,env_aux,pri);
		continue;
	}
	  else printf("Numero de argumentos insuficiente.\nEjemplo: xenv lista_variables cadena\n");
 	continue;

  }


  if (!(strcmp(args[0],"info"))) {
	if ((i-2)>1) {
			printf("Numero de argumentos invalido.\nEjemplos: info [usr]\n");
			continue;
		     }
	if ((i-2)==1) {
			pwd=getpwnam(args[1]);
			if (pwd!=NULL){
				printf("El login del usuario es: %s\n", pwd->pw_name);
				printf("El password del usuario es: %s\n", pwd->pw_passwd);
				printf("El id del usuario es: %d\n", pwd->pw_uid);
				printf("El id del grupo primario es: %d\n", pwd->pw_gid);
				printf("El nombre del usuario es: %s\n", pwd->pw_gecos);
				printf("El directorio de inicio del usuario es: %s\n", pwd->pw_dir);
				printf("El shell del usuario es: %s\n", pwd->pw_shell);
			  }
			   else printf("El usuario %s no existe\n",args[1]);
			continue;
	}
 
	printf("La variable externa environ esta en %x\n",environ);
	printf("La variable global espera esta en %x\n",espera);
	printf("La funcion donde esta en %x\n",donde);
	printf("La funcion actualizar esta en %x\n",actualizar);
	printf("La funcion ejecutar esta en %x\n",ejecutar);
	printf("La funcion instante esta en %x\n",instante);
	printf("La funcion buscavariable esta en %x\n",buscavariable);
	printf("La funcion rec esta en %x\n",rec);
	printf("La funcion main esta en %x\n",main);
	printf("La variable local i esta en %x\n",&i);
	printf("La variable local j esta en %x\n",&j);
	printf("La variable local k esta en %x\n",&k);
	printf("La variable local l esta en %x\n",&l);
	printf("La variable local n esta en %x\n",&n);
	printf("La variable local estado esta en %x\n",&estado);
	printf("La variable local pid esta en %x\n",&pid);
	printf("La variable local pos1 esta en %x\n",&pos1);
	printf("La variable local pos2 esta en %x\n",&pos2);
	printf("La variable local prompt esta en %x\n",prompt);
	printf("La variable local comando esta en %x\n",comando);
	printf("La variable local args esta en %x\n",args);
	printf("La variable local path esta en %x\n",path);
	printf("La variable local ejecutable esta en %x\n",ejecutable);
	printf("La variable local aux esta en %x\n",aux);
	printf("La variable local ruta esta en %x\n",ruta);
	printf("La variable local dir_actual esta en %x\n",dir_actual);
	printf("La variable local env_aux esta en %x\n",env_aux);
	printf("La variable local args_aux esta en %x\n",args_aux);
	printf("La variable local encontrado esta en %x\n",&encontrado);
	printf("La variable local x esta en %x\n",&x);
	continue;
  }


  if (!(strcmp(args[0],"recursiva"))) {
	if ((i-2)>1) {
			printf("Numero de argumentos invalido.\nEjemplo: recursiva n\n");
			continue;
		     }
	if (args[1]!=NULL){
			    	if ((n=atoi(args[1]))!=0) rec(n);
				   else if (!(strcmp(args[1],"0"))) rec(0);
						else printf("El argumento de la funcion recursiva tiene que ser un numero\n");
			  }		
	   else printf("Numero de argumentos insuficiente.\nEjemplo: recursiva n\n");
	continue;  
  }


 if (!(strcmp(args[0],"xpri"))) {
	n=0;
	if (!(strcmp(args[i-2],"&"))) {
					args[i-2]=NULL;
					i--;
					n=1;
				      }
	if ((i-2)>1) {
		if (!(strcmp(args[i-2],"0"))) pri=0;
			else{
				pri=atoi(args[i-2]);
				if (pri==0) {
						printf("Numero de argumentos insuficiente.\nEjemplo: xpri cadena valor [&]\n");
			  			continue;
					    }  
			  } 
		for (l=0;l<i-3;l++) args_aux[l]=args[l+1];
		ejecutar(args_aux,n,path,espera,2,env_aux,pri);
		continue;
	 }
	   else printf("Numero de argumentos insuficiente.\nEjemplo: xpri cadena valor [&]\n");
 	continue;
 
  }

  if (!(strcmp(args[0],"prio"))) {
	if ((i-2)>2) {
			printf("Numero de argumentos invalido. \nEjemplo:prio pid [valor] \n");
 			continue;
		    }
	if (args[1]!=NULL){	
				pid=atoi(args[1]);
				if (pid<0) {
						printf("El pid no puede ser negativo\n");
						continue;
					   }
				if (args[2]!=NULL) {if (establecerprioridad(pid,atoi(args[2]))==-1) printf ("Error al establecer la prioridad\n");}
					else if ((pri=obtenerprioridad(pid))>0) printf("Error al obtener la prioridad\n");
						else printf("La prioridad del proceso con pid %d es: %d\n",pid,pri);
			   }
	   else printf("Numero de argumentos insuficiente.\nEjemplo: prio pid [valor]\n");
 	continue;
  }


  if (!(strcmp(args[0],"lim"))) {
	if ((i-2)>2) {
			printf("Numero de argumentos invalido. \nEjemplo:lim pid [valor] \n");
 			continue;
		    }
	if (args[1]!=NULL){
				pid=atoi(args[1]);
				if (pid<0) {
						printf("El pid no puede ser negativo\n");
						continue;
					   }
				if (args[2]!=NULL) {if (establecerlimite(pid,atoi(args[2]))==-1) printf ("Error al establecer el limite\n");}
					else if ((lim=obtenerlimite(pid))>0) printf("Error al obtener el limite\n");
			   			else printf("El limite del proceso con pid %d es: %d\n",pid,lim);

				}
	   else printf("Numero de argumentos insuficiente.\nEjemplo: lim pid [valor]\n");
 	continue;
  }


  if (!(strcmp(args[0],"getpri"))){
	if ((i-2)>0) {
			printf("Numero de argumentos invalido. \nEjemplo: getpri\n");
			continue;
		      }
	pid=getpid();
	if ((pri=obtenerprioridad(pid))>0) printf("Error al obtener la prioridad\n");
		else printf("La prioridad del proceso con pid %d es: %d\n",pid,pri);
	if((lim=obtenerlimite(pid))>0) printf("Error al obtener el limite\n");
		else printf("El limite del proceso con pid %d es: %d\n",pid,lim);
	printf("El factor nice del proceso con pid %d es: %d\n",pid,nice(0));
	continue;
  }


  if (!(strcmp(args[0],"uid"))) {
	if ((i-2)>0) {
			printf("Numero de argumentos invalido. \nEjemplo: uid\n");
			continue;
		      }
	uid=getuid();
	printf("Las credenciales reales son: %d\n",uid);
	printf("Las credenciales efectivas son: %d\n",geteuid());
	pwd=getpwuid(uid);
	printf("El login del usuario del proceso es: %s\n",pwd->pw_name);
	continue;
  }


  if (!(strcmp(args[0],"setuid"))) {
	if ((i-2)>2) {
			printf("Numero de argumentos invalido. \nEjemplo: setuid [-l] valor\n");
			continue;
		      }
	if ((i-2)==0) {
			printf("Numero de argumentos insuficiente. \nEjemplo: setuid [-l] valor\n");
			continue;
		      }
	if ((i-2)==1) { 
			if ((setuid(atoi(args[1])))==-1) printf("Error al establecer las credenciales.\n");
			continue;
		      }
	if (strcmp(args[1],"-l")) {
					printf("Opcion invalida. \nEjemplo: setuid [-l] valor\n");
					continue;
				  }
	pwd=getpwnam(args[2]);
	if (pwd!=NULL){
			if ((setuid(pwd->pw_uid))==-1) printf("Error al establecer las credenciales.\n");
		      }
	    else printf("El usuario %s no existe.\n",args[2]);	
	continue;
  }

	
  if (!(strcmp(args[i-2],"&"))) {
				  args[i-2]=NULL;
				  i--;
				  if ((i-2)>-1) ejecutar(args,1,path,espera,0,env_aux,pri);
				  continue;
		    	       	  }

  ejecutar(args,0,path,espera,0,env_aux,pri);

  	 

}

}

return(0);
}
