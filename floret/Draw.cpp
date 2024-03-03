#include "Draw.h"

void drawDiffusion(const Tech &tech, Layout &dst, int model, int net, vec2i ll, vec2i ur, vec2i dir) {
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

void drawVia(const Tech &tech, Layout &dst, int net, int viaLevel, vec2i axis, vec2i size, bool expand, vec2i pos, vec2i dir) {
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

	if (expand) {
		if (downLevel >= 0) {
			dn = max(dn, off);
		}
		if (upLevel >= 0) {
			up = max(up, off);
		}
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

void drawViaStack(const Tech &tech, Layout &dst, int net, int downLevel, int upLevel, vec2i axis, vec2i size, vec2i pos, vec2i dir) {
	if (downLevel == upLevel) {
		int layer = tech.wires[downLevel].draw;
		int width = tech.paint[layer].minWidth;
		size[0] = max(size[0], width);
		size[1] = max(size[1], width);
		dst.push(tech.wires[downLevel], Rect(net, pos, pos+size*dir));
		return;
	}

	vector<int> vias = tech.findVias(downLevel, upLevel);
	for (int i = 0; i < (int)vias.size(); i++) {
		drawVia(tech, dst, net, vias[i], axis, size, true, pos, dir);
	}
}

void drawWire(const Tech &tech, Layout &dst, const Circuit *ckt, const Wire &wire, vec2i pos, vec2i dir) {
	// [via level][pin]
	vector<vector<int> > posArr;
	posArr.resize(tech.vias.size());

	for (int i = 0; i < (int)tech.vias.size(); i++) {
		posArr[i].reserve(wire.pins.size());

		for (int j = 0; j < (int)wire.pins.size(); j++) {
			const Pin &pin = ckt->pin(wire.pins[j].idx);

			int pinLevel = pin.layer;
			int prevLevel = wire.getLevel(j-1);
			int nextLevel = wire.getLevel(j);

			int viaPos = pin.pos;
			if (pinLevel != prevLevel or pinLevel != nextLevel) {
				viaPos = clamp(viaPos, wire.pins[j].left, wire.pins[j].right);
			}
			if (wire.pins[j].left > wire.pins[j].right) {
				printf("error: pin violation on pin %d\n", j);
				printf("pinPos=%d left=%d right=%d viaPos=%d\n", pin.pos, wire.pins[j].left, wire.pins[j].right, viaPos);
			}

			posArr[i].push_back(viaPos);
		}
	}

	for (int i = 0; i < (int)tech.vias.size(); i++) {
		vector<Layout> vias;
		vias.reserve(wire.pins.size());
		int height = 0;
		for (int j = 0; j < (int)wire.pins.size(); j++) {
			const Pin &pin = ckt->pin(wire.pins[j].idx);
			int pinLevel = pin.layer;
			int prevLevel = wire.getLevel(j-1);
			int nextLevel = wire.getLevel(j);
			int wireLow = min(nextLevel, prevLevel);
			int wireHigh = max(nextLevel, prevLevel);

			int wireLayer = tech.wires[nextLevel].draw;
			height = tech.paint[wireLayer].minWidth;

			if ((pinLevel <= tech.vias[i].downLevel and wireHigh >= tech.vias[i].upLevel) or
			    (wireLow <= tech.vias[i].downLevel and pinLevel >= tech.vias[i].upLevel)) {
				int width = tech.paint[tech.wires[pinLevel].draw].minWidth;
				dst.push(tech.wires[pinLevel], Rect(wire.net, vec2i(pin.pos, 0), vec2i(posArr[i][j], width)));

				vec2i axis(0,0);
				//if (wireLow <= tech.vias[i].downLevel and wireHigh >= tech.vias[i].downLevel and j > 0 and j < (int)wire.pins.size()-1) {
				//	axis[0] = 0;
				//}
				//if (wireLow <= tech.vias[i].upLevel and wireHigh >= tech.vias[i].upLevel and j > 0 and j < (int)wire.pins.size()-1) {
				//	axis[1] = 0;
				//}

				Layout next;
				drawVia(tech, next, wire.net, i, axis, vec2i(width, height), true, vec2i(posArr[i][j], 0));
				int off = numeric_limits<int>::min();
				if (not vias.empty() and minOffset(&off, tech, 0, vias.back().layers, 0, next.layers, 0, Layout::IGNORE, Layout::DEFAULT) and off > 0) {
					Rect box = vias.back().box.bound(next.box);
					vias.back().clear();
					drawVia(tech, vias.back(), wire.net, i, axis, vec2i(box.ur[0]-box.ll[0], height), true, vec2i(box.ll[0], 0));
				} else {
					vias.push_back(next);
				}
			}
		}

		for (int i = 0; i < (int)vias.size(); i++) {
			drawLayout(dst, vias[i], pos, dir);
		}
	}

	for (int i = 1; i < (int)wire.pins.size(); i++) {
		int prevLevel = wire.getLevel(i-1);
		int nextLevel = wire.getLevel(i);

		int left = numeric_limits<int>::min();
		int right = numeric_limits<int>::max();
		for (int j = 0; j < (int)tech.vias.size(); j++) {
			if (tech.vias[j].downLevel == prevLevel or tech.vias[j].upLevel == prevLevel) {
				left = max(left, posArr[j][i-1]);
			}
			if (tech.vias[j].downLevel == nextLevel or tech.vias[j].upLevel == nextLevel) {
				right = min(right, posArr[j][i]);
			}
		}

		int height = tech.paint[tech.wires[prevLevel].draw].minWidth;
		vec2i ll = pos+vec2i(left, 0)*dir;
		vec2i ur = pos+vec2i(right, height)*dir;
		dst.push(tech.wires[prevLevel], Rect(wire.net, ll, ur));
	}
}

void drawPin(const Tech &tech, Layout &dst, const Circuit *ckt, const Stack &stack, int pinID, vec2i pos, vec2i dir) {
	pos[0] += stack.pins[pinID].pos;
	if (stack.pins[pinID].device < 0) {
		int model = -1;
		for (int i = pinID-1; i >= 0 and model < 0; i--) {
			if (stack.pins[i].device >= 0) {
				model = ckt->mos[stack.pins[i].device].model;
			}
		}

		for (int i = pinID+1; i < (int)stack.pins.size() and model < 0; i++) {
			if (stack.pins[i].device >= 0) {
				model = ckt->mos[stack.pins[i].device].model;
			}
		}

		if (model >= 0) {
			drawViaStack(tech, dst, stack.pins[pinID].outNet, -model-1, 1, vec2i(1,1), vec2i(stack.pins[pinID].width, stack.pins[pinID].height), pos, dir);
		} else {
			printf("error: unable to identify transistor model for diffusion contact %d\n", pinID);
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

