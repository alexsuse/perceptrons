/*Alex Kunze Susemihl --- 28/10/2009

This program runs an agent-based simulation of the model described in http://www.iop.org/EJ/abstract/1742-5468/2009/03/P03015/
The file sociedade.c is the front-end whereas the more fundamental funcions are in sociedade.h

Some code-specified quantities is the number of times the simulation is run to take the mean histogram, given by Nruns, the number of bins in the issue projection histogram Nrhobins and the
number of bins in the agent-agent projection histograms NJJbins.

runsociety takes all the parameters for running the simulation and runs it for MCSTEPS. The montecarlo step is defined as the mean time it takes for the dynamics to visit every site on the
network. runsociety also increments the quantities of interest to be stored in the histograms passed as arguments of the function.

*/

#include "sociedade.h"

/*runsociety takes a graph, the number of issues, the issue matrix, the dimension of the agent's moral "matrix", the number of agents, the parameters delta and noise, and all the histograms where the output should be
stored.
*/
void print_histogram(char *name, agent *sociedade[Nruns],int nissues, gsl_histogram *JJ, gsl_histogram *rho1, gsl_histogram *rho2, gsl_histogram *rho3, gsl_histogram *rho4, gsl_histogram *rho5, int histsteps);

agent *runsociety(int MCSTEPS, int dim, int size, agent *sociedade, igraph_t *graph, int nissues, gsl_matrix *issues, gsl_histogram *rho1, gsl_histogram *rho2, gsl_histogram *rho3, gsl_histogram *rho4, gsl_histogram *rho5, gsl_histogram *JJ, double delta, double noise);

/*Main handles the interface, reads the parameters from the command line and runs the simulation through runsociety.*/

int main(int argc, char *argv[]){
	agent *sociedade[Nruns];
	
	int i,j,MCSTEPS, dim, network,L,histstep;
	
	char name[200] = "histograma_agents",aux[200]="\0";
	
	int size,nissues;
	double delta, rp, noise;
	
	gsl_matrix *issues;
	
	gsl_histogram *rho1, *rho2, *rho3, *rho4, *rho5, *JJ;
	
	
	igraph_t g;
	igraph_t *graph=&g;
	igraph_vector_t vec;
		
	igraph_vector_init(&vec,2);
	
	
	/*Parsing parameters!*/
	
	if(argc!=10){
		help();
		return 1;
	}
	
	L = atoi(argv[1]);//sqrt(size)
	delta = atof(argv[2]);//delta is the parameter that defines the cognitive strategy of the agents.
	dim = atoi(argv[4]);//dimension of the agents moral "matrix"
	nissues = atoi(argv[5]);//number of issues discussed
	network = atoi(argv[6]);//network option 1 gives a square 2d lattice, 2 gives a BA network, 3 a Watts-Strogatz small world network
	rp = atof(argv[7]);//Rewiring probability (only used for network = 1 or 3
	MCSTEPS = atoi(argv[8]);//Montecarlosteps
	histstep = atoi(argv[9]);//unused
	noise = atof(argv[3]);//noise, analogue to the temperature in the mean-field model. noise = 0.5 -> temp = infty, noise = 0 -> temp = 0
	
	size = L*L;
	
	strncat(aux,"_P:",3);
	strncat(aux,argv[5],4);
	strncat(aux,"_delta:",7);
	strncat(aux,argv[2],4);
	strncat(aux,"_noise:",7);
	strncat(aux,argv[3],4);
	strncat(aux,"_net:",5);
	strncat(aux,argv[6],4);
	strncat(name,aux,51);
	
	/*Graph initialization*/
	
	if(network==1){
		VECTOR(vec)[0]=L;
		VECTOR(vec)[1]=L;
		
		igraph_lattice(graph,&vec,0,0,0,1);
		
		igraph_rewire_edges(graph,rp);
	}
	
	if(network==2){
		igraph_barabasi_game(graph,size,2,NULL,0,0);
	}
	
	if(network==3){
		
		igraph_watts_strogatz_game(graph,1,size,3,rp);

	}
	
	
	/*Issue matrix initialization*/
	
	issues = initialize_issue_matrix(nissues,dim,1);
	
	/*Sure, not very elegant, but whatever!*/

	for(i=0;i<Nruns;i++){
	  sociedade[i]=NULL;
	}
	
	/*running the simulation Nruns times*/
	
	j=0;
	
	while(j*histstep<MCSTEPS){

	  /*Histograms for the projections on each of the (possibly) five issues.*/
	
	  rho1 = gsl_histogram_alloc(Nrhobins);
	  rho2 = gsl_histogram_alloc(Nrhobins);
	  rho3 = gsl_histogram_alloc(Nrhobins);
	  rho4 = gsl_histogram_alloc(Nrhobins);
	  rho5 = gsl_histogram_alloc(Nrhobins);
	  
	/*And the histogram for the projections between agents.*/
	  
	  JJ = gsl_histogram_alloc(NJJbins);
	  
	  gsl_histogram_set_ranges_uniform(rho1,-1.0,1.0);
	  gsl_histogram_set_ranges_uniform(rho2,-1.0,1.0);
	  gsl_histogram_set_ranges_uniform(rho3,-1.0,1.0);
	  gsl_histogram_set_ranges_uniform(rho4,-1.0,1.0);
	  gsl_histogram_set_ranges_uniform(rho5,-1.0,1.0);
	  gsl_histogram_set_ranges_uniform(JJ,-1.0,1.0);
	  
	  j++;

	  for(i=0;i<Nruns;i++){
	    sociedade[i] = runsociety(histstep,dim,size,sociedade[i], graph, nissues, issues, rho1, rho2, rho3, rho4, rho5, JJ, delta, noise);
	  }
	
	  print_histogram(name,sociedade,nissues,JJ,rho1,rho2,rho3,rho4,rho5,j*histstep);

	}

	gsl_matrix_free(issues);
	
	igraph_destroy(graph);
	
	printf("\a\a\a\a\a");
	
	return 0;
}


void print_histogram(char *name, agent *sociedade[Nruns],int nissues, gsl_histogram *JJ, gsl_histogram *rho1, gsl_histogram *rho2, gsl_histogram *rho3, gsl_histogram *rho4, gsl_histogram *rho5, int histsteps){
  FILE *f;
  char copy[200],runsteps[20];

  sprintf(runsteps,"%d",histsteps);

	/*And writing out all the histograms.*/

	strncpy(copy,name,200);	
	
	strncat(copy,"_MCsteps:",9);
	strncat(copy,runsteps,20);
	
	strncat(copy,"_JJ\0",4);
	
	if((f=fopen(copy,"w"))==NULL){
		printf("couldnt open file!!");
	
	}
	
	gsl_histogram_scale(JJ,1.0/gsl_histogram_max_val(JJ));
	
	gsl_histogram_fprintf(f,JJ,"%lf","%lf");

	fclose(f);

	gsl_histogram_free(JJ);

	strncpy(copy,name,200);	
	
	strncat(copy,"_MCsteps:",9);
	strncat(copy,runsteps,20);
	
	strncat(copy,"_rho1\0",6);
	
	if((f=fopen(copy,"w"))==NULL){
		printf("couldnt open file!!");
	}
	
	gsl_histogram_scale(rho1,1.0/gsl_histogram_max_val(rho1));
	
	gsl_histogram_fprintf(f,rho1,"%lf","%lf");

	gsl_histogram_free(rho1);
	
	fclose(f);
	
	if(nissues>=2){

	strncpy(copy,name,200);	
	
	strncat(copy,"_MCsteps:",9);
	strncat(copy,runsteps,20);
	
	strncat(copy,"_rho2\0",6);
	
	if((f=fopen(copy,"w"))==NULL){
		printf("couldnt open file!!");
		
	}
	
	gsl_histogram_scale(rho2,1.0/gsl_histogram_max_val(rho2));
	
	gsl_histogram_fprintf(f,rho2,"%lf","%lf");

	gsl_histogram_free(rho2);
	
	fclose(f);
	
	}
	
	if(nissues>=3){

	strncpy(copy,name,200);	
	
	strncat(copy,"_MCsteps:",9);
	strncat(copy,runsteps,20);
	
	strncat(copy,"_rho3\0",6);
		
	if((f=fopen(copy,"w"))==NULL){
		printf("couldnt open file!!");
		
	}
	
	gsl_histogram_scale(rho3,1.0/gsl_histogram_max_val(rho3));
	
	gsl_histogram_fprintf(f,rho3,"%lf","%lf");

	gsl_histogram_free(rho3);
	
	fclose(f);
	
	}
	
	if(nissues>=4){
	
	strncpy(copy,name,200);	
	
	strncat(copy,"_MCsteps:",9);
	strncat(copy,runsteps,20);   

	strncat(copy,"_rho4\0",6);
	
	if((f=fopen(copy,"w"))==NULL){
		printf("couldnt open file!!");
	}
	
	gsl_histogram_scale(rho4,1.0/gsl_histogram_max_val(rho4));
	
	gsl_histogram_fprintf(f,rho4,"%lf","%lf");

	gsl_histogram_free(rho4);
	
	fclose(f);
	
	}
	
	if(nissues>=5){
	
	  strncpy(copy,name,200);	

	  strncat(copy,"_MCsteps:",9);
	  strncat(copy,runsteps,20);

	  strncat(copy,"_rho5\0",6);
	
	  if((f=fopen(copy,"w"))==NULL){
		printf("couldnt open file!!");
	  }
	
	  gsl_histogram_scale(rho5,1.0/gsl_histogram_max_val(rho5));
	
	  gsl_histogram_fprintf(f,rho5,"%lf","%lf");

	  fclose(f);

	  gsl_histogram_free(rho5);
	  
	}
	


}


agent *runsociety(int MCSTEPS, int dim, int size, agent *sociedade, igraph_t *graph, int nissues, gsl_matrix *issues, gsl_histogram *rho1, gsl_histogram *rho2, gsl_histogram *rho3, gsl_histogram *rho4, gsl_histogram *rho5, gsl_histogram *JJ, double delta, double noise){
	int i,j,k,l;
	
	if(sociedade==NULL) sociedade = initialize_society(size,delta,dim);
	
	for(i=0;i<MCSTEPS;i++){

			
		for(j=0;j<size;j++){
			l = rand()%size;
			k = rand()%nissues;
			learn_from_neighbor(sociedade, graph, l, issues, k, dim,noise);
		}


	}
		
	

	increment_JJ_histogram(sociedade,dim,size,graph,JJ);
	increment_rho_histogram(sociedade,dim,size,graph,issues,0,rho1);

	
	if(nissues>1){

		increment_rho_histogram(sociedade,dim,size,graph,issues,1,rho2);
		
		if(nissues>2){
				

			increment_rho_histogram(sociedade,dim,size,graph,issues,2,rho3);
				
			if(nissues>3){ 
					

				increment_rho_histogram(sociedade,dim,size,graph,issues,3,rho4);
					
				if(nissues>4){

					increment_rho_histogram(sociedade,dim,size,graph,issues,4,rho5);

				}
					
			}
				
		}
		
	}	
							

	
	return sociedade;

}

