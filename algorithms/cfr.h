//
// Created by Jakub Rozlivek on 8/8/18.
//

#ifndef GTLIB2_CFR_H
#define GTLIB2_CFR_H

#include "../base/base.h"

namespace GTLib2 {
  namespace algorithms {

    BehavioralStrategy getStrategyFor(const Domain &domain, int player,
            const unordered_map<shared_ptr<InformationSet>,
                    pair<vector<double>, vector<double>>>& allMP);

    /**
     * The main function for CFR iteration.
     * Implementation based on Algorithm 1 in M. Lanctot PhD thesis.
     */
    pair<double,double> CFRiterations(const Domain &domain, int iterations);

    /**
     * The main function for CFR iteration.
     * Implementation based on Algorithm 1 in M. Lanctot PhD thesis.
     * AOhistory created in iterations, not in nodes. It should be a first choice.
     */
    pair<double,double> CFRiterationsAOH(const Domain &domain, int iterations);
  }
}


#endif //GTLIB2_CFR_H
