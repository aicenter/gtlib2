//
// Created by Pavel Rytir on 1/21/18.
//

#ifndef ALGORITHMS_BESTRESPONSE_H_
#define ALGORITHMS_BESTRESPONSE_H_

#include <utility>
#include "base/base.h"


namespace GTLib2 {
namespace algorithms {

pair<BehavioralStrategy, double> bestResponseTo(const BehavioralStrategy &opoStrat, int opponent,
                                                int player, const Domain &domain);

pair<BehavioralStrategy, double> bestResponseTo(const BehavioralStrategy &opoStrat, int opponent,
                                                int player, const Domain &domain,
                                                int maxDepth);

pair<BehavioralStrategy, double> bestResponseToPrunning(const BehavioralStrategy &opoStrat,
                                                        int opponent,
                                                        int player,
                                                        const Domain &domain);

pair<BehavioralStrategy, double> bestResponseToPrunning(const BehavioralStrategy &opoStrat,
                                                        int opponent,
                                                        int player,
                                                        const Domain &domain,
                                                        int maxDepth);

}  // namespace algorithms
}  // namespace GTLib2

#endif  // ALGORITHMS_BESTRESPONSE_H_
