//
// Created by Jacob on 07.08.2017.
//

#include "base.h"

Action::Action(int t_id): id(t_id) {}

Action::Action(int t_id,  const std::string& t_s): id(t_id), s(t_s){}


Observation::Observation(int t_id): id(t_id){}

Observation::Observation(int t_id,  const std::string& t_s):id(t_id), s(t_s){}


State::State() = default;

void State::getActions(std::vector<Action> &list, int player) {};


Outcome::Outcome( const std::vector<Observation> &t_ob, const std::vector<int> &t_rew):  ob(t_ob), rew(t_rew){}


Domain::Domain(int h, int w, State &r): width(w), height(h), root(r) {}