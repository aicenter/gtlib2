dbar_iigs5_reweigh_2019-11-03.csv - reweighing just multiplies regrets and baseline denominators by action prob. It resets the avg strategy.
dbar_iigs5_reweigh_2019-11-04.csv - reweigh does not reset avg strategy
    I did not let this one finish.
    
I've discovered a few problems:
- I used NoBaseline for MCCR! so all CFV constraints were zero! I added a check for that.
- The weighting uses some strange update s_h_all for baselines. I need to check this.

2019-11-06.85ef1d1b.dbar_iigs3 - updated baselines to actually have some non-zero value :) (I forgot to use reference, so only local copy was updated). Does not weight using strange s_h_all


2019-11-06.85ef1d1.dbar_iigs4
2019-11-06.5a65b36.dbar_iigs4_strange
It seems like the strange weighing is indeed not good, it was some artifact from previos development.

