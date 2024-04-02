# This is a test framework to execute the tech file separate from floret

Models = 0
Layers = 0
Routes = 0
Vias = 0
Rules = 0

def dbunit(value):
	print(f"dbunit({value})")

def paint(name, major, minor):
	global Layers
	print(f"paint(\"{name}\", {major}, {minor})")
	Layers += 1
	return Layers-1

def width(layer, value):
	print(f"width({layer}, {value})")

def fill(layer):
	print(f"fill({layer})")

def nmos(name, polyOverhang):
	global Models
	print(f"nmos(\"{name}\", {polyOverhang})")
	Models += 1
	return -Models

def pmos(name, polyOverhang):
	global Models
	print(f"pmos(\"{name}\", {polyOverhang})")
	Models += 1
	return -Models

def subst(model, draw, label, pin, overhangX, overhangY):
	print(f"subst({model}, {draw}, {label}, {pin}, {overhangX}, {overhangY})")

def via(draw, label, pin, downLevel, upLevel, downLo, downHi, upLo, upHi):
	global Vias
	print(f"via({draw}, {label}, {pin}, {downLevel}, {upLevel}, {downLo}, {downHi}, {upLo}, {upHi})")
	Vias += 1
	return Vias-1

def route(draw, label, pin):
	global Routes
	print(f"route({draw}, {label}, {pin})")
	Routes += 1
	return Routes-1

def spacing(left, right, value):
	global Rules
	print(f"spacing({left}, {right}, {value})")
	Rules += 1
	return -Rules

def b_and(left, right):
	global Rules
	print(f"b_and({left}, {right})")
	Rules += 1
	return -Rules

def b_or(left, right):
	global Rules
	print(f"b_or({left}, {right})")
	Rules += 1
	return -Rules

def b_not(layer):
	global Rules
	print(f"b_not({layer})")
	Rules += 1
	return -Rules

def bound(layer):
	print(f"bound({layer})")

