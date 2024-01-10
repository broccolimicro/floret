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
}

Model *Tech::findModel(string name) {
	for (auto model = models.begin(); model != models.end(); model++) {
		if (model->name == name) {
			return &(*model);
		}
	}

	return nullptr;
} 
