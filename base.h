//
// Created by rozliv on 07.08.2017.
//

#ifndef PURSUIT_BASE_H
#define PURSUIT_BASE_H

#include <iostream>
#include <vector>


class Action {
public:
    explicit Action(int t_id);

    Action(int t_id, const std::string& t_s);

    inline const std::string& getDesc() const{
        return s;
    }

    inline int getID() const{
        return id;
    }

private:
    int id;
    std::string s;
};

class Observation {
public:
    explicit Observation(int t_id);

    Observation(int t_id, const std::string& t_s);

    inline const std::string& getDesc() const{
        return s;
    }

    inline int getID() const{
        return id;
    }

private:
    int id;
    std::string s;
};


class State {
public:
    explicit State();

    void getActions(std::vector<Action>&list ,int player);
};


class Outcome{
public:
    Outcome(const std::vector<Observation> &t_ob, const std::vector<int> &t_rew);

    inline const std::vector<Observation>& getObs() const {
        return ob;
    }

    inline const std::vector<int>& getReward() const {
        return rew;
    }

private:
    std::vector<Observation> ob;
    std::vector<int> rew;
};

class Domain {
public:
    explicit Domain(int h,int w,  State &r);

    inline int getHeight() const{
        return height;
    }

    inline int getWidth() const{
        return  width;
    }

private:
    int width;
    int height;
    State root;
};

#endif //PURSUIT_BASE_H
