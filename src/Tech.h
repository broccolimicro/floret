#pragma once

#include <string>
#include <vector>

using namespace std;

struct Layer {
	Layer();
	Layer(string name, int major = 0, int minor = 0);
	~Layer();

	string name;
	int major;
	int minor;

	int minSpacing;
	int minWidth;
};

struct Diffusion {
	Diffusion();
	Diffusion(int layer, int overhangX, int overhangY);
	~Diffusion();

	// these index into Tech::layers
	int layer;

	int overhangX;
	int overhangY;
};

struct Model {
	Model();
	Model(int type, string name);
	~Model();

	enum {
		NMOS = 0,
		PMOS = 1,
	};
	int type;
	
	string name;

	// Start top down
	vector<Diffusion> layers;
};

struct Wire {
	Wire();
	Wire(int drawing, int pin, int label);
	~Wire();
	
	// index into Tech::layers
	int drawingLayer;
	int pinLayer;
	int labelLayer;
};

struct Via {
	Via();
	Via(int from, int to, int layer, int downLo = 0, int downHi = 0, int upLo = 0, int upHi = 0);
	~Via();

	// index into Tech::layers
	int from;
	int to;
	int drawingLayer;

	int downLo;
	int downHi;
	int upLo;
	int upHi;
};

struct Tech {
	Tech();
	~Tech();
	
	double dbunit;

	int boundary;
	vector<Layer> layers;
	vector<Model> models;
	vector<Via> vias;
	vector<Wire> wires;
	

	int findLayer(string name) const;
	int findModel(string name) const;
};

