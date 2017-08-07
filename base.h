//
// Created by Jacob on 07.08.2017.
//

#ifndef PURSUIT_BASE_H
#define PURSUIT_BASE_H

#include <iostream>
#include <vector>


class Action {
    int id;
    std::string s;
public:
    explicit Action(int t_id);

    Action(int t_id, const std::string& t_s);

    inline const std::string& getDesc() const{
        return s;
    }

    inline int getID() const{
        return id;
    }
};

class Observation {
    int id;
    std::string s;
public:
    explicit Observation(int t_id);

    Observation(int t_id, const std::string& t_s);

    inline const std::string& getDesc() const{
        return s;
    }

    inline int getID() const{
        return id;
    }
};


class State {
public:

    explicit State();

    void getActions(std::vector<Action>&list ,int player);
};


class Outcome{
    std::vector<Observation> ob;
    std::vector<int> rew;
public:
    Outcome(const std::vector<Observation> &t_ob, const std::vector<int> &t_rew);

    inline const std::vector<Observation>& getObs() const {
        return ob;
    }

    inline const std::vector<int>& getReward() const {
        return rew;
    }
};

class Domain {
    int width;
    int height;
    State root;
public:
    explicit Domain(int h,int w,  State &r);

    inline int getHeight() const{
        return height;
    }

    inline int getWidth() const{
        return  width;
    }
};




#endif //PURSUIT_BASE_H
