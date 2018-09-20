/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m4helper.h
 * Author: lol
 *
 * Created on March 28, 2018, 3:26 PM
 */

#ifndef M4HELPER_H
#define M4HELPER_H
#include <vector>
#include <string>
#include <functional>
#include <queue>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include "StreetsDatabaseAPI.h"
#include "m1helper.h"
#include "m2helper.h"
#include "m3helper.h"
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <math.h>
#include <cmath>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/core/cs.hpp>
#include <limits>

struct DeliveryInfo {

    DeliveryInfo(unsigned pick_up, unsigned drop_off)
        :pickUp(pick_up), dropOff(drop_off) {}

    unsigned pickUp;
    unsigned dropOff;

};


std::vector<unsigned> traveling_courier(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots, const float turn_penalty);

std::pair<vector<unsigned>, vector<unsigned>> greedyAlgoAnother(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots, const float turn_penalty);

unsigned calculateStartBestDepot (const std::vector<unsigned>& depots, const std::set<unsigned>& pickUps);
unsigned calculateStartEndDepot (const std::vector<unsigned>& depots, unsigned currInter);
std::vector<unsigned> greedyAlgo(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots, const float turn_penalty);
pair<unsigned, vector<unsigned>> betterPlace(const std::vector<unsigned>& considerThese, const float turn_penalty, unsigned currInt);
double distoPath(vector <unsigned> intersectionInOrder);
vector <unsigned>  twoOpt (vector <unsigned> intersectionInOrder, unordered_map <unsigned, vector <unsigned>> dropOfftoPickUps);
bool isLegal (const vector <unsigned> intersectionOrder, vector<unsigned>::iterator itersUno, vector<unsigned>::iterator itersDos, unordered_map <unsigned, vector <unsigned>> dropOfftoPickUps);
vector <unsigned> createPath(const vector <unsigned> intersectionOrder, const float turn_penalty);

typedef pair<point, set<unsigned>::iterator> iteratLatLonPairCart;

#endif /* M4HELPER_H */

