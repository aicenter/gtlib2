from common import *

num_matches = 5
preplay = 100
move = 100

##

seed = 0
print(f"echo -n 'id,'; ./main play_match --tag_header -l 100;")
for alg1, settings1 in algs.items():
  for alg2, settings2 in algs.items():
    if alg1 == alg2:
      continue
    for domain in domains:
      for match in range(num_matches):
        seed1 = np.random.randint(1, 1e8)
        seed2 = np.random.randint(1, 1e8)

        print(f"echo -n '{seed},'; "
              f"./main play_match -l 100 -t "
              f"-d {domain} "
              f"-a '{alg1}' -c {settings1} --x1 seed={seed1} "
              f"-a '{alg2}' -c {settings2} --x2 seed={seed2} "
              f"--preplay={preplay} --move={move} --seed={seed} --time;")
        seed += 1
