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

2019-11-06.8d500d1.dbar_iigs4
not reweighing denominator makes it worse somehow...

2019-11-06.8a18d31.dbar_iigs4
avg strat should not be updated, since it does not use leaves reach prob
now the perf gap is much smaller:

    MCCR_naivekeep.ini   ,1000,1000, ,0.028413
    MCCR_reset.ini       ,1000,1000, ,0.120949
    MCCR_reweighkeep.ini ,1000,1000, ,0.0307611


2019-11-06.7bacb63.dbar_iigs4.o
when avg strat is reset, the reweigh is worse than complete reset.
This is strange:

    MCCR_reweighkeep.ini  0.124618
    MCCR_reset.ini        0.120949

2019-11-06.0bb7ae0.dbar_iigs4.o
Just reweighing the histories is better! which works as expected.
There is smaller variance, and having high values on CFVs restricts 
the search for equilibrium too much.
So the problem is with reweighing regrets.

    
    IIGS_4,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0839454
    IIGS_4,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.120949
    
    IIOZ_5,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0364247
    IIOZ_5,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.0293356
    
    LD_tiny,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.590078
    LD_tiny,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.398923

I was talking with Vilo and maybe I forgot to multiply out 
the leaf coefficient

    2019-11-07.b50337c.dbar_*
    IIGS_4,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.112742
    IIGS_4,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.123992
    
    IIOZ_5,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0425759
    IIOZ_5,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.0285144
    
    LD_tiny,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.599886
    LD_tiny,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.374915

Hmm, now I would have expected to have expl smaller for all reweigh,
but still not the case!

---

Returning after some MCCR debugging and other university duties.

I've fixed a bug with OOS chance node baselines.

Test: reweigh: regrets and avg are reset.
      baseline values are multiplied by the prob. update
File: 2019-11-21.312450a.dbar.o
       
    IIGS_4,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0907702
    IIGS_4,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.0665932
    
    IIOZ_5,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0374502
    IIOZ_5,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.0194375
    
    LD_tiny,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.543075
    LD_tiny,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.490739

Seems resets are better now (except for LD_tiny).
So are reweighs. But still I was expecting the reweighs to be better than resets!

---

I largely simplified the reweighing, and baselines and node values
have been separated out. Slightly worse results than reset: 

    IIGS_4,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.12738
    IIGS_4,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.123992
    
    IIOZ_5,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0365062
    IIOZ_5,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.0285156
    
    LD_tiny,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.384823
    LD_tiny,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.374915
    
---

I forgot to use (1-lambda) update!

    IIGS_4,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.122971
    IIGS_4,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.123992
    
    IIOZ_5,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0365062
    IIOZ_5,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.0285156
    
    LD_tiny,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.39886
    LD_tiny,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.374915

---

Not reseting baselines:

    IIGS_4,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.13219
    IIGS_4,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.123992

    IIOZ_5,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0289625
    IIOZ_5,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.0285156

    LD_tiny,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.560497
    LD_tiny,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.374915

---

Same treatment of baselines and nodes (i.e. 1-lambda)

    IIGS_4,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0660759
    IIGS_4,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.123992
    
    IIOZ_5,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.136621
    IIOZ_5,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.0285156
    
    LD_tiny,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.445176
    LD_tiny,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.374915

Finally managed to make IIGS better :) but everything else is broken lol

---

    IIGS_4,MCCR,cfg/MCCR_naivekeep.ini,1000,1000,iterations,0.0289363
    IIGS_4,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0823888
    IIGS_4,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.151848

    IIOZ_5,MCCR,cfg/MCCR_naivekeep.ini,1000,1000,iterations,0.0652906
    IIOZ_5,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.0207208
    IIOZ_5,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.0350273

    LD_tiny,MCCR,cfg/MCCR_naivekeep.ini,1000,1000,iterations,0.302449
    LD_tiny,MCCR,cfg/MCCR_reweighkeep.ini,1000,1000,iterations,0.40525
    LD_tiny,MCCR,cfg/MCCR_reset.ini,1000,1000,iterations,0.494662
