from common import *

# gadget_its = [int(pow(10, i)) for i in np.arange(2, 6.5, .5)]
gadget_its = [int(pow(10, i)) for i in np.arange(2, 6.5, .5)][::-1]
# root_its =   [int(pow(10, i)) for i in np.arange(2., 7.5, .5)]
root_its =   [int(pow(10, i)) for i in np.arange(2., 7.5, .5)][::-1]

num_seeds = 50
resetData = "false"

for domain in domains:
  for gadgetit in gadget_its:
    for rootit in root_its:
      print(
          f"iterationsInRoot={rootit} "
          f"iterationsPerGadgetGame={gadgetit} "
          f"numSeeds={num_seeds} "
          f"resetData={resetData} "
          f"player=1 "
          f"{cmd} \"x\" 0 MCCRavg "
          f"{domain} "
          f">> experiments_expl_avg_keep/{_(domain)}_$(hostname)_$$ "
          f"2> /dev/null""")

#
