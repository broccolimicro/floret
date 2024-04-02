from floret import *
import sys
import csv
import os

def parseLine(line):
	result = list(csv.reader([line.strip()], delimiter=' ', quotechar='"'))[0]
	for i, elem in enumerate(result):
		if elem.startswith("#"):
			result = result[0:i]
			break

	return [elem.strip() for elem in result if elem.strip()]

def loadActConf(path):
	result = dict()
	stack = [result]
	with open(path, "r") as fptr:
		for number, line in enumerate(fptr):
			args = parseLine(line)
			if len(args) > 0:
				if args[0] == "include":
					result = result | loadActConf(args[1])
				elif args[0] == "begin":
					stack[-1][args[1]] = dict()
					stack.append(stack[-1][args[1]])
				elif args[0] == "end":
					stack.pop()
				elif args[0] == "string":
					stack[-1][args[1]] = args[2]
				elif args[0] == "int":
					stack[-1][args[1]] = int(args[2])
				elif args[0] == "real":
					stack[-1][args[1]] = float(args[2])
				elif args[0] == "int_table":
					stack[-1][args[1]] = [int(arg) for arg in args[2:]]
				elif args[0] == "string_table":
					stack[-1][args[1]] = args[2:]
	return result

confPath = ""
if len(sys.argv) >= 2 and sys.argv[1] != "":
	confPath = sys.argv[1]
elif "ACT_TECH" in os.environ and os.environ["ACT_TECH"] != "":
	confPath = os.environ["ACT_TECH"]
else:
	print("error: expected path to tech file")
	exit(1)
	
print(f"loading configuration from {confPath}")
net = loadActConf(f"{confPath}/prs2net.conf")
if not net:
	print(f"error: {confPath}/prs2net.conf not found")
	exit(1)
layout = loadActConf(f"{confPath}/layout.conf")
if not layout:
	print(f"error: {confPath}/layout.conf not found")
	exit(1)

dbunit(layout["general"]["scale"]*1e-3)

no = -1
layers = {}
for name, major, minor in zip(layout["gds"]["layers"], layout["gds"]["major"], layout["gds"]["minor"]):
	layers[name] = paint(name, major, minor)

mats = layout["materials"]
models = layout["diff"]
modelNames = []
for i, variant in enumerate(models["types"]):
	polyOverhang = mats["polysilicon"]["overhang"][i]

	typ = mats[models["ntype"][i]]
	fet = mats[models["nfet"][i]]
	well = models["nfet_well"][i].split(":")
	
	overhang = typ["overhang"][0]

	if f"nfet_{variant}" in net["net"]:	
		ndev = net["net"][f"nfet_{variant}"]
		model = nmos(ndev, polyOverhang)
		modelNames.append(models["ntype"][i])
		for j, (name, ovr) in enumerate(reversed(list(zip(typ["gds"], typ["gds_bloat"])))):
			layer = layers[name]
			subst(model, layer, no, no, overhang if j == 0 else ovr, ovr)
			if j != 0:
				fill(layer)
				
		if well[0] != "":
			mat = mats[well[0]]
			for j, (name, ovr) in enumerate(reversed(list(zip(mat["gds"], mat["gds_bloat"])))):
				layer = layers[name]
				subst(model, layer, no, no, ovr, ovr)
				fill(layer)

	typ = mats[models["ptype"][i]]
	fet = mats[models["pfet"][i]]
	well = models["pfet_well"][i].split(":")
	
	overhang = typ["overhang"][0]

	if f"pfet_{variant}" in net["net"]:	
		ndev = net["net"][f"pfet_{variant}"]
		model = nmos(ndev, polyOverhang)
		modelNames.append(models["ntype"][i])
		for j, (name, ovr) in enumerate(reversed(list(zip(typ["gds"], typ["gds_bloat"])))):
			layer = layers[name]
			subst(model, layer, no, no, overhang if j == 0 else ovr, ovr)
			if j != 0:
				fill(layer)
				
		if well[0] != "":
			mat = mats[well[0]]
			for j, (name, ovr) in enumerate(reversed(list(zip(mat["gds"], mat["gds_bloat"])))):
				layer = layers[name]
				subst(model, layer, no, no, ovr, ovr)
				fill(layer)

mets = []
for name, mat in mats.items():
	if "gds" in mat:
		layer = layers[mat["gds"][-1]]
		if name == "polysilicon":
			mets.append(route(layer, -1, -1))

		if name != "metal":
			if "width" in mat:
				width(layer, mat["width"])
			if "spacing" in mat:
				spacing(layer, layer, mat["spacing"][-1])

met = mats["metal"]
for i in range(1, layout["general"]["metals"]):
	name = met[f"m{i}"]
	layer = layers[met[f"m{i}_gds"][-1]];
	mets.append(route(layer, no, no))
	width(layer, met[name]["width"][-1])
	spacing(layer, layer, met[name]["spacing"][-1])

vias = layout["vias"]
for i, lo in enumerate(modelNames):
	if lo in vias:
		name = vias[lo]
		viac = vias[name]
		layer = layers[vias[f"{lo}_gds"][-1]]

		dnlo = viac["surround"]["dn"]
		dnhi = viac["surround"]["asym_dn"]
		uplo = viac["surround"]["up"]
		uphi = viac["surround"]["asym_up"]

		via(layer, no, no, -i-1, 1, dnlo, dnhi, uplo, uphi)
		width(layer, viac["width"])
		spacing(layer, layer, viac["spacing"])

if "polysilicon" in vias:
	name = vias["polysilicon"]
	viac = vias[name]
	layer = layers[vias[f"polysilicon_gds"][-1]]

	dnlo = viac["surround"]["dn"]
	dnhi = viac["surround"]["asym_dn"]
	uplo = viac["surround"]["up"]
	uphi = viac["surround"]["asym_up"]

	via(layer, no, no, 0, 1, dnlo, dnhi, uplo, uphi)
	width(layer, viac["width"])
	spacing(layer, layer, viac["spacing"])

for i in range(1, layout["general"]["metals"]-1):
	lo = f"m{i}"
	if lo in vias:
		name = vias[lo]
		viac = vias[name]
		layer = layers[vias[f"{lo}_gds"][-1]]

		dnlo = viac["surround"]["dn"]
		dnhi = viac["surround"]["asym_dn"]
		uplo = viac["surround"]["up"]
		uphi = viac["surround"]["asym_up"]

		via(layer, no, no, i, i+1, dnlo, dnhi, uplo, uphi)
		width(layer, viac["width"])
		spacing(layer, layer, viac["spacing"])

