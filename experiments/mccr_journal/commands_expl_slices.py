from common import *

times = [10, 31, 100, 316, 1000, 3162, 10000]

for p in times:
  for m in times:
    for v in {"reset", "naivekeep", "reweighkeep"}:
      print(f"./main calc_expl -l 0 --alg=MCCR --algcfg=cfg/MCCR_{v}.ini --preplay={p} --move={m} --iterations -d IIGS_5 -t")
