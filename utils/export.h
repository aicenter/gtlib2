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


#ifndef GTLIB2_EXPORT_H
#define GTLIB2_EXPORT_H

#include "base/base.h"
#include "base/efg.h"
#include "base/cache.h"
#include <fstream>

namespace GTLib2::utils {

void exportGraphViz(const Domain &domain, const string &fileToSave);
void exportGraphViz(const Domain &domain, std::ostream &fs);
void exportGraphViz(const PublicStateCache &domain, const string &fileToSave);
void exportGraphViz(const PublicStateCache &domain, std::ostream &fs);
void exportGambit(const Domain &domain, const string &fileToSave);
void exportGambit(const Domain &domain, std::ostream &fs);

void exportGambit(const shared_ptr<EFGNode>& rootNode, std::ostream &fs);
void exportGambit(const shared_ptr<EFGNode> &rootNode, const string &fileToSave);

}

#endif //GTLIB2_EXPORT_H
