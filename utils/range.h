/*
    Copyright 2019 Faculty of Electrical Engineering at CTU in Prague

    This file is part of Game Theoretic Library.

    Game Theoretic Library is free software: you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public License
    as published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    Game Theoretic Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public 
    License along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef GTLIB2_RANGE_H
#define GTLIB2_RANGE_H


#include "base/base.h"
#include <cmath>

namespace GTLib2::utils {

inline vector<double> logRange(double start, double end, unsigned int numSteps, double base) {
    assert(start > 0);
    assert(end > 0);
    assert(end > start);
    assert(base > 0);
    assert(numSteps > 0);

    const double logStart = log(start) / log(base);
    const double logEnd = log(end) / log(base);
    vector<double> steps;
    for (double i = 0; i < numSteps; ++i) {
        double ratio = (i / (numSteps - 1));
        double y = logEnd * ratio + logStart * (1 - ratio);
        steps.push_back(pow(base, y));
    }
    return steps;
}

inline vector<double> log10Range(double start, double end, unsigned int numSteps) {
    return logRange(start, end, numSteps, 10.);
}

}

#endif //GTLIB2_RANGE_H
