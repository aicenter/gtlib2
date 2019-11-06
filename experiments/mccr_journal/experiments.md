dbar_iigs5_reweigh_2019-11-03.csv - reweighing just multiplies regrets and baseline denominators by action prob. It resets the avg strategy.
dbar_iigs5_reweigh_2019-11-04.csv - reweigh does not reset avg strategy
    I did not let this one finish.
    
I've discovered a few problems:
- I used NoBaseline for MCCR! so all CFV constraints were zero!
  I added a check for that.
- The weighting uses 

Add gadget export!
