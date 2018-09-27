//
// Created by Pavel Rytir on 22/01/2018.
//

#ifndef ALGORITHMS_EQUILIBRIUM_H_
#define ALGORITHMS_EQUILIBRIUM_H_

#include <tuple>
#include "base/base.h"

namespace GTLib2 {
namespace algorithms {

std::tuple<double, BehavioralStrategy> findEquilibriumTwoPlayersZeroSum(const Domain &domain);

}  // namespace algorithms
}  // namespace GTLib2

#endif  // ALGORITHMS_EQUILIBRIUM_H_
