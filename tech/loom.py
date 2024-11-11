# This is a test framework to execute the tech file separate from floret

Models = 0
Layers = 0
Routes = 0
Vias = 0
Rules = 0

def dbunit(value):
	print(f"dbunit({value})")

def scale(value):
	print(f"scale({value})")

def paint(name, major, minor):
	global Layers
	Layers += 1
	print(f"paint(\"{name}\", {major}, {minor}) # {Layers-1}")
	return Layers-1

def width(layer, value):
	print(f"width({layer}, {value})")

def fill(layer):
	print(f"fill({layer})")

def nmos(variant, name, stack, exclude=[], bins=[]):
	global Models
	Models += 1
	print(f"nmos(\"{variant}\", \"{name}\", {stack}, {exclude}, {bins}) # {-Models}")
	return -Models

def pmos(variant, name, stack, exclude=[], bins=[]):
	global Models
	Models += 1
	print(f"pmos(\"{variant}\", \"{name}\", {stack}, {exclude}, {bins}) # {-Models}")
	return -Models

def subst(draw, label, pin):
	print(f"subst({draw}, {label}, {pin})")

def well(draw, label, pin):
	print(f"well({draw}, {label}, {pin})")

def via(level, downLevel, upLevel):
	global Vias
	Vias += 1
	print(f"via({level}, {downLevel}, {upLevel}) # {Vias-1}")
	return Vias-1

def route(draw, label, pin):
	global Routes
	Routes += 1
	print(f"route({draw}, {label}, {pin}) # {Routes-1}")
	return Routes-1

def spacing(left, right, value):
	global Rules
	Rules += 1
	print(f"spacing({left}, {right}, {value}) # {-Rules}")
	return -Rules

def enclosing(left, right, lo, hi):
	global Rules
	Rules += 1
	print(f"spacing({left}, {right}, {lo}, {hi}) # {-Rules}")
	return -Rules

def b_and(left, right):
	global Rules
	Rules += 1
	print(f"b_and({left}, {right}) # {-Rules}")
	return -Rules

def b_or(left, right):
	global Rules
	Rules += 1
	print(f"b_or({left}, {right}) # {-Rules}")
	return -Rules

def b_not(layer):
	global Rules
	Rules += 1
	print(f"b_not({layer}) # {-Rules}")
	return -Rules

def bound(layer):
	print(f"bound({layer})")

