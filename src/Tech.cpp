#include "Tech.h"

Layer::Layer() {
	name = "";
	major = 0;
	minor = 0;
}

Layer::Layer(string name, int major, int minor) {
	this->name = name;
	this->major = major;
	this->minor = minor;
}

Layer::~Layer() {
}

Model::Model() {
	name = "";
	type = NMOS;
}

Model::Model(int type, string name) {
	this->name = name;
	this->type = type;
}

Model::~Model() {
}

Tech::Tech() {
	dbunit = 5e-9;

	// TODO(edward.bingham) hardcoding tech configuration values for now, but
	// this should be parsed in from python
	/*layers.push_back(Layer("diff.drawing", 65, 20));
	layers.push_back(Layer("poly.drawing",*/ 

	models.push_back(Model(Model::NMOS, "sky130_fd_pr__nfet_01v8"));
	models.push_back(Model(Model::PMOS, "sky130_fd_pr__pfet_01v8"));
}

Tech::~Tech() {
}

int Tech::findLayer(string name) const {
	for (int i = 0; i < (int)layers.size(); i++) {
		if (layers[i].name == name) {
			return i;
		}
	}

	return -1;
}

int Tech::findModel(string name) const {
	for (int i = 0; i < (int)models.size(); i++) {
		if (models[i].name == name) {
			return i;
		}
	}

	return -1;
} 
