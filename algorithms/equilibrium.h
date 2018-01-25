//
// Created by Pavel Rytir on 22/01/2018.
//

#ifndef PURSUIT_EQUILIBRIUM_H
#define PURSUIT_EQUILIBRIUM_H


#include "../base/base.h"

namespace GTLib2 {
    namespace algorithms {


        tuple<double, BehavioralStrategy> findEquilibriumTwoPlayersZeroSum(const Domain &domain);

    }
}




#endif //PURSUIT_EQUILIBRIUM_H
