#include "Draw.h"

void drawTransistor(const Tech &tech, Layout &dst, const Mos &mos, bool flip, vec2i pos, vec2i dir) {
	vec2i ll = pos;
	vec2i ur = pos + mos.size*dir;

	// draw poly
	vec2i polyOverhang = vec2i(0, tech.models[mos.model].polyOverhang)*dir;
	dst.box.bound(ll - polyOverhang, ur + polyOverhang);
	dst.push(tech.wires[0], Rect(mos.gate, ll - polyOverhang, ur + polyOverhang));

	// draw diffusion
	for (auto layer = tech.models[mos.model].paint.begin(); layer != tech.models[mos.model].paint.end(); layer++) {
		vec2i diffOverhang = layer->overhang*dir;
		ll -= diffOverhang;
		ur += diffOverhang;
		bool isDiffusion = layer == tech.models[mos.model].paint.begin();
		if (isDiffusion) {
			dst.box.bound(ll, ur);
		}
		dst.push(*layer, Rect(-1, ll, ur));
	}
}

void drawVia(const Tech &tech, Layout &dst, int net, int viaLevel, vec2i axis, vec2i size, vec2i pos, vec2i dir) {
	int viaLayer = tech.vias[viaLevel].draw;
	int downLevel = tech.vias[viaLevel].downLevel;
	int upLevel = tech.vias[viaLevel].upLevel;

	// spacing and width of a via
	int viaWidth = tech.paint[viaLayer].minWidth;
	int viaSpacing = tech.findSpacing(viaLayer, viaLayer);

	// enclosure rules and default orientation
	vec2i dn = tech.vias[viaLevel].dn;
	if (axis[0] == 0) {
		dn.swap(0,1);
	}
	vec2i up = tech.vias[viaLevel].up;
	if (axis[1] == 0) {
		up.swap(0,1);
	}
	
	vec2i num = max(1 + (size-viaWidth - 2*dn) / (viaSpacing + viaWidth), 1);
	vec2i width = num * viaWidth + (num-1)*viaSpacing;
	vec2i off = max((size-width)/2, 0);

	// Special rule for diffusion vias
	if (downLevel < 0) {
		if (off[1] >= dn[1] and off[0] < dn[1]) {
			dn.swap(0,1);
		}
		dn[1] = off[1];
	} else if (off[axis[0]] < dn[axis[0]] and off[1-axis[0]] >= dn[axis[0]]) {
		dn.swap(0,1);
	}

	if (off[axis[1]] < up[axis[1]] and off[1-axis[1]] >= up[axis[1]]) {
		up.swap(0,1);
	}

	if (downLevel >= 0) {
		dn = max(dn, off);
	}
	if (upLevel >= 0) {
		up = max(up, off);
	}

	// draw down
	vec2i ll = pos+(off-dn)*dir;
	vec2i ur = pos+(off+width+dn)*dir;
	if (downLevel >= 0) {
		// routing level
		dst.box.bound(ll, ur);
		dst.push(tech.wires[downLevel], Rect(net, ll, ur));
	} else {
		// diffusion level
		int model = -downLevel-1;
		for (int i = 0; i < (int)tech.models[model].paint.size(); i++) {
			if (i != 0) {
				ll -= tech.models[model].paint[i].overhang*dir;
				ur += tech.models[model].paint[i].overhang*dir;
			}
			if (i == 0) {
				dst.box.bound(ll, ur);
			}
			dst.push(tech.models[model].paint[i], Rect(-1, ll, ur));
		}
	}

	// draw via
	vec2i idx;
	int step = viaWidth+viaSpacing;
	for (idx[0] = 0; idx[0] < num[0]; idx[0]++) {
		for (idx[1] = 0; idx[1] < num[1]; idx[1]++) {
			dst.push(tech.vias[viaLevel], Rect(net, pos+(off+idx*step)*dir, pos+(off+idx*step+viaWidth)*dir));
		}
	}

	// draw up
	ll = pos+(off-up)*dir;
	ur = pos+(off+width+up)*dir;
	if (upLevel >= 0) {
		// routing level
		dst.box.bound(ll, ur);
		dst.push(tech.wires[upLevel], Rect(net, ll, ur));
	} else {
		// diffusion level
		int model = -upLevel-1;
		for (int i = 0; i < (int)tech.models[model].paint.size(); i++) {
			if (i != 0) {
				ll -= tech.models[model].paint[i].overhang*dir;
				ur += tech.models[model].paint[i].overhang*dir;
			}
			if (i == 0) {
				dst.box.bound(ll, ur);
			}
			dst.push(tech.models[model].paint[i], Rect(-1, ll, ur));
		}
	}
}

void drawViaStack(const Tech &tech, Layout &dst, int net, int downLevel, int upLevel, vec2i size, vec2i pos, vec2i dir) {
	vector<int> vias = tech.findVias(downLevel, upLevel);
	for (int i = 0; i < (int)vias.size(); i++) {
		drawVia(tech, dst, net, vias[i], vec2i(1,1), size, pos, dir);
	}
}

void drawWire(const Tech &tech, Layout &dst, const Circuit *ckt, const Wire &wire, vec2i pos, vec2i dir) {
	int prevPos = 0;
	Layout prevLayout;
	Layout nextLayout;
	Layout levelLayout;
	for (int i = 0; i < (int)tech.vias.size(); i++) {
		bool nextToDraw = false;
		int from = 0;
		int to = 0;

		int height = 0;
		for (int j = 0; j < (int)wire.pins.size(); j++) {
			const Pin &pin = ckt->pin(wire.pins[j]);
			int pinLevel = pin.layer;
			int prevLevel = wire.getLevel(j-1);
			int nextLevel = wire.getLevel(j);
			int wireLow = min(nextLevel, prevLevel);
			int wireHigh = max(nextLevel, prevLevel);

			int wireLayer = tech.wires[nextLevel].draw;
			height = tech.paint[wireLayer].minWidth;
			if ((pinLevel <= tech.vias[i].downLevel and wireHigh >= tech.vias[i].upLevel) or
			    (wireLow <= tech.vias[i].downLevel and pinLevel >= tech.vias[i].upLevel)) {
				int pinLayer = tech.wires[pinLevel].draw;
				int width = tech.paint[pinLayer].minWidth;
				vec2i axis(0,0);
				/*if (wireLow <= tech.vias[i].downLevel and wireHigh >= tech.vias[i].downLevel and j > 0 and j < (int)wire.pins.size()-1) {
					axis[0] = 0;
				}
				if (wireLow <= tech.vias[i].upLevel and wireHigh >= tech.vias[i].upLevel and j > 0 and j < (int)wire.pins.size()-1) {
					axis[1] = 0;
				}*/

				drawVia(tech, nextLayout, wire.net, i, axis, vec2i(width, height));
				if (j != 0) {
					//printf("mid pin.pos=%d from=%d to=%d nextToDraw=%d\n", pin.pos, from, to, nextToDraw);
					int off = 0;
					if (not minOffset(&off, tech, 0, prevLayout.layers, 0, nextLayout.layers, 0, false) or pin.pos-prevPos >= off) {
						//printf("\tconflict pinOff=%d off=%d nextToDraw=%d\n", pin.pos-prevPos, off, nextToDraw);
						if (nextToDraw) {
							//printf("drawing %d -> %d\n", from, to);
							drawVia(tech, levelLayout, wire.net, i, axis, vec2i(to-from, height), vec2i(from, 0));
						}
						nextToDraw = false;
					} else {
						//printf("\tmerge pinOff=%d off=%d nextToDraw=%d\n", pin.pos-prevPos, off, nextToDraw);
					}
				} else {
					//printf("first pin.pos=%d from=%d to=%d nextToDraw=%d\n", pin.pos, from, to, nextToDraw);
				}
				prevLayout = nextLayout;
				prevPos = pin.pos;
				nextLayout.clear();

				if (not nextToDraw) {
					nextToDraw = true;
					from = pin.pos;
				}
				to = pin.pos + width;
			}
		}

		vec2i axis(0,0);
		//printf("last from=%d to=%d nextToDraw=%d\n", from, to, nextToDraw);
		if (nextToDraw) {
			//printf("drawing %d -> %d\n", from, to);
			drawVia(tech, levelLayout, wire.net, i, axis, vec2i(to-from, height), vec2i(from, 0));
		}

		drawLayout(dst, levelLayout, pos, dir);
		levelLayout.clear();
	}

	for (int i = 0; i < (int)wire.pins.size()-1; i++) {
		const Pin &pin = ckt->pin(wire.pins[i]);
		const Pin &next = ckt->pin(wire.pins[i+1]);
		int pinLevel = next.layer;
		int pinLayer = tech.wires[pinLevel].draw;
		int width = tech.paint[pinLayer].minWidth;

		int wireLevel = wire.getLevel(i);
		int wireLayer = tech.wires[wireLevel].draw;
		int height = tech.paint[wireLayer].minWidth;

		vec2i ll = pos+vec2i(pin.pos,0)*dir;
		vec2i ur = pos+vec2i(next.pos+width,height)*dir;
		dst.box.bound(ll, ur);
		dst.push(tech.wires[wireLevel], Rect(wire.net, ll, ur));
	}

	// This would draw the vias without merging them
	//for (auto pin = wire.pins.begin(); pin != wire.pins.end(); pin++) {
	//	drawLayout(dst, ckt->pin(*pin).conLayout, vec2i(ckt->pin(*pin).pos, ll[1]), dir);
	//}
}

void drawRoute(const Tech &tech, Layout &dst, const Circuit *ckt, const Wire &wire, vec2i pos, vec2i dir) {
	//printf("drawing route %d\n", wire.pOffset);
	drawLayout(dst, wire.layout, vec2i(0, wire.pOffset)*dir, dir);

	for (int i = 0; i < (int)wire.pins.size(); i++) {
		const Pin &pin = ckt->pin(wire.pins[i]);
		int wireLevel = wire.getLevel(i);
		int wireLayer = tech.wires[wireLevel].draw;
		int height = tech.paint[wireLayer].minWidth;

		int pinLevel = pin.layer;
		int pinLayer = tech.wires[pinLevel].draw;
		int width = tech.paint[pinLayer].minWidth;

		int pinMid = (wire.pins[i].type == Model::NMOS)*ckt->cellHeight;

		int top = max(pinMid, wire.pOffset+height);
		int bottom = min(pinMid, wire.pOffset);

		//printf("rect %d %d %d %d,%d %d,%d\n", pin->type, layer, wire.net, left, bottom, right, top);
		dst.push(tech.wires[pinLevel], Rect(wire.net, vec2i(pin.pos, bottom), vec2i(pin.pos+width, top)));
	}
}

void drawPin(const Tech &tech, Layout &dst, const Circuit *ckt, const Stack &stack, int pinID, vec2i pos, vec2i dir) {
	pos[0] += stack.pins[pinID].pos;
	if (stack.pins[pinID].device < 0) {
		int model = -1;
		if (pinID-1 >= 0 and stack.pins[pinID-1].device >= 0) {
			model = ckt->mos[stack.pins[pinID-1].device].model;
		} else if (pinID+1 < (int)stack.pins.size() and stack.pins[pinID+1].device >= 0) {
			model = ckt->mos[stack.pins[pinID+1].device].model;
		}

		if (model >= 0) {
			drawViaStack(tech, dst, stack.pins[pinID].outNet, -model-1, 1, vec2i(stack.pins[pinID].width, stack.pins[pinID].height), pos, dir);
		}
	} else {
		drawTransistor(tech, dst, ckt->mos[stack.pins[pinID].device], stack.pins[pinID].leftNet != ckt->mos[stack.pins[pinID].device].ports[0], pos, dir);
	}
}

void drawLayout(Layout &dst, const Layout &src, vec2i pos, vec2i dir) {
	dst.box.bound(src.box.ll*dir + pos, src.box.ur*dir+pos);
	for (auto layer = src.layers.begin(); layer != src.layers.end(); layer++) {
		auto dstLayer = dst.findLayer(layer->draw);
		for (int i = 0; i < (int)layer->geo.size(); i++) {
			dstLayer->push(layer->geo[i].shift(pos, dir));
		}
	}
}

