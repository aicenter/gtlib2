//
// Created by Pavel Rytir on 1/21/18.
//

#ifndef PURSUIT_BESTRESPONSE_H
#define PURSUIT_BESTRESPONSE_H


#include "../base/base.h"


namespace GTLib2 {
    namespace algorithms {

        pair<BehavioralStrategy, double> bestResponseTo(const BehavioralStrategy &opoStrat, int opponent,
                                                        int player, const Domain &domain);

        pair<BehavioralStrategy, double> bestResponseTo(const BehavioralStrategy &opoStrat, int opponent,
                                                        int player, const Domain &domain,
                                                        int maxDepth);
    }
}

#endif //PURSUIT_BESTRESPONSE_H
