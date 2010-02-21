#!/usr/bin/python

import string
import os
import re

def getphase(stra,P):
	fname = './rho1/'+stra
	g = open(fname,'r')
	c1 = g.read()
	g.close()
	if P==1:
		return checkphase(c1)
	p = re.compile('rho1')
	fname2 = p.sub('rho2',stra)
	fn = './rho2/'+fname2
	g = open(fn,'r')
	c2=g.read()
	g.close()
	if P==2:
		return checkphase(c1) or checkphase(c2)
	fname3 = p.sub('rho3',stra)
	fn = './rho3/'+fname3
	g = open(fn,'r')
	c3 = g.read()
	g.close()
	if P==3:
		return checkphase(c1) or checkphase(c2) or checkphase(c3)
	fname4 = p.sub('rho4',stra)
	fn = './rho4/'+fname4
	g = open(fn,'r')
	c4 = g.read()
	g.close()
	if P==4:
		return checkphase(c1) or checkphase(c2) or checkphase(c3) or checkphase(c4)
	fname5 = p.sub('rho5',stra)
	fn = './rho5/'+fname5
	g = open(fn,'r')
	c5 = g.read()
	g.close()
	if P==5:
		return checkphase(c1) or checkphase(c2) or checkphase(c3) or checkphase(c4) or checkphase(c5)
	
def getparams(stra):
	a = stra.split('_')
	for i in a:
		b = i.split(':')
		if b[0] == 'noise':
			noise = float(b[1])
		if b[0] == 'delta':
			delta = float(b[1])
		if b[0] == 'P':
			P = int(b[1])
		if b[0] == 'MCsteps':
			MCsteps = int(b[1])
	return [delta,P,noise,MCsteps]
	
	
def checkphase(stra):
	d = stra.split('\n')
	d = map(string.split,d)
	c = d[:(len(d)-1)]
	scores = map(thirdelement,c)
	total = reduce(lambda x,y:float(x)+float(y) , scores)
	aux =0.0
	for	i in c:
		aux+=float(i[2])*0.5*(float(i[0])+float(i[1]))
	if abs(aux/total)>0.05:
		return True
	return False
	
def firstelement(arg):
	return arg[0]	
	
def secondelement(arg):
	return arg[1]	

def thirdelement(arg):
	return arg[2]
	

lista = os.listdir('./rho1/')

reg = re.compile('histograma_agents')
P1 = []
P2 = []
P3 = []
P4 = []
P5 = []

for i in lista:
	if reg.match(i):
		[delta,P,noise,MCSTEPS]=getparams(i)
		if P==1:
		 	P1.append(str(P)+' '+str(delta) +' '+str(noise)+' '+str(MCSTEPS)+' '+str(int(getphase(i,P)))+'\n')
		if P==2:
			P2.append(str(P)+' '+str(delta) +' '+str(noise)+' '+str(MCSTEPS)+' '+str(int(getphase(i,P)))+'\n')
		if P==3:
			P3.append(str(P)+' '+str(delta) +' '+str(noise)+' '+str(MCSTEPS)+' '+str(int(getphase(i,P)))+'\n')
		if P==4:
			P4.append(str(P)+' '+str(delta) +' '+str(noise)+' '+str(MCSTEPS)+' '+str(int(getphase(i,P)))+'\n')
		if P==5:
			P5.append(str(P)+' '+str(delta) +' '+str(noise)+' '+str(MCSTEPS)+' '+str(int(getphase(i,P)))+'\n')

f = open("fase-1-issue","w")
for i in P1:
	f.write(i)
f.close()

f = open("fase-2-issue","w")
for i in P2:
	f.write(i)
f.close()

f = open("fase-3-issue","w")
for i in P3:
	f.write(i)
f.close()

f = open("fase-4-issue","w")
for i in P4:
	f.write(i)
f.close()

f = open("fase-5-issue","w")
for i in P5:
	f.write(i)
f.close()

