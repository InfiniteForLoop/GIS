/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   m2.h
 * Author: zhan2464
 *
 * Created on February 14, 2018, 3:25 PM
 */

#ifndef M2HELPER_H
#define M2HELPER_H
#include "m1helper.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <cstdlib>
#include <vector>
#include <string>
#include "graphics.h"
#include "StreetsDatabaseAPI.h"
#include <readline/readline.h>
#include <readline/history.h>



using namespace std;
/* 
 * Copyright 2018 University of Toronto
 *
 * Permission is hereby granted, to use this software and associated 
 * documentation files (the "Software") in course work at the University 
 * of Toronto, or for personal use. Other uses are prohibited, in 
 * particular the distribution of the Software either publicly or to third 
 * parties.
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

//Draws the map which has been loaded with load_map().
//Your main() program should do the same.
enum diffZoomLevels{
    ZoomedOut = 0,
    ZoomedInLvl1,
    ZoomedInLvl2,
    ZoomedInLvl3,
    ZoomedInLvl4,
    ZoomedInLvl5,
    ZoomedInLvl6,
    ZoomedInLvl7,
    ZoomedInLvl8,
    ZoomedInMax
};

void draw_map();

// parsing input for map
bool mapParser ();

// Callbacks for event-driven window handling.
void drawscreen(void);
void act_on_new_button_func(void (*drawscreen_ptr) (void));
void act_on_button_press(float x, float y, t_event_buttonPressed event);
void act_on_mouse_move(float x, float y);
void act_on_key_press(char c, int keysym);

// A handy delay function for the animation example
void delay(long milliseconds);

// draw functions
void drawStreetSegs(diffZoomLevels queryZoom);
void drawFeatures(diffZoomLevels queryZoom);
void drawStreetName(vector <t_point> streetSegCoords, int layer, int j, diffZoomLevels queryZoom);
void drawOpenFeatureName(vector <t_point> streetSegCoords, int layer, int i, diffZoomLevels queryZoom);
void drawClosedFeatureName(vector <t_point> streetSegCoords, int layer, int i, diffZoomLevels queryZoom);
void drawArrow(diffZoomLevels queryZoom);
void drawAllFeatureNames(diffZoomLevels queryZoom);
void drawAllStreetNames(diffZoomLevels queryZoom);
void drawAllPOI(diffZoomLevels queryZoom);
void drawArrow(t_point from, t_point to);

// find button
void find(void (*drawscreen_ptr) (void));
vector <unsigned> findReturn();
void pathfindingQuery(void (*drawscreen_ptr) (void));
void POIfindingQuery(void (*drawscreen_ptr) (void));
string autocomplete(string street);

//toggle functions
void togglePath(void (*drawscreen_ptr) (void));
void toggleFeatures(void (*drawscreen_ptr) (void));
void toggleStreetNames(void (*drawscreen_ptr) (void));
void toggleFeatureNames(void (*drawscreen_ptr) (void));
void toggleArrows(void (*drawscreen_ptr) (void));
void togglePOI(void (*drawscreen_ptr) (void));

// zoom
diffZoomLevels currentzoomlevel();

// useful conversions
double areaOfFeature(vector <t_point> connectedDots);
LatLon xytolatlon(double x, double y);

//Search bars
void drawSearchBar ();
void drawUserWords ();

// global variables
extern double heightInit, currentHights;
extern unsigned zoomFactor;
extern std::string map_path;
extern unsigned closestIntersectionPOI;

//documentation
void help(void (*drawscreen_ptr) (void));


//autocomplete
char** command_completion(const char* stem_text, int start, int end);
char* intersection_name_generator(const char* stem_text, int state);

void multi_path(void (*drawscreen_ptr) (void));
void drawMultiPath();



#endif /* M2HELPER_H */

