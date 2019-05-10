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




#include "base/base.h"
#include "algorithms/common.h"
#include "tests/domainsTest.h"

#include "gtest/gtest.h"


namespace GTLib2::algorithms {


TEST(Common, CompatibilityOfAOids) {
    //@formatter:off
    typedef ActionObservationIds aoi;
    // both empty seq
    EXPECT_TRUE (isAOCompatible(  {},                        {}                        ));

    // one empty seq
    EXPECT_TRUE (isAOCompatible(  {},                        {NO_ACTION_OBSERVATION}   ));
    EXPECT_TRUE (isAOCompatible(  {NO_ACTION_OBSERVATION},   {}                        ));
    EXPECT_TRUE (isAOCompatible(  {},                        {}                        ));
    EXPECT_TRUE (isAOCompatible(  {},                        {aoi{1,1}}                ));
    EXPECT_TRUE (isAOCompatible(  {aoi{1,1}},                {}                        ));
    EXPECT_TRUE (isAOCompatible(  {},                        {aoi{1,NO_OBSERVATION}}   ));
    EXPECT_TRUE (isAOCompatible(  {aoi{1,NO_OBSERVATION}},   {}                        ));
    EXPECT_TRUE (isAOCompatible(  {},                        {aoi{NO_ACTION,1}}        ));
    EXPECT_TRUE (isAOCompatible(  {aoi{NO_ACTION,1}},        {}                        ));
    // one empty, one longer
    EXPECT_TRUE (isAOCompatible(  {NO_ACTION_OBSERVATION,  NO_ACTION_OBSERVATION},  {} ));
    EXPECT_TRUE (isAOCompatible(  {NO_ACTION_OBSERVATION,  aoi{NO_ACTION,1}},       {} ));
    EXPECT_TRUE (isAOCompatible(  {aoi{NO_ACTION,1},       aoi{NO_ACTION,1}},       {} ));
    EXPECT_TRUE (isAOCompatible(  {NO_ACTION_OBSERVATION,  aoi{1, NO_OBSERVATION}}, {} ));
    EXPECT_TRUE (isAOCompatible(  {aoi{1, NO_OBSERVATION}, aoi{1, NO_OBSERVATION}}, {} ));
    EXPECT_TRUE (isAOCompatible(  {aoi{1, 1},              aoi{1, 1}},              {} ));
    // the other way around
    EXPECT_TRUE (isAOCompatible(  {}, {NO_ACTION_OBSERVATION,  NO_ACTION_OBSERVATION}  ));
    EXPECT_TRUE (isAOCompatible(  {}, {NO_ACTION_OBSERVATION,  aoi{NO_ACTION,1}}       ));
    EXPECT_TRUE (isAOCompatible(  {}, {aoi{NO_ACTION,1},       aoi{NO_ACTION,1}}       ));
    EXPECT_TRUE (isAOCompatible(  {}, {NO_ACTION_OBSERVATION,  aoi{1, NO_OBSERVATION}} ));
    EXPECT_TRUE (isAOCompatible(  {}, {aoi{1, NO_OBSERVATION}, aoi{1, NO_OBSERVATION}} ));
    EXPECT_TRUE (isAOCompatible(  {}, {aoi{1, 1},              aoi{1, 1}}              ));

    // both seq size 1 (cartesian product of actions {NO_ACTION, 1, 2} and observations {NO_OBSERVATION, 1, 2}
    EXPECT_TRUE (isAOCompatible( {aoi{NO_ACTION, NO_OBSERVATION}}, {aoi{NO_ACTION, NO_OBSERVATION}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{NO_ACTION, NO_OBSERVATION}}, {aoi{NO_ACTION, 1}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{NO_ACTION, NO_OBSERVATION}}, {aoi{NO_ACTION, 2}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{NO_ACTION, 1}},              {aoi{NO_ACTION, NO_OBSERVATION}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{NO_ACTION, 1}},              {aoi{NO_ACTION, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 1}},              {aoi{NO_ACTION, 2}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{NO_ACTION, 2}},              {aoi{NO_ACTION, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 2}},              {aoi{NO_ACTION, 1}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{NO_ACTION, 2}},              {aoi{NO_ACTION, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, NO_OBSERVATION}}, {aoi{1, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, NO_OBSERVATION}}, {aoi{1, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, NO_OBSERVATION}}, {aoi{1, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 1}},              {aoi{1, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 1}},              {aoi{1, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 1}},              {aoi{1, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 2}},              {aoi{1, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 2}},              {aoi{1, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 2}},              {aoi{1, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, NO_OBSERVATION}}, {aoi{2, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, NO_OBSERVATION}}, {aoi{2, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, NO_OBSERVATION}}, {aoi{2, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 1}},              {aoi{2, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 1}},              {aoi{2, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 1}},              {aoi{2, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 2}},              {aoi{2, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 2}},              {aoi{2, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{NO_ACTION, 2}},              {aoi{2, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, NO_OBSERVATION}},         {aoi{NO_ACTION, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, NO_OBSERVATION}},         {aoi{NO_ACTION, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, NO_OBSERVATION}},         {aoi{NO_ACTION, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 1}},                      {aoi{NO_ACTION, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 1}},                      {aoi{NO_ACTION, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 1}},                      {aoi{NO_ACTION, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 2}},                      {aoi{NO_ACTION, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 2}},                      {aoi{NO_ACTION, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 2}},                      {aoi{NO_ACTION, 2}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{1, NO_OBSERVATION}},         {aoi{1, NO_OBSERVATION}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{1, NO_OBSERVATION}},         {aoi{1, 1}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{1, NO_OBSERVATION}},         {aoi{1, 2}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{1, 1}},                      {aoi{1, NO_OBSERVATION}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{1, 1}},                      {aoi{1, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 1}},                      {aoi{1, 2}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{1, 2}},                      {aoi{1, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 2}},                      {aoi{1, 1}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{1, 2}},                      {aoi{1, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, NO_OBSERVATION}},         {aoi{2, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, NO_OBSERVATION}},         {aoi{2, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, NO_OBSERVATION}},         {aoi{2, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 1}},                      {aoi{2, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 1}},                      {aoi{2, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 1}},                      {aoi{2, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 2}},                      {aoi{2, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 2}},                      {aoi{2, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{1, 2}},                      {aoi{2, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, NO_OBSERVATION}},         {aoi{NO_ACTION, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, NO_OBSERVATION}},         {aoi{NO_ACTION, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, NO_OBSERVATION}},         {aoi{NO_ACTION, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 1}},                      {aoi{NO_ACTION, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 1}},                      {aoi{NO_ACTION, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 1}},                      {aoi{NO_ACTION, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 2}},                      {aoi{NO_ACTION, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 2}},                      {aoi{NO_ACTION, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 2}},                      {aoi{NO_ACTION, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, NO_OBSERVATION}},         {aoi{1, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, NO_OBSERVATION}},         {aoi{1, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, NO_OBSERVATION}},         {aoi{1, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 1}},                      {aoi{1, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 1}},                      {aoi{1, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 1}},                      {aoi{1, 2}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 2}},                      {aoi{1, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 2}},                      {aoi{1, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 2}},                      {aoi{1, 2}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{2, NO_OBSERVATION}},         {aoi{2, NO_OBSERVATION}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{2, NO_OBSERVATION}},         {aoi{2, 1}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{2, NO_OBSERVATION}},         {aoi{2, 2}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{2, 1}},                      {aoi{2, NO_OBSERVATION}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{2, 1}},                      {aoi{2, 1}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 1}},                      {aoi{2, 2}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{2, 2}},                      {aoi{2, NO_OBSERVATION}} ));
    EXPECT_FALSE(isAOCompatible( {aoi{2, 2}},                      {aoi{2, 1}} ));
    EXPECT_TRUE (isAOCompatible( {aoi{2, 2}},                      {aoi{2, 2}} ));

    //@formatter:on
}

}
