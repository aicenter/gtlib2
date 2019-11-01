import numpy as np

##############################################################################
# Settings
##############################################################################

domains = [
  "IIGS_small", "IIGS_large",
  "GP_small", "GP_large",
  "LD_small", "LD_large",
  # "KS_MICROCHESS",
  # "STRAT6x6",
  # "PTTT"
]

small_domains = [
  "IIGS_small",
  "GP_small",
  "LD_small",
]

big_domains = [
  "IIGS_large",
  "GP_large",
  "LD_large",
  "KS_MICROCHESS",
  "STRAT6x6",
  "PTTT",
]

algs = {
  # "MCCFR":       "cfg/MCCFR.ini",
  "MCCR keep":   "cfg/MCCR_naivekeep.ini",
  "MCCR reset":  "cfg/MCCR_reset.ini",
  # "OOS":         "cfg/OOS.ini",
  # "RND":         "cfg/RND.ini",
  # "UCT":         "cfg/UCT.ini",
  # "RM":          "cfg/RM.ini",
}

game_values = {  # for player 0
  "B-RPS": 0.3235,  # exact value
  "GP-3322": -0.11262621627252051,  # 5000 iters of CFR
  "GP": -0.11262621627252051,  # 5000 iters of CFR
  "GP-4644": 0,  # how knows
  "LD-116": -0.027693493122754417,  # 5000 iters of CFR
  "LD": -0.027693493122754417,  # 5000 iters of CFR
  "LD-226": 0,
  "IIGS-4": 0,
  "IIGS-4-ignore": 0,
  "IIGS-5": 0,  # exact value
  "IIGS": 0,  # exact value
  "IIGS-13": 0,
  "PTTT": 0,
}

##############################################################################
# Plotting
##############################################################################

import matplotlib as mpl

mpl.use('pdf')
import matplotlib.pyplot as plt
import matplotlib.colors as mcolors

plt.rc('font', family='Linux Libertine', size=8)
plt.rc('text', usetex=False)
plt.rc('xtick', labelsize=6)
plt.rc('ytick', labelsize=6)
plt.rc('axes', labelsize=8)
plt.rc('legend', fontsize=6)
plt.rc('legend', handlelength=1)
# 'legend.handlelength': 2

tableau20 = [
  (31, 119, 180), (174, 199, 232), (255, 127, 14), (255, 187, 120),
  (44, 160, 44), (152, 223, 138), (214, 39, 40), (255, 152, 150),
  (148, 103, 189), (197, 176, 213), (140, 86, 75), (196, 156, 148),
  (227, 119, 194), (247, 182, 210), (127, 127, 127), (199, 199, 199),
  (188, 189, 34), (219, 219, 141), (23, 190, 207), (158, 218, 229)]
colors = []
for (r, g, b) in tableau20:
  colors.append((r / 255., g / 255., b / 255.))
cmap = "cool"


def nicelim(min, max):
  return pow(10, np.floor(np.log10(min))), pow(10, np.ceil(np.log10(max)))
