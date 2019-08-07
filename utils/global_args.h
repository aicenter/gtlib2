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

#ifndef GTLIB2_GLOBAL_ARGSH
#define GTLIB2_GLOBAL_ARGSH

#include "utils/args.hpp"
#include "utils/logging.h"

// Specify all global CLI arguments here
namespace args {

args::Group arguments("global arguments");

args::ValueFlag<std::string> domain(arguments,
                                    "DOMAIN",
                                    "Domain name to use, see constructDomain function for details.",
                                    {'d', "domain"}, "IIGS_3");
args::ValueFlagList<std::string> alg(arguments,
                                     "ALG_NAME",
                                     "Algorithms to run",
                                     {'a', "alg"}, {"CFR", "CFR"});
args::ValueFlagList<std::string> algcfg(arguments,
                                        "CFG_PATH",
                                        "Config files to use",
                                        {'c', "algcfg"},
                                        {"settings/cfr.json", "settings/cfr.json"});
args::ValueFlag<unsigned int> log_level(arguments, "",
                                        "Logging level", {'l', "log_level"}, GTLib2::CLI::LOGLEVEL_INFO);

}

void initializeParser(args::Subparser &parser) {
    parser.Parse();
    GTLib2::CLI::log_level = args::get(args::log_level);
}

#endif // GTLIB2_GLOBAL_ARGSH
