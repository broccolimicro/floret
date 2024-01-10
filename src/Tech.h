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

	// these index into Tech::layers
	vector<int> diffLayers;
	vector<int> polyLayers;
	vector<int> contactLayers;
};

struct Tech {
	Tech();
	~Tech();
	
	double dbunit;

	vector<Layer> layers;
	vector<Model> models;

	int findLayer(string name) const;
	int findModel(string name) const;
};

