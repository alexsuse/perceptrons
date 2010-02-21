#!/usr/bin/env python2.6

import os
import multiprocessing
from multiprocessing import Process, Queue
from Queue import Empty

def runAgents(q):
	while True:
            try:
                x = q.get(block=False)
			
                [L,delta,noise,dim,P,network,RP,MCSTEPS,histsteps] = x
		
		name = './sociedade '+str(L)+' '+str(delta)+' '+str(noise)+' '+str(dim)+' '+str(P)+' '+str(network)+' '+str(RP)+' '+str(MCSTEPS)+' '+str(histsteps)
		print name
		os.system(name)
			
                pr = 'curl -u zepellinusp:zepellin -d status\" running '+name+'\" http://www.twitter.com/statuses/update.xml'
            except Empty:
                break	
             
if __name__ == '__main__':

	#cria nthreads (=numero de cores)
	nthreads=multiprocessing.cpu_count()
	
	print "Running on "+str(nthreads)+" cores.\n"

	#parametros fixos
	D=5  		 # dimensao da matriz moral
        RP=0.00		 # branching
	L=20		 # tamanho da rede quadrada LxL
        MCSTEPS= 3000000	 #numero de passos de montecarlo
        histsteps = 100000      #tamanho do intervalo para histogramas sucessivos
        network = 1;

	#prepara fila de tarefas
	work_queue = Queue()
	noise=0.0
	for l in range (0,3,1):
                for P in range(1,5,1): 			
	     	        for k in range(0,10,1):
                                  delta= float(k)/10			
                             	  name = 'histograma_agents_P:'+str(P)+'_delta:'+str(delta)+'_noise:'+str(noise)+'_net:'+str(network)+'_MCsteps:'+str(MCSTEPS)+'_JJ'
				  list1 = os.listdir('./')
				  list2 = os.listdir('./JJ/')
				  if not name in list1 and not name in list2:
				  	work_queue.put([L,delta,noise,D,P,network,RP,MCSTEPS,histsteps])
                noise=noise+0.05

	#multiprocessamento
	processes = [Process(target=runAgents, args=(work_queue,)) for i in range(nthreads)]		
	
	for p in processes:
		p.start()
	for p in processes:
		p.join()

	 #cria diretorio para dados	
   	lista= os.listdir(os.getcwd())
     	if not ('JJ' in lista):os.system('mkdir JJ')
    	if not ('rho4' in lista):os.system('mkdir rho4')
    	if not ('rho5' in lista):os.system('mkdir rho5')
    	if not ('rho1' in lista):os.system('mkdir rho1')
   	if not ('rho2' in lista):os.system('mkdir rho2')
   	if not ('rho3' in lista):os.system('mkdir rho3')
    
    	#transfere resultados
    	os.system('mv *JJ JJ')
    	os.system('mv *rho1 rho1')
    	os.system('mv *rho2 rho2')
    	os.system('mv *rho3 rho3')
    	os.system('mv *rho4 rho4')
    	os.system('mv *rho5 rho5')
	
