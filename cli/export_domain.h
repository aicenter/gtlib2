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

    You should have received a copy of the GNU Lesser General Public License
    along with Game Theoretic Library.

    If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef GTLIB2_EXPORT_DOMAIN_H
#define GTLIB2_EXPORT_DOMAIN_H

#include "cli_helpers.h"
#include "utils/export.h"

namespace GTLib2::CLI {

void Command_ExportDomain(args::Subparser &parser) {
    args::Group group(parser, "Export type:", args::Group::Validators::Xor);
    args::Flag gambit(group, "gambit", "", {"gbt", "gambit"});
    args::Flag graphviz(group, "graphviz", "", {"dot", "graphviz"});
    initializeParser(parser); // always include this line in command

    unique_ptr<Domain> d = constructDomain(args::get(args::domain));

    if (gambit) {
        utils::exportGambit(*d, cout);
        return;
    }
    if (graphviz) {
        utils::exportGraphViz(*d, cout);
        return;
    }
    throw std::runtime_error("Invalid export type!");
}

}

#endif // GTLIB2_EXPORT_DOMAIN_H
