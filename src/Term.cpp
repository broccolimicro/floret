#include "Term.h"

Gate::Gate()
{
}

Gate::Gate(int net, int width, int length)
{
	this->net = net;
	this->width = width;
	this->length = length;
}

Gate::~Gate()
{
}

Term::Term(int model, int gate, int source, int drain, int bulk, int width, int length)
{
	this->gate.push_back(Gate(gate, width, length));
	this->source = source;
	this->drain = drain;
	this->bulk = bulk;

	this->model = model;

	this->selected = 0;
}

Term::~Term()
{
}
