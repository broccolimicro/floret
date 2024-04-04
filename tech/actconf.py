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

def key(m, name, default=None):
	if name != "" and name in m:
		return m[name]
	return default

def getLayers(layers, mat):
	result = []
	if mat:
		for name, bloat in zip(key(mat, "gds"), key(mat, "gds_bloat")):
			if name in layers:
				result = [(layers[name], bloat)] + result
			else:
				print(f"error: layer not found \"{name}\"")
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


mtrls = layout["materials"]

dbunit(layout["general"]["scale"]*1e-3)

no = -1
polyOverhang = mtrls["polysilicon"]["overhang"][-1]




# Handle GDS Paint layers
layers = {}
for name, major, minor in zip(layout["gds"]["layers"], layout["gds"]["major"], layout["gds"]["minor"]):
	layers[name] = paint(name, major, minor)




# Handle Routing
metalLevels = []
metalLayers = []
routeConf = mtrls["metal"]
## Poly
if "polysilicon" in mtrls:
	mat = mtrls["polysilicon"]
	if "gds" in mat:
		layer = layers[mat["gds"][-1]]
		metalLayers.append(layer)
		metalLevels.append(route(layer, no, no))

		if "width" in mat:
			width(layer, mat["width"])
		if "spacing" in mat:
			spacing(layer, layer, mat["spacing"][-1])

## Metals
for i in range(1, layout["general"]["metals"]+1):
	name = routeConf[f"m{i}"]
	mat = routeConf[name]
	if f"m{i}_gds" in routeConf:
		layer = layers[routeConf[f"m{i}_gds"][-1]];
		metalLayers.append(layer)
		metalLevels.append(route(layer, no, no))

		if "width" in mat:
			width(layer, mat["width"][-1])
		if "spacing" in mat:
			spacing(layer, layer, mat["spacing"][-1])



# Handle Transistor Models
models = layout["diff"]
modelNames = set()
for i, variant in enumerate(models["types"]):
	ndiffName = models["ntype"][i]
	pdiffName = models["ptype"][i]
	if f"nfet_{variant}" in net["net"]:	
		modelNames.add(ndiffName)
	if f"pfet_{variant}" in net["net"]:	
		modelNames.add(pdiffName)



# Handle Vias

## Diffusion Vias
viaConf = layout["vias"]
diffVias = dict()
for i, lo in enumerate(modelNames):
	if lo in viaConf:
		name = viaConf[lo]
		viac = viaConf[name]
		layer = layers[viaConf[f"{lo}_gds"][-1]]

		dnlo = viac["surround"]["dn"]
		dnhi = viac["surround"]["asym_dn"]
		uplo = viac["surround"]["up"]
		uphi = viac["surround"]["asym_up"]

		diffVias[lo] = layer
		via(layer, no, no, -i-1, metalLevels[1], dnlo, dnhi, uplo, uphi)
		width(layer, viac["width"])
		spacing(layer, layer, viac["spacing"])

## Poly Vias
if "polysilicon" in viaConf:
	name = viaConf["polysilicon"]
	viac = viaConf[name]
	layer = layers[viaConf[f"polysilicon_gds"][-1]]

	dnlo = viac["surround"]["dn"]
	dnhi = viac["surround"]["asym_dn"]
	uplo = viac["surround"]["up"]
	uphi = viac["surround"]["asym_up"]

	via(layer, no, no, metalLevels[0], metalLevels[1], dnlo, dnhi, uplo, uphi)
	width(layer, viac["width"])
	spacing(layer, layer, viac["spacing"])

## Metal Vias
for i in range(1, layout["general"]["metals"]+1):
	lo = f"m{i}"
	if lo in viaConf:
		name = viaConf[lo]
		viac = viaConf[name]
		layer = layers[viaConf[f"{lo}_gds"][-1]]

		dnlo = viac["surround"]["dn"]
		dnhi = viac["surround"]["asym_dn"]
		uplo = viac["surround"]["up"]
		uphi = viac["surround"]["asym_up"]

		via(layer, no, no, metalLevels[i], metalLevels[i+1], dnlo, dnhi, uplo, uphi)
		width(layer, viac["width"])
		spacing(layer, layer, viac["spacing"])






models = layout["diff"]
for i, variant in enumerate(models["types"]):
	ndiffName = models["ntype"][i]
	nfetName = models["nfet"][i]
	pwellName, ptapName = models["nfet_well"][i].split(":")

	pdiffName = models["ptype"][i]
	pfetName = models["pfet"][i]
	nwellName, ntapName = models["pfet_well"][i].split(":")

	poly = key(mtrls, "polysilicon")

	ndiff = key(mtrls, ndiffName)
	nfet  = key(mtrls, nfetName)
	pwell = key(mtrls, pwellName)
	ptap  = key(mtrls, ptapName)

	pdiff = key(mtrls, pdiffName)
	pfet  = key(mtrls, pfetName)
	nwell = key(mtrls, nwellName)
	ntap  = key(mtrls, ntapName)

	polyMats  = getLayers(layers, poly)
	ndiffMats = getLayers(layers, ndiff)
	pwellMats = getLayers(layers, pwell)
	pdiffMats = getLayers(layers, pdiff)
	nwellMats = getLayers(layers, nwell)
	
	

	if "width" in ndiff:
		width(ndiffMats[0][0], ndiff["width"])
	if "spacing" in ndiff:
		spacing(ndiffMats[0][0], ndiffMats[0][0], ndiff["spacing"][-1])
	#if "minarea" in ndiff:
	#	minarea(ndiffMats[0][0], ndiff["minarea"])
	if "oppspacing" in ndiff:
		spacing(ndiffMats[0][0], pdiffMats[0][0], pdiff["oppspacing"][-1])
	if "polyspacing" in ndiff:
		spacing(ndiffMats[0][0], b_and(b_not(ndiffMats[0][0]), polyMats[0][0]), ndiff["polyspacing"])
	if "via" in ndiff:
		if "fet" in ndiff["via"]:
			spacing(polyMats[0][0], diffVias[ndiffName], ndiff["via"]["fet"])
	
	if f"nfet_{variant}" in net["net"]:	
		model = nmos(net["net"][f"nfet_{variant}"], polyOverhang)
		for j, (layer, ovr) in enumerate(ndiffMats):
			ovrx = ovr
			if j == 0 and "overhang" in ndiff:
				ovrx = ndiff["overhang"][0]
			if j != 0:
				fill(layer)
			subst(model, layer, no, no, ovrx, ovr)
				
		if pwell:
			for j, (layer, ovr) in enumerate(pwellMats):
				subst(model, layer, no, no, ovr, ovr)
				fill(layer)
		
	if "width" in pdiff:
		width(pdiffMats[0][0], pdiff["width"])
	if "spacing" in pdiff:
		spacing(pdiffMats[0][0], pdiffMats[0][0], pdiff["spacing"][-1])
	#if "minarea" in pdiff:
	#	minarea(pdiffMats[0][0], pdiff["minarea"])
	if "oppspacing" in pdiff:
		spacing(pdiffMats[0][0], ndiffMats[0][0], ndiff["oppspacing"][-1])
	if "polyspacing" in pdiff:
		spacing(pdiffMats[0][0], b_and(b_not(pdiffMats[0][0]), polyMats[0][0]), pdiff["polyspacing"])
	if "via" in pdiff:
		if "fet" in pdiff["via"]:
			spacing(polyMats[0][0], diffVias[pdiffName], pdiff["via"]["fet"])
	


	if f"pfet_{variant}" in net["net"]:	
		model = pmos(net["net"][f"pfet_{variant}"], polyOverhang)
		for j, (layer, ovr) in enumerate(pdiffMats):
			ovrx = ovr
			if j == 0 and "overhang" in pdiff:
				ovrx = pdiff["overhang"][0]
			if j != 0:
				fill(layer)
			subst(model, layer, no, no, ovrx, ovr)
				
		if nwell:
			for j, (layer, ovr) in enumerate(nwellMats):
				subst(model, layer, no, no, ovr, ovr)
				fill(layer)





