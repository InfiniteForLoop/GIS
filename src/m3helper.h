/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m3helper.h
 * Author: zhan2464
 *
 * Created on March 11, 2018, 1:13 PM
 */

#ifndef M3HELPER_H
#define M3HELPER_H
#define INF 50000000000000000000000000


#include <vector>
#include <string>
#include <functional>
#include <queue>
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include "graphics.h"
#include "StreetsDatabaseAPI.h"
#include "m1helper.h"
#include "m2helper.h"
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <math.h>
#include <cmath>
#include <set>
#include <algorithm>


// path finding algorithms (required)
double compute_path_travel_time(const std::vector<unsigned>& path, const double turn_penalty);
std::vector<unsigned> find_path_between_intersections(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty);
std::vector<unsigned> find_path_to_point_of_interest(const unsigned intersect_id_start, const std::string point_of_interest_name, const double turn_penalty);

// printing the travel directions for a given path
void print_travel_directions(vector<unsigned> path);
void print_travel_directions(vector<unsigned> path, unsigned intersect_id_start, unsigned intersect_id_end);

// organizing a path for further use 
std::vector<unsigned> walkthruPath (vector<unsigned> Path,unsigned intersect_id_start, unsigned intersect_id_end);

// improvement functions for pathfinding algorithm
double heuristic (unsigned currNode, unsigned adjNode);
double perpscore (unsigned prevSeg, unsigned thisSeg, const double turn_penalty);
vector <unsigned> traceback (vector <unsigned> prevNodes, vector <unsigned> prevSegs, unsigned bgin, unsigned end);

typedef pair<double, unsigned> timeIntPair;

std::vector<unsigned> dijkstra(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty);


#endif /* M3HELPER_H */

