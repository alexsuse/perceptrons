/*sociedade.h defines all the functions needed for the agent-based simulation of adaptive opinion dynamics.*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include <time.h>
#include <unistd.h>

#include <gsl/gsl_math.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_histogram.h>
#include <gsl/gsl_sf_erf.h>
#include <igraph/igraph.h>

#define Nrhobins 81
#define NJJbins 161
#define Nruns 10
#define eta 1.0

/*Our agent is a vector and a delta which caracterizes his cognitive strategy.*/
typedef struct{
	gsl_vector *J;
	double delta;
	int fixed;
	int degree;
	} agent;

gsl_vector *generate_random_vector(int dim);//self-explanatory

agent *initialize_society(int N, double delta, int dim);//allocates an agent vector with initally random worldview vectors

agent *initialize_society_random_delta(int N, int dim, igraph_t *graph);//allocates an agent vector with initially random worldview vectors and random delta

void learn_from_neighbor(agent *society, igraph_t *graph, int student, gsl_matrix *issues, int issue, int dim,double noise);//Takes a graph, a society, and has the student'th agent in the society learn from one of his neighbors

void get_observables(agent *society, gsl_matrix *issues, int dim, int size, int nissues, int steps);

void print_JJ_histogram(FILE *f, agent *society, int dim, int size, igraph_t *grapho);//Prints an histogram of agent-agent overlaps to a file

void print_rho_histogram(FILE *f, agent *society, int dim, int size, igraph_t *graph, gsl_matrix *issues, int issue);//Prints an histogram of agent-issue overlaps to a file

void initalize_issue_matrix(gsl_matrix *issues, int nissues, int dim, int option);//Initializes a random issue matrix

void agent_free(agent *society, int size);//frees the memory used for a society

int get_average_opinion(agent *society, int dim, int size, gsl_matrix *issues, int issue);//gets the average opinion of the society on a given issue (+-1)

void increment_JJ_histogram(agent *society, int dim, int size, igraph_t *graph, gsl_histogram *H);//Takes a histogram and sums to it all the agent-agent projections for society.

void increment_rho_histogram(agent *society, int dim, int size, igraph_t *graph, gsl_matrix *issues, int issue, gsl_histogram *H);//Takes a histogram and sums to it all the agent-issue projections for society.

agent *initialize_society(int N, double delta, int dim){
	agent *society;
	int i,j, seed;
	double aux;
	
	gsl_rng *r;
	
	gsl_rng_env_setup();
	
	r = gsl_rng_alloc(gsl_rng_default);
	
	seed = time (NULL) * getpid();
    	//seed = 13188839657852;
    	gsl_rng_set(r,seed);
   	
	society = (agent *)malloc(N*sizeof(agent));
	
	for(i = 0; i < N; ++i){
		society[i].delta = delta;
		society[i].J = gsl_vector_alloc(dim);
		society[i].fixed = 0;
		for(j = 0; j < dim; ++j){
			gsl_vector_set(society[i].J,j,gsl_ran_ugaussian(r));
		}
		gsl_blas_ddot(society[i].J,society[i].J,&aux);
		gsl_vector_scale(society[i].J,1.0/sqrt(aux));
	}
	
	gsl_rng_free(r);
	
	return society;
	
}

agent *initalize_society_random_delta(int N, int dim, igraph_t *graph){
	agent *society;
	int i,j, seed;
	double aux;
	
	gsl_rng *r;
	
	gsl_rng_env_setup();
	
	r = gsl_rng_alloc(gsl_rng_default);
	
	seed = time (NULL) * getpid();
    	/*seed = 13188839657852;*/
	gsl_rng_set(r,seed);
	
	society = (agent *)malloc(N*sizeof(agent));
	
	for(i = 0; i < N; ++i){
		society[i].delta = gsl_rng_uniform(r);
		society[i].J = gsl_vector_alloc(dim);
		for(j = 0; j < dim; ++j){
			gsl_vector_set(society[i].J,j,gsl_ran_ugaussian(r));
		}
		gsl_blas_ddot(society[i].J,society[i].J,&aux);
		gsl_vector_scale(society[i].J,1.0/sqrt(aux));
	}
	
	gsl_rng_free(r);
	
	return society;
}

void learn_from_neighbor(agent *society, igraph_t *graph, int student, gsl_matrix *issues, int issue, int dim, double noise){
	int teacher,teacherindex;
	
	double h1,h2,alfa,scale;
	
	if(society[student].fixed==1) return;
	
	gsl_vector *current_issue = gsl_vector_alloc(dim);
	
	igraph_vector_t neighbors;
	
	igraph_vector_init(&neighbors,0);
	
	igraph_neighbors(graph,&neighbors,student, IGRAPH_OUT);
	
	teacherindex = rand()%igraph_vector_size(&neighbors);
	
	teacher =  (int)VECTOR(neighbors)[teacherindex];

	gsl_matrix_get_row(current_issue,issues,issue);
	
	gsl_blas_ddot(society[student].J,current_issue,&h1);
	gsl_blas_ddot(society[teacher].J,current_issue,&h2);

	if((double)rand()/RAND_MAX < noise) h2 *= -1.0;
	
	scale = (h2>0?1.0:-1.0);
	
	//scale = h2;/*Uncomment for linear perceptrons!*/
	
	alfa = (h1*h2>0?society[student].delta:1.0);
		
	gsl_vector_scale(current_issue,eta*scale*alfa/dim);
	
	gsl_vector_add(society[student].J,current_issue);

	gsl_vector_free(current_issue);
	
	igraph_vector_destroy(&neighbors);

	gsl_blas_ddot(society[student].J,society[student].J,&h1);
	gsl_vector_scale(society[student].J,1.0/sqrt(h1));/*Uncomment for normalized perceptrons!*/

}

gsl_matrix *initialize_issue_matrix(int nissues, int dim, int option){
	int i,j, seed;
	
	gsl_matrix *issues;
	
	double mod;
	
	gsl_rng *r;
	
	gsl_vector *temp = gsl_vector_alloc(dim);
	
	gsl_rng_env_setup();
	
	r = gsl_rng_alloc(gsl_rng_default);

	seed = time (NULL) * getpid();
    	//seed = 13188839657852;
    	gsl_rng_set(r,seed);
	
	issues = gsl_matrix_alloc(nissues,dim);
	if(option == 0){
		for(i = 0; i < nissues; ++i){
			for(j = 0; j < dim; ++j){
				gsl_matrix_set(issues,i,j,gsl_ran_ugaussian(r));
			}
		}

		for(i = 0; i < nissues; ++i){
			gsl_matrix_get_row(temp,issues,i);
			gsl_blas_ddot(temp,temp,&mod);
			gsl_vector_scale(temp,1.0/sqrt(mod));
			gsl_matrix_set_row(issues,i,temp);
		}
	}
	else{
		gsl_matrix_set_all(issues,0.0);
		for(i = 0; i < (dim<nissues?dim:nissues); ++i){
			gsl_matrix_set(issues,i,i,1.0);
		}
	}
	
	gsl_vector_free(temp);
	
	gsl_rng_free(r);
	
	return issues;
}

void get_observables(agent *society, gsl_matrix *issues, int dim, int size, int nissues, int steps){
	double M=0.,temp;
	int i,j;
	
	gsl_vector *aux = gsl_vector_alloc(dim);
	
	for(j=0;j<nissues;j++){
		gsl_matrix_get_row(aux,issues,j);
		for(i = 0; i < size; ++i){
			gsl_blas_ddot(aux,society[i].J,&temp);
			M+=temp;
		}
	}
	
	M = M/(nissues*size);
	
	printf("%d %lf\n",steps,M);
}

void agent_free(agent *society, int size){
	int i;
	
	for(i = 0; i < size; ++i){
		gsl_vector_free(society[i].J);
	}
	
	free(society);
	
}
/*
gsl_vector *generate_random_vector(int dim){
	gsl_vector *vec = gsl_vector_alloc(dim);
	gsl_rng *r;
	int i, seed;
	double mod;
		
	gsl_rng_env_setup();

	seed = time (NULL) * getpid();
    	//seed = 13188839657852;
    	gsl_rng_set(r,seed);
    		
	r = gsl_rng_alloc(gsl_rng_default);
	
	for(i = 0; i < dim; ++i){
		gsl_vector_set(vec,i,gsl_ran_ugaussian(r));
	}
	
	gsl_blas_ddot(vec,vec,&mod);
	
	gsl_vector_scale(vec,1.0/sqrt(mod));
	
	gsl_rng_free(r);
	
	return vec;
	
}
*/
void print_JJ_histogram(FILE *f, agent *society, int dim, int size, igraph_t *graph){
	double h,m1,m2;
	
	int NBINS=161,i,j;
	
	gsl_histogram *H = gsl_histogram_alloc(NBINS);	

	gsl_histogram_set_ranges_uniform(H,-1.0,1.0);
	
	for(i = 0; i < size; ++i){
		for(j = i+1; j < size; ++j){
			gsl_blas_ddot(society[i].J,society[j].J,&h);
			gsl_blas_ddot(society[i].J,society[i].J,&m1);
			gsl_blas_ddot(society[j].J,society[j].J,&m2);
			gsl_histogram_increment(H,h/sqrt(m1*m2));	
		}
	}
	gsl_histogram_scale(H,1.0/gsl_histogram_max_val(H));
	
	gsl_histogram_fprintf(f,H,"%lf","%lf");

	gsl_histogram_free(H);
	
}


void print_rho_histogram(FILE *f, agent *society, int dim, int size, igraph_t *graph, gsl_matrix *issues, int issue){
	gsl_vector *v = gsl_vector_alloc(dim);
	
	double h,m1;
	
	int NBINS=81,i,avg_op;
	
	gsl_histogram *H = gsl_histogram_alloc(NBINS);	

	gsl_histogram_set_ranges_uniform(H,-1.0,1.0);
	
	gsl_matrix_get_row(v,issues,issue);
	
	avg_op = get_average_opinion(society,dim,size,issues,issue);
	
	for(i = 0; i < size; ++i){

		gsl_blas_ddot(society[i].J,v,&h);
		gsl_blas_ddot(society[i].J,society[i].J,&m1);
		gsl_histogram_increment(H,avg_op*h/sqrt(m1));	

	}
	gsl_histogram_scale(H,1.0/gsl_histogram_max_val(H));
	
	gsl_histogram_fprintf(f,H,"%lf","%lf");
	
	gsl_vector_free(v);
	gsl_histogram_free(H);
	
}

void increment_rho_histogram( agent *society, int dim, int size, igraph_t *graph, gsl_matrix *issues, int issue, gsl_histogram *H){
	gsl_vector *v = gsl_vector_alloc(dim);
	
	double h,m1;
	
	int i,avg_op;

	gsl_matrix_get_row(v,issues,issue);
	
	avg_op = get_average_opinion(society,dim,size,issues,issue);
	
	for(i = 0; i < size; ++i){

		gsl_blas_ddot(society[i].J,v,&h);
		gsl_blas_ddot(society[i].J,society[i].J,&m1);
		gsl_histogram_increment(H,avg_op*h/sqrt(m1));	

	}
	
	gsl_vector_free(v);
	
}


void increment_JJ_histogram( agent *society, int dim, int size, igraph_t *graph, gsl_histogram *H){
	double h,m1,m2;
	
	int i,j;
	
	for(i = 0; i < size; ++i){
		for(j = i; j < size; ++j){
			gsl_blas_ddot(society[i].J,society[j].J,&h);
			gsl_blas_ddot(society[i].J,society[i].J,&m1);
			gsl_blas_ddot(society[j].J,society[j].J,&m2);
			gsl_histogram_increment(H,h/sqrt(m1*m2));	
		}
	}
	
}


int get_average_opinion(agent *society, int dim, int size, gsl_matrix *issues, int issue){
	double aux;
	int temp=0,i;

	gsl_vector *t = gsl_vector_alloc(dim);
	
	gsl_matrix_get_row(t,issues,issue);
	
	for(i=0;i<size;i++){
		gsl_blas_ddot(society[i].J,t,&aux);
		temp+=(aux>0.0?1:-1);
	}
	
	gsl_vector_free(t);
	
	return (temp>0?1:-1);

}

void help(){
	printf("\nCorrect usage: ./society size delta noise dim nissues network rp mcsteps histsteps\n");
	printf("\tsize: number of agents in network = size*size.\n");
	printf("\tdelta: novelty-seeking behavior parameter. Negative values result in random delta.\n");
	printf("\tnoise: noise parameter (between 0 and 1)\n");
	printf("\tdim: dimension of the moral matrices.\n");
	printf("\tnissues: number of issues being discussed.\n");
	printf("\tnetwork:\n\t\t1 - square lattice with rewiring probability rp\n\t\t2 - barabasi-alberts graph\n\t\t3 - small-world graph\n");
	printf("\trp: rewiring probability (only used if network = 1)\n");
	printf("\tmcsteps: number of \"monte carlo\" steps\n");
	printf("\thiststeps: number of mc steps for each histogram\n");
}
