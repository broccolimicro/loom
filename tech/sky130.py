from floret import *

AL = 0  # do not change
CU = 1  # do not change

# technology configuration
########################

# choose betwen only one of AL or CU back-end flow here:
backend_flow = AL

# enable / disable rule groups
FEOL    = True # front-end-of-line checks
BEOL    = True # back-end-of-line checks
OFFGRID = True # manufacturing grid/angle checks

# Define Global Rules
dbunit(5e-3)
scale(1e6)

# Define Paint Layers
no = -1
diff = paint("diff.drawing", 65, 20)
tap = paint("tap.drawing", 65, 44)
nwell = paint("nwell.drawing", 64, 20)
dnwell = paint("dnwell.drawing", 64, 18)
pwbm = paint("pwbm.drawing", 19, 44)
pwde = paint("pwde.drawing", 124, 20)
hvtr = paint("hvtr.drawing", 18, 20)
hvtp = paint("hvtp.drawing", 78, 44)
ldntm = paint("ldntm.drawing", 11, 44)
hvi = paint("hvi.drawing", 75, 20)
tunm = paint("tunm.drawing", 80, 20)
lvtn = paint("lvtn.drawing", 125, 44)
poly = paint("poly.drawing", 66, 20)
hvntm = paint("hvntm.drawing", 125, 20)
nsdm = paint("nsdm.drawing", 93, 44)
psdm = paint("psdm.drawing", 94, 20)
rpm = paint("rpm.drawing", 86, 20)
urpm = paint("urpm.drawing", 79, 20)
npc = paint("npc.drawing", 95, 20)
licon1 = paint("licon1.drawing", 66, 44)
li1 = paint("li1.drawing", 67, 20)
mcon = paint("mcon.drawing", 67, 44)
met1 = paint("met1.drawing", 68, 20)
via1 = paint("via.drawing", 68, 44)
met2 = paint("met2.drawing", 69, 20)
via2 = paint("via2.drawing", 69, 44)
met3 = paint("met3.drawing", 70, 20)
via3 = paint("via3.drawing", 70, 44)
met4 = paint("met4.drawing", 71, 20)
via4 = paint("via4.drawing", 71, 44)
met5 = paint("met5.drawing", 72, 20)
pad = paint("pad.drawing", 76, 20)
nsm = paint("nsm.drawing", 61, 20)
capm = paint("capm.drawing", 89, 44)
cap2m = paint("cap2m.drawing", 97, 44)
vhvi = paint("vhvi.drawing", 74, 21)
uhvi = paint("uhvi.drawing", 74, 22)
npn = paint("npn.drawing", 82, 20)
inductor = paint("inductor.drawing", 82, 24)
capacitor = paint("capacitor.drawing", 82, 64)
pnp = paint("pnp.drawing", 82, 44)
lvsPrune = paint("lvsPrune.drawing", 84, 44)
ncm = paint("ncm.drawing", 92, 44)
padcenter = paint("padcenter.drawing", 81, 20)
target = paint("target.drawing", 76, 44)
areaid_sl = paint("areaid.sl.identifier", 81, 1)
areaid_ce = paint("areaid.ce.identifier", 81, 2)
areaid_fe = paint("areaid.fe.identifier", 81, 3)
areaid_sc = paint("areaid.sc.identifier", 81, 4)
areaid_sf = paint("areaid.sf.identifier", 81, 6)
areaid_sl1 = paint("areaid.sl.identifier1", 81, 7)
areaid_sr = paint("areaid.sr.identifier", 81, 8)
areaid_mt = paint("areaid.mt.identifier", 81, 10)
areaid_dt = paint("areaid.dt.identifier", 81, 11)
areaid_ft = paint("areaid.ft.identifier", 81, 12)
areaid_ww = paint("areaid.ww.identifier", 81, 13)
areaid_ld = paint("areaid.ld.identifier", 81, 14)
areaid_ns = paint("areaid.ns.identifier", 81, 15)
areaid_ij = paint("areaid.ij.identifier", 81, 17)
areaid_zr = paint("areaid.zr.identifier", 81, 18)
areaid_ed = paint("areaid.ed.identifier", 81, 19)
areaid_de = paint("areaid.de.identifier", 81, 23)
areaid_rd = paint("areaid.rd.identifier", 81, 24)
areaid_dn = paint("areaid.dn.identifier", 81, 50)
areaid_cr = paint("areaid.cr.identifier", 81, 51)
areaid_cd = paint("areaid.cd.identifier", 81, 52)
areaid_st = paint("areaid.st.identifier", 81, 53)
areaid_op = paint("areaid.op.identifier", 81, 54)
areaid_en = paint("areaid.en.identifier", 81, 57)
areaid_en20 = paint("areaid.en20.identifier", 81, 58)
areaid_le = paint("areaid.le.identifier", 81, 60)
areaid_hl = paint("areaid.hl.identifier", 81, 63)
areaid_sd = paint("areaid.sd.identifier", 81, 70)
areaid_po = paint("areaid.po.identifier", 81, 81)
areaid_it = paint("areaid.it.identifier", 81, 84)
areaid_et = paint("areaid.et.identifier", 81, 101)
areaid_lvt = paint("areaid.lvt.identifier", 81, 108)
areaid_re = paint("areaid.re.identifier", 81, 125)
fom_dummy = paint("fom.dummy", 22, 23)
poly_gate = paint("poly.gate", 66, 9)
poly_model = paint("poly.model", 66, 83)
poly_resistor = paint("poly.resistor", 66, 13)
diff_resistor = paint("diff.resistor", 65, 13)
pwell_resistor = paint("pwell.resistor", 64, 13)
li1_resistor = paint("li1.resistor", 67, 13)
diff_highVoltage = paint("diff.highVoltage", 65, 8)
met4_fuse = paint("met4.fuse", 71, 17)
inductor_terminal1 = paint("inductor.terminal1", 82, 26)
inductor_terminal2 = paint("inductor.terminal2", 82, 27)
inductor_terminal3 = paint("inductor.terminal3", 82, 28)
li1_block = paint("li1.block", 67, 10)
met1_block = paint("met1.block", 68, 10)
met2_block = paint("met2.block", 69, 10)
met3_block = paint("met3.block", 70, 10)
met4_block = paint("met4.block", 71, 10)
met5_block = paint("met5.block", 72, 10)
prbndry_boundary = paint("prbndry.boundary", 235, 4)
diff_boundary = paint("diff.boundary", 65, 4)
tap_boundary = paint("tap.boundary", 65, 60)
mcon_boundary = paint("mcon.boundary", 67, 60)
poly_boundary = paint("poly.boundary", 66, 4)
via1_boundary = paint("via.boundary", 68, 60)
via2_boundary = paint("via2.boundary", 69, 60)
via3_boundary = paint("via3.boundary", 70, 60)
via4_boundary = paint("via4.boundary", 71, 60)
li1_label = paint("li1.label", 67, 5)
met1_label = paint("met1.label", 68, 5)
met2_label = paint("met2.label", 69, 5)
met3_label = paint("met3.label", 70, 5)
met4_label = paint("met4.label", 71, 5)
met5_label = paint("met5.label", 72, 5)
poly_label = paint("poly.label", 66, 5)
diff_label = paint("diff.label", 65, 6)
pwell_label = paint("pwell.label", 64, 59)
pwelliso_label = paint("pwelliso.label", 44, 5)
pad_label = paint("pad.label", 76, 5)
tap_label = paint("tap.label", 65, 5)
nwell_label = paint("nwell.label", 64, 5)
inductor_label = paint("inductor.label", 82, 25)
text_label = paint("text.label", 83, 44)
li1_net = paint("li1.net", 67, 23)
met1_net = paint("met1.net", 68, 23)
met2_net = paint("met2.net", 69, 23)
met3_net = paint("met3.net", 70, 23)
met4_net = paint("met4.net", 71, 23)
met5_net = paint("met5.net", 72, 23)
poly_net = paint("poly.net", 66, 23)
diff_net = paint("diff.net", 65, 23)
li1_pin = paint("li1.pin", 67, 16)
met1_pin = paint("met1.pin", 68, 16)
met2_pin = paint("met2.pin", 69, 16)
met3_pin = paint("met3.pin", 70, 16)
met4_pin = paint("met4.pin", 71, 16)
met5_pin = paint("met5.pin", 72, 16)
poly_pin = paint("poly.pin", 66, 16)
diff_pin = paint("diff.pin", 65, 16)
nwell_pin = paint("nwell.pin", 64, 16)
pad_pin = paint("pad.pin", 76, 16)
pwell_pin = paint("pwell.pin", 122, 16)
pwelliso_pin = paint("pwelliso.pin", 44, 16)
nwell_pin1 = paint("nwell.pin1", 64, 0)
poly_pin1 = paint("poly.pin1", 66, 0)
li1_pin1 = paint("li1.pin1", 67, 0)
met1_pin1 = paint("met1.pin1", 68, 0)
met2_pin1 = paint("met2.pin1", 69, 0)
met3_pin1 = paint("met3.pin1", 70, 0)
met4_pin1 = paint("met4.pin1", 71, 0)
met5_pin1 = paint("met5.pin1", 72, 0)
pad_pin1 = paint("pad.pin1", 76, 0)
pwell_pin1 = paint("pwell.pin1", 122, 0)
diff_cut = paint("diff.cut", 65, 14)
poly_cut = paint("poly.cut", 66, 14)
li1_cut = paint("li1.cut", 67, 14)
met1_cut = paint("met1.cut", 68, 14)
met2_cut = paint("met2.cut", 69, 14)
met3_cut = paint("met3.cut", 70, 14)
met4_cut = paint("met4.cut", 71, 14)
met5_cut = paint("met5.cut", 72, 14)
met5_probe = paint("met5.probe", 72, 25)
met4_probe = paint("met4.probe", 71, 25)
met3_probe = paint("met3.probe", 70, 25)
met2_probe = paint("met2.probe", 69, 25)
met1_probe = paint("met1.probe", 68, 25)
li1_probe = paint("li1.probe", 67, 25)
poly_probe = paint("poly.probe", 66, 25)
poly_short = paint("poly.short", 66, 15)
li1_short = paint("li1.short", 67, 15)
met1_short = paint("met1.short", 68, 15)
met2_short = paint("met2.short", 69, 15)
met3_short = paint("met3.short", 70, 15)
met4_short = paint("met4.short", 71, 15)
met5_short = paint("met5.short", 72, 15)
cncm_mask = paint("cncm.mask", 17, 0)
crpm_mask = paint("crpm.mask", 96, 0)
cpdm_mask = paint("cpdm.mask", 37, 0)
cnsm_mask = paint("cnsm.mask", 22, 0)
cmm5_mask = paint("cmm5.mask", 59, 0)
cviam4_mask = paint("cviam4.mask", 58, 0)
cmm4_mask = paint("cmm4.mask", 51, 0)
cviam3_mask = paint("cviam3.mask", 50, 0)
cmm3_mask = paint("cmm3.mask", 34, 0)
cviam2_mask = paint("cviam2.mask", 44, 0)
cmm2_mask = paint("cmm2.mask", 41, 0)
cviam1_mask = paint("cviam.mask", 40, 0)
cmm1_mask = paint("cmm1.mask", 36, 0)
ctm1_mask = paint("ctm1.mask", 35, 0)
cli1m_mask = paint("cli1m.mask", 56, 0)
clicm1_mask = paint("clicm1.mask", 43, 0)
cpsdm_mask = paint("cpsdm.mask", 32, 0)
cnsdm_mask = paint("cnsdm.mask", 30, 0)
cldntm_mask = paint("cldntm.mask", 11, 0)
cnpc_mask = paint("cnpc.mask", 49, 0)
chvntm_mask = paint("chvntm.mask", 39, 0)
cntm_mask = paint("cntm.mask", 27, 0)
cp1m_mask = paint("cp1m.mask", 28, 0)
clvom_mask = paint("clvom.mask", 46, 0)
conom_mask = paint("conom.mask", 88, 0)
ctunm_mask = paint("ctunm.mask", 20, 0)
chvtrm_mask = paint("chvtrm.mask", 98, 0)
chvtpm_mask = paint("chvtpm.mask", 97, 0)
clvtnm_mask = paint("clvtnm.mask", 25, 0)
cnwm_mask = paint("cnwm.mask", 21, 0)
cdnm_mask = paint("cdnm.mask", 48, 0)
cfom_mask = paint("cfom.mask", 23, 0)
cfom = paint("cfom.drawing", 22, 20)
clvtnm = paint("clvtnm.drawing", 25, 44)
chvtpm = paint("chvtpm.drawing", 88, 44)
conom = paint("conom.drawing", 87, 44)
clvom = paint("clvom.drawing", 45, 20)
cntm = paint("cntm.drawing", 26, 20)
chvntm = paint("chvntm.drawing", 38, 20)
cnpc = paint("cnpc.drawing", 44, 20)
cnsdm = paint("cnsdm.drawing", 29, 20)
cpsdm = paint("cpsdm.drawing", 31, 20)
cli1m = paint("cli1m.drawing", 115, 44)
cviam3 = paint("cviam3.drawing", 112, 20)
cviam4 = paint("cviam4.drawing", 117, 20)
cncm = paint("cncm.drawing", 96, 44)
cntm_maskAdd = paint("cntm.maskAdd", 26, 21)
clvtnm_maskAdd = paint("clvtnm.maskAdd", 25, 43)
chvtpm_maskAdd = paint("chvtpm.maskAdd", 97, 43)
cli1m_maskAdd = paint("cli1m.maskAdd", 115, 43)
clicm1_maskAdd = paint("clicm1.maskAdd", 106, 43)
cpsdm_maskAdd = paint("cpsdm.maskAdd", 31, 21)
cnsdm_maskAdd = paint("cnsdm.maskAdd", 29, 21)
cp1m_maskAdd = paint("cp1m.maskAdd", 33, 43)
cfom_maskAdd = paint("cfom.maskAdd", 22, 21)
cntm_maskDrop = paint("cntm.maskDrop", 26, 22)
clvtnm_maskDrop = paint("clvtnm.maskDrop", 25, 42)
chvtpm_maskDrop = paint("chvtpm.maskDrop", 97, 42)
cli1m_maskDrop = paint("cli1m.maskDrop", 115, 42)
clicm1_maskDrop = paint("clicm1.maskDrop", 106, 42)
cpsdm_maskDrop = paint("cpsdm.maskDrop", 31, 22)
cnsdm_maskDrop = paint("cnsdm.maskDrop", 29, 22)
cp1m_maskDrop = paint("cp1m.maskDrop", 33, 42)
cfom_maskDrop = paint("cfom.maskDrop", 22, 22)
cmm4_waffleDrop = paint("cmm4.waffleDrop", 112, 4)
cmm3_waffleDrop = paint("cmm3.waffleDrop", 107, 24)
cmm2_waffleDrop = paint("cmm2.waffleDrop", 105, 52)
cmm1_waffleDrop = paint("cmm1.waffleDrop", 62, 24)
cp1m_waffleDrop = paint("cp1m.waffleDrop", 33, 24)
cfom_waffleDrop = paint("cfom.waffleDrop", 22, 24)
cmm5_waffleDrop = paint("cmm5.waffleDrop", 117, 4)
nvtn = paint("nvtn", 251, 0)

# Define DRC Rules
fill(nwell)
fill(nwell_pin)
fill(nwell_label)
fill(dnwell)
fill(hvtp)
fill(nsdm)
fill(psdm)
fill(pwell_pin)
fill(pwell_label)
width(diff, 30)
width(hvtp, 76)
width(poly, 30)
width(licon1, 34)
width(li1, 34)
width(mcon, 34)
width(met1, 28)
width(via1, 30)
width(met2, 28)
width(via2, 40)
width(met3, 60)
width(via3, 40)
width(met4, 60)
width(via4, 160)
width(met5, 320)

enclosing(diff, poly, no, 50)
enclosing(nsdm, diff, 25, 25)
enclosing(psdm, diff, 25, 25)
enclosing(hvtp, psdm, 11, 11)
enclosing(lvtn, psdm, 11, 11)
enclosing(lvtn, nsdm, 11, 11)
enclosing(nwell, psdm, 36, 36)
enclosing(nwell, hvtp, 36, 36)
enclosing(nwell, lvtn, 36, 36)
enclosing(poly, diff, no, 26)
enclosing(diff, licon1, 8, 12)

enclosing(poly, licon1, 10, 16)
enclosing(li1, licon1, 0, 16)
enclosing(li1, mcon, 0, 0)
enclosing(met1, mcon, 6, 12)
enclosing(met1, via1, 11, 11)
enclosing(met2, via1, 11, 17)
enclosing(met2, via2, 8, 17)
enclosing(met3, via2, 13, 13)
enclosing(met3, via3, 12, 18)
enclosing(met4, via3, 13, 13)
enclosing(met4, via4, 38, 38)
enclosing(met5, via4, 62, 62)

spacing(nwell, nwell, 254)
spacing(nsdm, nsdm, 76)
spacing(psdm, psdm, 76)
spacing(diff, diff, 54)
spacing(hvtp, hvtp, 76)
spacing(poly, poly, 42)
spacing(licon1, licon1, 34)
spacing(li1, li1, 34)
spacing(mcon, mcon, 38)
spacing(met1, met1, 28)
spacing(via1, via1, 34)
spacing(met2, met2, 28)
spacing(via2, via2, 40)
spacing(met3, met3, 60)
spacing(via3, via3, 40)
spacing(met4, met4, 60)
spacing(via4, via4, 160)
spacing(met5, met5, 320)
spacing(poly, licon1, 18)
spacing(b_and(b_and(diff, b_not(uhvi)), nsdm), nwell, 68)
spacing(b_and(licon1, poly), b_or(diff, tap), 38)
spacing(b_and(poly, b_not(diff)), diff, 15)

bound(areaid_sc)

# Define Routing and Device Models
df = subst(diff, diff_label, diff_pin)
ns = subst(nsdm, no, no)
ps = subst(psdm, no, no)
hvp = subst(hvtp, no, no)
hvn = subst(hvntm, no, no) # TODO(edward.bingham) DRC rules
lvn = subst(lvtn, no, no)  # TODO(edward.bingham) DRC rules
nvn = subst(nvtn, no, no)  # TODO(edward.bingham) DRC rules
hvx = subst(hvi, no, no)   # TODO(edward.bingham) DRC rules
nw = well(nwell, nwell_label, nwell_pin)
pw = well(no, pwell_label, pwell_pin)

p = route(poly, poly_label, poly_pin)
m0 = route(li1, li1_label, li1_pin)
m1 = route(met1, met1_label, met1_pin)
m2 = route(met2, met2_label, met2_pin)
m3 = route(met3, met3_label, met3_pin)
m4 = route(met4, met4_label, met4_pin)
m5 = route(met5, met5_label, met5_pin)

nfet = nmos("svt", "sky130_fd_pr__nfet_01v8", [df, ns, pw], exclude=[hvp, lvn, hvx], bins=[(0,500)])
nfet_lvt = nmos("lvt", "sky130_fd_pr__nfet_01v8_lvt", [df, ns, lvn, pw], exclude=[nvn], bins=[(0,500)])
nfet_pin = nmos("npin", "sky130_fd_pr__nfet_g5v0d10v5", [df, ns, hvn, pw, hvx])
nfet_nvt = nmos("nvt", "sky130_fd_pr__nfet_05v0_nvt", [df, ns, nvn, lvn, pw])

pfet = pmos("svt", "sky130_fd_pr__pfet_01v8", [df, ps, nw], exclude=[hvp, lvn, hvx], bins=[(0,500)])
pfet_hvt = pmos("hvt", "sky130_fd_pr__pfet_01v8_hvt", [df, ps, hvp, nw], bins=[(0,500)])
pfet_lvt = pmos("lvt", "sky130_fd_pr__pfet_01v8_lvt", [df, ps, lvn, nw], bins=[(0,500)])
pfet_pin = pmos("ppin", "sky130_fd_pr__pfet_g5v0d10v5", [df, ps, nw, hvx])

via(nfet, m0, licon1)
via(nfet_lvt, m0, licon1)
via(nfet_pin, m0, licon1)
via(nfet_nvt, m0, licon1)
via(pfet, m0, licon1)
via(pfet_hvt, m0, licon1)
via(pfet_lvt, m0, licon1)
via(pfet_pin, m0, licon1)
via(p, m0, licon1)
via(m0, m1, mcon)
via(m1, m2, via1)
via(m2, m3, via2)
via(m3, m4, via3)
via(m4, m5, via4)


