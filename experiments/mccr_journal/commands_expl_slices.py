from common import *

times = [10, 31, 100, 316, 1000]
times.reverse()

print(f"./main calc_expl --alg=MCCR --tag_header ")
for p in times:
  for m in times:
    for v in {"reset", "naivekeep", "reweighkeep"}:
      print(f"./main calc_expl -l 0 --alg=MCCR --algcfg=cfg/MCCR_{v}.ini --preplay={p} --move={m} --iterations -d IIGS_3 -t --dbar")
