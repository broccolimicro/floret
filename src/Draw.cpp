#include "Draw.h"

void drawTransistor(const Tech &tech, Layout &dst, const Mos &mos, bool flip, vec2i pos, vec2i dir) {
	vec2i ll = pos;
	vec2i ur = pos + mos.size*dir;

	vec2i lm(ll[0]+mos.size[0]*dir[0]/2, ll[1]);
	vec2i um(ll[0]+mos.size[0]*dir[0]/2, ll[1]+mos.size[1]*dir[1]);
	
	// draw poly
	vec2i polyOverhang = vec2i(0, tech.models[mos.model].polyOverhang)*dir;
	dst.updateBox(ll - polyOverhang, ur + polyOverhang);
	printf("draw poly %d\n", tech.wires[0].drawing);
	dst.push(tech.wires[0].drawing, Rect(mos.ports[Mos::GATE], ll - polyOverhang, ur + polyOverhang));

	// draw diffusion
	for (auto layer = tech.models[mos.model].mats.begin(); layer != tech.models[mos.model].mats.end(); layer++) {
		vec2i diffOverhang = vec2i(layer->overhangX, layer->overhangY)*dir;
		ll -= diffOverhang;
		ur += diffOverhang;
		lm[1] -= diffOverhang[1];
		um[1] += diffOverhang[1];
		bool isDiffusion = layer == tech.models[mos.model].mats.begin();
		if (isDiffusion) {
			dst.updateBox(ll, ur);
			//dst.push(layer->layer, Rect(mos.ports[flip ? Mos::DRAIN : Mos::SOURCE], ll, um));
			//dst.push(layer->layer, Rect(mos.ports[flip ? Mos::SOURCE : Mos::DRAIN], lm, ur));
		} //else {
			dst.push(layer->layer, Rect(-1, ll, ur));
		//}
	}
}

void drawVia(const Tech &tech, Layout &dst, int net, int downLevel, int upLevel, vec2i size, vec2i pos, vec2i dir) {
	// layers
	vector<int> vias = tech.findVias(downLevel, upLevel);
	for (int i = 0; i < (int)vias.size(); i++) {
		int viaLayer = tech.vias[vias[i]].drawing;
		int downLevel = tech.vias[vias[i]].downLevel;
		int upLevel = tech.vias[vias[i]].upLevel;

		// spacing and width of a via
		int viaWidth = tech.mats[viaLayer].minWidth;
		int viaSpacing = tech.findSpacing(viaLayer, viaLayer);

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
			dst.updateBox(ll, ur);
			dst.push(tech.wires[downLevel].drawing, Rect(net, ll, ur));
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
					dst.updateBox(ll, ur);
					//dst.push(tech.models[model].mats[i].layer, Rect(net, ll, ur));
				} //else {
					dst.push(tech.models[model].mats[i].layer, Rect(-1, ll, ur));
				//}
			}
		}

		// draw via
		vec2i idx;
		int step = viaWidth+viaSpacing;
		for (idx[0] = 0; idx[0] < num[0]; idx[0]++) {
			for (idx[1] = 0; idx[1] < num[1]; idx[1]++) {
				dst.push(viaLayer, Rect(net, pos+(off+idx*step)*dir, pos+(off+idx*step+viaWidth)*dir));
			}
		}

		// draw up
		ll = pos+(off-up)*dir;
		ur = pos+(off+width+up)*dir;
		if (upLevel >= 0) {
			// routing level
			dst.updateBox(ll, ur);
			dst.push(tech.wires[upLevel].drawing, Rect(net, ll, ur));
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
					dst.updateBox(ll, ur);
					//dst.push(tech.models[model].mats[i].layer, Rect(net, ll, ur));
				} //else {
					dst.push(tech.models[model].mats[i].layer, Rect(-1, ll, ur));
				//}
			}
		}
	}
}

void drawWire(const Tech &tech, Layout &dst, const Solution *ckt, const Wire &wire, vec2i pos, vec2i dir) {
	vec2i ll = pos+vec2i(wire.left,wire.inWeight)*dir;
	vec2i ur = pos+vec2i(wire.right,wire.inWeight)*dir;

	dst.updateBox(ll, ur);
	dst.push(tech.wires[wire.layer].drawing, Rect(wire.net, ll, ur));

	for (auto pin = wire.pins.begin(); pin != wire.pins.end(); pin++) {
		int level = ckt->pin(*pin).layer;
		int layer = tech.wires[level].drawing;
		int height = ckt->pin(*pin).height;
		vec2i pp(ckt->pin(*pin).pos, 0);
		vec2i ps(tech.mats[layer].minWidth,height/2);
		if (pin->type == Model::NMOS) {
			pp[1] = -ckt->cellHeight + height;
		}

		// TODO(edward.bingham) check to see if this is actually needed
		int wireHeight = 0;
		drawVia(tech, dst, wire.net, level, 2, vec2i(0, wireHeight), vec2i(pp[0], ll[1]), dir);
		dst.push(tech.wires[level].drawing, Rect(wire.net, vec2i(pp[0], ll[1]+(wireHeight/2)*dir[1]), pp+ps*dir));
	}
}

void drawPin(const Tech &tech, Layout &dst, const Solution *ckt, int type, int pinID, vec2i pos, vec2i dir) {
	pos[0] += ckt->stack[type][pinID].pos;
	if (ckt->stack[type][pinID].device < 0) {
		int model = -1;
		if (pinID-1 >= 0 and ckt->stack[type][pinID-1].device >= 0) {
			model = ckt->base->mos[ckt->stack[type][pinID-1].device].model;
		} else if (pinID+1 < (int)ckt->stack[type].size() and ckt->stack[type][pinID+1].device >= 0) {
			model = ckt->base->mos[ckt->stack[type][pinID+1].device].model;
		}

		if (model >= 0) {
			drawVia(tech, dst, ckt->stack[type][pinID].outNet, -model-1, 1, vec2i(ckt->stack[type][pinID].width, ckt->stack[type][pinID].height), pos, dir);
		}
	} else {
		drawTransistor(tech, dst, ckt->base->mos[ckt->stack[type][pinID].device], ckt->stack[type][pinID].leftNet != ckt->base->mos[ckt->stack[type][pinID].device].ports[Mos::SOURCE], pos, dir);
	}
}

void drawLayout(Layout &dst, const Layout &src, vec2i pos, vec2i dir) {
	dst.updateBox(src.box.ll, src.box.ur);
	for (auto layer = src.layers.begin(); layer != src.layers.end(); layer++) {
		auto dstLayer = dst.findLayer(layer->draw);
		for (int i = 0; i < (int)layer->geo.size(); i++) {
			dstLayer->push(layer->geo[i].shift(pos, dir));
		}
	}
}

