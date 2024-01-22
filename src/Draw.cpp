#include "Draw.h"

void drawTransistor(const Tech &tech, Layout &layout, const Mos &mos, vec2i pos, vec2i dir) {
	vec2i ll = pos;
	vec2i ur = pos + mos.size*dir;

	// draw poly
	vec2i polyOverhang = vec2i(0, tech.models[mos.model].polyOverhang)*dir;
	layout.updateBox(ll - polyOverhang, ur + polyOverhang);
	layout.push(tech.wires[0], Rect(mos.ports[Mos::GATE], ll - polyOverhang, ur + polyOverhang));
	
	// draw diffusion
	for (auto layer = tech.models[mos.model].mats.begin(); layer != tech.models[mos.model].mats.end(); layer++) {
		vec2i diffOverhang = vec2i(layer->overhangX, layer->overhangY)*dir;
		ll -= diffOverhang;
		ur += diffOverhang;
		if (layer == tech.models[mos.model].mats.begin()) {
			layout.updateBox(ll, ur);
		}
		layout.push(layer->layer, Rect(-1, ll, ur));
	}
}

void drawVia(const Tech &tech, Layout &layout, int net, int downLevel, int upLevel, vec2i size, vec2i pos, vec2i dir) {
	// layers
	vector<int> vias = tech.findVias(downLevel, upLevel);
	for (int i = 0; i < (int)vias.size(); i++) {
		int viaLayer = tech.vias[vias[i]].drawing;
		int downLevel = tech.vias[vias[i]].downLevel;
		int upLevel = tech.vias[vias[i]].upLevel;

		// spacing and width of a via
		int viaWidth = tech.mats[viaLayer].minWidth;
		int viaSpacing = tech.mats[viaLayer].minSpacing;

		// enclosure rules
		vec2i dn(tech.vias[vias[i]].downLo, tech.vias[vias[i]].downHi);
		vec2i up(tech.vias[vias[i]].upLo, tech.vias[vias[i]].upHi);

		vec2i num = 1 + (size-viaWidth - 2*dn[0]) / (viaSpacing + viaWidth);
		vec2i width = num * viaWidth + (num-1)*viaSpacing;

		vec2i off(0,0);
		if (size[0] > width[0]) {
			off[0] = (size[0] - width[0])/2;
		}
		if (size[1] > width[1]) {
			off[1] = (size[1] - width[1])/2;
		}

		if ((downLevel >= 0 or off[1] >= dn[1]) and off[0] < dn[1]) {
			swap(dn[0], dn[1]);
		}
		if (downLevel >= 0 and off[0] < up[1]) {
			swap(up[0], up[1]);
		}
		
		//if (off[1] < max(dn[1], up[1])) {
		//	off[1] = max(dn[1], up[1]);
		//}

		if (downLevel < 0) {
			dn[1] = off[1];
		}

		// draw down
		vec2i ll = pos+(off-dn)*dir;
		vec2i ur = pos+(off+width+dn)*dir;
		if (downLevel >= 0) {
			// routing level
			layout.updateBox(ll, ur);
			layout.push(tech.wires[downLevel].drawing, Rect(-1, ll, ur));
		} else {
			// diffusion level
			int model = -downLevel-1;
			for (int i = 0; i < (int)tech.models[model].mats.size(); i++) {
				if (i != 0) {
					ll[0] -= tech.models[model].mats[i].overhangX*dir[0];
					ll[1] -= tech.models[model].mats[i].overhangY*dir[1];
					ur[0] += tech.models[model].mats[i].overhangX*dir[0];
					ur[1] += tech.models[model].mats[i].overhangY*dir[1];
				}
				if (i == 0) {
					layout.updateBox(ll, ur);
				}
				layout.push(tech.models[model].mats[i].layer, Rect(-1, ll, ur));
			}
		}

		// draw via
		vec2i idx;
		int step = viaWidth+viaSpacing;
		for (idx[0] = 0; idx[0] < num[0]; idx[0]++) {
			for (idx[1] = 0; idx[1] < num[1]; idx[1]++) {
				layout.push(viaLayer, Rect(-1, pos+(off+idx*step)*dir, pos+(off+idx*step+viaWidth)*dir));
			}
		}

		// draw up
		ll = pos+(off-up)*dir;
		ur = pos+(off+width+up)*dir;
		if (upLevel >= 0) {
			// routing level
			layout.updateBox(ll, ur);
			layout.push(tech.wires[upLevel].drawing, Rect(-1, ll, ur));
		} else {
			// diffusion level
			int model = -upLevel-1;
			for (int i = 0; i < (int)tech.models[model].mats.size(); i++) {
				if (i != 0) {
					ll[0] -= tech.models[model].mats[i].overhangX*dir[0];
					ll[1] -= tech.models[model].mats[i].overhangY*dir[1];
					ur[0] += tech.models[model].mats[i].overhangX*dir[0];
					ur[1] += tech.models[model].mats[i].overhangY*dir[1];
				}
				if (i == 0) {
					layout.updateBox(ll, ur);
				}
				layout.push(tech.models[model].mats[i].layer, Rect(-1, ll, ur));
			}
		}
	}
}

void drawWire(const Tech &tech, Layout &layout, const Solution *ckt, const Wire &wire, vec2i pos, vec2i dir) {
	vec2i ll = pos+vec2i(wire.left,wire.inWeight)*dir;
	vec2i ur = pos+vec2i(wire.right,wire.inWeight+wire.height)*dir;

	layout.updateBox(ll, ur);
	layout.push(tech.wires[wire.layer], Rect(wire.net, ll, ur));

	for (auto pin = wire.pins.begin(); pin != wire.pins.end(); pin++) {
		int level = ckt->pin(*pin).layer;
		int layer = tech.wires[level].drawing;
		int height = ckt->pin(*pin).height;
		vec2i pp(ckt->pin(*pin).pos, 0);
		vec2i ps(tech.mats[layer].minWidth,height/2);
		if (pin->type == Model::NMOS) {
			pp[1] = -ckt->cellHeight + height;
		}

		drawVia(tech, layout, wire.net, level, 2, vec2i(0, wire.height), vec2i(pp[0], ll[1]), dir);
		layout.push(tech.wires[level], Rect(wire.net, vec2i(pp[0], ll[1]+(wire.height/2)*dir[1]), pp+ps*dir));
	}
}

void drawCell(const Tech &tech, Layout &layout, const Solution *ckt) {
	layout.name = ckt->base->name;

	layout.nets.reserve(ckt->base->nets.size());
	for (int i = 0; i < (int)ckt->base->nets.size(); i++) {
		layout.nets.push_back(ckt->base->nets[i].name);
	}

	for (int type = 0; type < 2; type++) {
		for (auto pin = ckt->stack[type].begin(); pin != ckt->stack[type].end(); pin++) {
			vec2i pos(pin->pos, 0);
			vec2i dir(1,-1);
			if (type == Model::NMOS) {
				pos[1] = -ckt->cellHeight + pin->height;
			}

			if (pin->device < 0) {
				int model = -1;
				int i = pin-ckt->stack[type].begin();
				if (i-1 >= 0 and ckt->stack[type][i-1].device >= 0) {
					model = ckt->base->mos[ckt->stack[type][i-1].device].model;
				} else if (i+1 < (int)ckt->stack[type].size() and ckt->stack[type][i+1].device >= 0) {
					model = ckt->base->mos[ckt->stack[type][i+1].device].model;
				}

				if (model >= 0) {
					drawVia(tech, layout, pin->outNet, -model-1, 1, vec2i(pin->width, pin->height), pos, dir);
				}
			} else {
				drawTransistor(tech, layout, ckt->base->mos[pin->device], pos, dir);
			}
		}
	}

	vec2i pos(0,0);
	vec2i dir(1,-1);
	for (int i = 0; i < (int)ckt->routes.size(); i++) {
		drawWire(tech, layout, ckt, ckt->routes[i], pos, dir);
	}

	layout.merge();
}

