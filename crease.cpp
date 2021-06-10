#include "crease.h"

Crease::Crease():
        Sigma(0.),
        Next(0),
        Prev(0)
{}

Crease::Crease(float Sigma,int Next,int Prev):
		Sigma(Sigma),
		Next(Next),
		Prev(Prev)
	{}
