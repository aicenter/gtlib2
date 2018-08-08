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

    pair<double,double> CFR(const Domain &domain, int iterations);
  }
}


#endif //GTLIB2_CFR_H
