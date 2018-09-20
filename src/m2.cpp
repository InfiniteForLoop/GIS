#include "m2helper.h"
#include "m3helper.h"
#include "m4helper.h"
#include <X11/keysym.h>
//useful toggle/data structs
static bool line_entering_demo = false;
static t_point rubber_pt; // Last point to which we rubber-banded.
static std::vector<t_point> line_pts; // Stores the points entered by user clicks.
static std::vector<t_point> POI_pts;
string map_path;
string fulls = "";
double previousLength = 0;
static vector <string> searchBarInput;
double heightInit, currentHights;
unsigned zoomFactor;
static vector <t_point> found_intersectionIDs;
// start and ending coordinates for pathfinding functions
static t_point pathintersectionStart, pathintersectionEnd;
static t_point pathintersectionStartClick, pathintersectionEndClick;
// vector of street segments for pathfinding in clicking and button functionalituy
static vector<unsigned> pathFinding;
static vector<unsigned> pathFindingClick;
// utility variables for starting and ending intersection IDs
static unsigned pathStart;
static unsigned pathEnd;
bool isTwice = false;
// toggle for buttons
bool togglePathFinding = false;
bool toggleFeaturesBool = true;
bool toggleStreetNamesBool = true;
bool toggleFeatureNamesBool = true;
bool toggleArrowsBool = true;
bool togglePOIBool = false;
bool exitFlag = false;
unsigned closestIntersectionPOI;
vector<unsigned> multiPathsMULTI;
vector<unsigned> intersectionOrderMULTI;

//preset colours
t_color BEACH= t_color(238,232,170,255);
t_color LAKE= t_color(135,206,250,255);
t_color ISLAND= t_color(238,232,170,255);
t_color GREENSPACE= t_color(34,139,34,255);
t_color PARK= t_color(144,238,144,255);
t_color RIVER= t_color(135,206,250,255);
t_color SHORELINE= t_color(169,169,169,255);
t_color GOLFCOURSE= t_color(154,205,50,255);
t_color BUILDING= t_color(250,235,215,255);
t_color STREAM= t_color(135,206,250,255);

//creates gui
void draw_map() {
    const t_bound_box initial_coords = t_bound_box(bottomleftx, bottomlefty, toprightx, toprighty);
    heightInit = abs(initial_coords.get_height());
    std::cout << "About to start graphics.\n";

    /**************** initialize display **********************/

    const t_color BGCOLOUR = t_color(220,220,220,255);
    init_graphics("Map of " + map_path,BGCOLOUR); // you could pass a t_color RGB triplet instead
    set_drawing_buffer(OFF_SCREEN);
    set_visible_world(initial_coords);
    update_message("Map Loaded");
    set_keypress_input(true);
    set_mouse_move_input(true);
    line_entering_demo = true;
    //t_bound_box old_coords = get_visible_world(); // save the current view for later;
    create_button("Window", "Find", find); // name is UTF-8
    create_button("Find", "Nature Off", toggleFeatures);
    create_button("Nature Off", "Names Off", toggleStreetNames);
    create_button("Names Off", "F Names Off", toggleFeatureNames);
    create_button("F Names Off", "Directions Off", toggleArrows);
    create_button("Directions Off", "POI On", togglePOI);
    create_button("POI On", "PathFinding", pathfindingQuery);
    create_button("PathFinding", "POIFinding", POIfindingQuery);
    create_button("POIFinding", "Path Off", togglePath);
    create_button("Path Off", "Help", help);
    create_button( "Help", "Multi", multi_path);
    

    event_loop(act_on_button_press, act_on_mouse_move, act_on_key_press, drawscreen); //returns when proceed is pressed
    close_graphics();
    
    pathFinding.clear();
    
}
/**
 * Draws screen
 * calls all individual modules to draw the entire gui
 */
void drawscreen(void) {
    diffZoomLevels queryZoom;
    set_draw_mode(DRAW_NORMAL); // Should set this if your program does any XOR drawing in callbacks.
    clearscreen(); 
    
    queryZoom = currentzoomlevel();
    
    if (toggleFeaturesBool) {
       drawFeatures(queryZoom);
    }
       drawStreetSegs(queryZoom);

    if (toggleStreetNamesBool) {
       drawAllStreetNames(queryZoom);
    }
    if (toggleFeatureNamesBool) {
       drawAllFeatureNames(queryZoom);
    }
    if (toggleArrowsBool) {
       drawArrow(queryZoom);
    }
    if (togglePOIBool) {
        drawAllPOI(queryZoom);
    }
    
    if(multiPathsMULTI.size()!=0){
        drawMultiPath();
    }
    
       //Draws Nearest Intersection
    for (unsigned int i = 0; i < line_pts.size(); i++){
        setcolor(RED);
        fillarc(line_pts[i], currentHights/101, 0, 360);
    }
       //Draws Nearest Intersection
    for (unsigned int i = 0; i < line_pts.size(); i++){
        setcolor(RED);
        fillarc(line_pts[i], currentHights/101, 0, 360);
    }
       //Draws Nearest POI
    for(unsigned int i = 0; i < POI_pts.size(); i++) {
        setcolor(BLUE);  
        fillarc(POI_pts[i], currentHights/101, 0, 360);
    }
    
    for(unsigned int i = 0; i < found_intersectionIDs.size(); i++) {
        setcolor(GREEN);  
        fillarc(found_intersectionIDs[i], currentHights/101, 0, 360);
    }
    
    //populate a vector with all coordinate points of a street segment
    vector <vector<t_point>> pathFindingCoord(pathFinding.size());
    for(unsigned i = 0; i < pathFinding.size() ; i++) {
        pathFindingCoord[i].push_back(latlontoxy(interPositGlobal[streetSegInfoGlobal[pathFinding[i]].from]));
        for (unsigned j = 0; j < streetSegInfoGlobal[pathFinding[i]].curvePointCount; j++) {    
            pathFindingCoord[i].push_back(latlontoxy(getStreetSegmentCurvePoint(pathFinding[i], j)));
        }
        pathFindingCoord[i].push_back(latlontoxy(interPositGlobal[streetSegInfoGlobal[pathFinding[i]].to]));
    }
    // draws the found path for the search pathfinding
    if (togglePathFinding == false) {
        if (queryZoom == ZoomedOut || queryZoom == ZoomedInLvl1) {
            setlinewidth(zoomFactor);
            for (unsigned i = 0; i < pathFinding.size(); i++) {
                if (streetSegInfoGlobal[pathFinding[i]].speedLimit >= 80) {
                    setlinewidth(zoomFactor);
                    vector <t_point> streetSegCoords = pathFindingCoord[i];
                    if (streetSegCoords.size() > 1) { // street seg is significant/L1
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            setcolor(ORANGE);
                            drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                        }
                    }
                }
            }
        } else if (queryZoom == ZoomedInLvl2 || queryZoom == ZoomedInLvl3 || queryZoom == ZoomedInLvl4) {
            for (unsigned i = 0; i < pathFinding.size(); i++) {
                if ((streetSegInfoGlobal[pathFinding[i]].speedLimit < 80) && (streetSegInfoGlobal[pathFinding[i]].speedLimit >= 50)) {
                    vector <t_point> streetSegCoords = pathFindingCoord[i];
                    setlinewidth(zoomFactor * 0.6);
                    if (streetSegCoords.size() > 1) {
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            if (togglePathFinding == false) {
                                setcolor(ORANGE);
                                drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                            }
                        }
                    }
                }
            }
            for (unsigned i = 0; i < pathFinding.size(); i++) {
                if (streetSegInfoGlobal[pathFinding[i]].speedLimit >= 80) {
                    vector <t_point> streetSegCoords = pathFindingCoord[i];
                    setlinewidth(zoomFactor);
                    if (streetSegCoords.size() > 1) {
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            setcolor(ORANGE);
                            drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                        }
                    }
                }
            }
        }
        else {
            for (unsigned i = 0; i < pathFinding.size(); i++) {
                if (streetSegInfoGlobal[pathFinding[i]].speedLimit < 50) {
                    vector <t_point> streetSegCoords = pathFindingCoord[i];
                    setlinewidth(zoomFactor * 0.2);
                    if (streetSegCoords.size() > 1) {
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            setcolor(ORANGE);
                            drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                        }
                    }
                }

            }
            for (unsigned i = 0; i < pathFinding.size(); i++) {
                if ((streetSegInfoGlobal[pathFinding[i]].speedLimit < 80) && (streetSegInfoGlobal[pathFinding[i]].speedLimit >= 50)) {
                    vector <t_point> streetSegCoords = pathFindingCoord[i];
                    setlinewidth(zoomFactor * 0.6);
                    if (streetSegCoords.size() > 1) {
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            setcolor(ORANGE);
                            drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                        }
                    }
                }
            }
            for (unsigned i = 0; i < pathFinding.size(); i++) {
                if (streetSegInfoGlobal[pathFinding[i]].speedLimit >= 80) {
                    vector <t_point> streetSegCoords = pathFindingCoord[i];
                    setlinewidth(zoomFactor);
                    if (streetSegCoords.size() > 1) {
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            setcolor(ORANGE);
                            drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                        }
                    }
                }
            }
        }
    }
    
    // draws the found path between mouse clicks
    //populate a vector with all coordinate points between mouse clicked intersections
    vector <vector<t_point>> pathFindingClickCoord(pathFindingClick.size());
    for(unsigned i = 0; i < pathFindingClick.size() ; i++) {
        pathFindingClickCoord[i].push_back(latlontoxy(interPositGlobal[streetSegInfoGlobal[pathFindingClick[i]].from]));
        for (unsigned j = 0; j < streetSegInfoGlobal[pathFindingClick[i]].curvePointCount; j++) {    
            pathFindingClickCoord[i].push_back(latlontoxy(getStreetSegmentCurvePoint(pathFindingClick[i], j)));
        }
        pathFindingClickCoord[i].push_back(latlontoxy(interPositGlobal[streetSegInfoGlobal[pathFindingClick[i]].to]));
    }
    // draws the found path for the search pathfinding
    if (togglePathFinding == false) {
        if (queryZoom == ZoomedOut || queryZoom == ZoomedInLvl1) {
            setlinewidth(zoomFactor);
            for (unsigned i = 0; i < pathFindingClick.size(); i++) {
                if (streetSegInfoGlobal[pathFindingClick[i]].speedLimit >= 80) {
                    setlinewidth(zoomFactor);
                    vector <t_point> streetSegCoords = pathFindingClickCoord[i];
                    if (streetSegCoords.size() > 1) { // street seg is significant/L1
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            setcolor(RED);
                            drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                        }
                    }
                }
            }
        } else if (queryZoom == ZoomedInLvl2 || queryZoom == ZoomedInLvl3 || queryZoom == ZoomedInLvl4) {
            for (unsigned i = 0; i < pathFindingClick.size(); i++) {
                if ((streetSegInfoGlobal[pathFindingClick[i]].speedLimit < 80) && (streetSegInfoGlobal[pathFindingClick[i]].speedLimit >= 50)) {
                    vector <t_point> streetSegCoords = pathFindingClickCoord[i];
                    setlinewidth(zoomFactor * 0.6);
                    if (streetSegCoords.size() > 1) {
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            if (togglePathFinding == false) {
                                setcolor(RED);
                                drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                            }
                        }
                    }
                }
            }
            for (unsigned i = 0; i < pathFindingClick.size(); i++) {
                if (streetSegInfoGlobal[pathFindingClick[i]].speedLimit >= 80) {
                    vector <t_point> streetSegCoords = pathFindingClickCoord[i];
                    setlinewidth(zoomFactor);
                    if (streetSegCoords.size() > 1) {
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            setcolor(RED);
                            drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                        }
                    }
                }
            }
        }
        else {
            for (unsigned i = 0; i < pathFindingClick.size(); i++) {
                if (streetSegInfoGlobal[pathFindingClick[i]].speedLimit < 50) {
                    vector <t_point> streetSegCoords = pathFindingClickCoord[i];
                    setlinewidth(zoomFactor * 0.2);
                    if (streetSegCoords.size() > 1) {
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            setcolor(RED);
                            drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                        }
                    }
                }

            }
            for (unsigned i = 0; i < pathFindingClick.size(); i++) {
                if ((streetSegInfoGlobal[pathFindingClick[i]].speedLimit < 80) && (streetSegInfoGlobal[pathFindingClick[i]].speedLimit >= 50)) {
                    vector <t_point> streetSegCoords = pathFindingClickCoord[i];
                    setlinewidth(zoomFactor * 0.6);
                    if (streetSegCoords.size() > 1) {
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            setcolor(RED);
                            drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                        }
                    }
                }
            }
            for (unsigned i = 0; i < pathFindingClick.size(); i++) {
                if (streetSegInfoGlobal[pathFindingClick[i]].speedLimit >= 80) {
                    vector <t_point> streetSegCoords = pathFindingClickCoord[i];
                    setlinewidth(zoomFactor);
                    if (streetSegCoords.size() > 1) {
                        for (unsigned j = 0; j < (streetSegCoords.size() - 1); j++) {
                            setcolor(RED);
                            drawline(streetSegCoords[j], streetSegCoords[j + 1]);
                        }
                    }
                }
            }
        }
    }
    // write start and end text for button and clicking
    if (isTwice == true) {    
        setcolor(BLACK);
        drawtext(pathintersectionStartClick, "START", FLT_MAX, FLT_MAX);
        drawtext(pathintersectionEndClick, "END", FLT_MAX, FLT_MAX);
    }
    if (pathFinding.size() > 0) {    
        setcolor(BLACK);
        drawtext(pathintersectionStart, "START", FLT_MAX, FLT_MAX);
        drawtext(pathintersectionEnd, "END", FLT_MAX, FLT_MAX);
    }
    
    //search bar stuff
    drawSearchBar();
    drawUserWords();
    
    POI_pts.clear();
    line_pts.clear();
    copy_off_screen_buffer_to_screen();

}



void act_on_button_press(float x, float y, t_event_buttonPressed event) {

    /* Called whenever event_loop gets a button press in the graphics *
     * area.  Allows the user to do whatever he/she wants with button *
     * clicks.
     *                                                        */
    LatLon latlonPressed, nearestlatlon;
    t_point highlightThis;
    latlonPressed = xytolatlon(x, y);
    
    if (event.shift_pressed) {
        unsigned intID = find_closest_point_of_interest(latlonPressed);
        nearestlatlon = POICoordinatesGlobal[intID];
        highlightThis = latlontoxy(nearestlatlon);    
        POI_pts.push_back(highlightThis);
        std::cout << POINamesGlobal[intID] << endl;
    }
    else if (event.ctrl_pressed) {
            if(isTwice == true) {
                pathStart = pathEnd;
                pathEnd = find_closest_intersection(latlonPressed);
            }
            else {
                isTwice = true;
                pathEnd = find_closest_intersection(latlonPressed);
            }
        
        pathFindingClick = find_path_between_intersections(pathStart, pathEnd, 0);
        if (pathFindingClick.size() == 0) {
            cout << "The clicked path is blocked!" << endl;
        }
        else {
            print_travel_directions(pathFindingClick, pathStart, pathEnd);
            pathintersectionEndClick = latlontoxy(interPositGlobal[pathEnd]);
            pathintersectionStartClick = latlontoxy(interPositGlobal[pathStart]);
            
        }
    }
    else {
        unsigned intID = find_closest_intersection(latlonPressed);
        nearestlatlon = interPositGlobal[intID];
        highlightThis = latlontoxy (nearestlatlon);

        line_pts.push_back(highlightThis);
    
        std::cout << interNamesGlobal[intID] << endl;
    }
    drawscreen(); 

    }

void act_on_mouse_move(float x, float y) {
    ;
}




void act_on_key_press(char c, int keysym) {
    // function to handle keyboard press event, the ASCII character is returned
    // along with an extended code (keysym) on X11 to represent non-ASCII
    // characters like the arrow keys.
    stringstream ss;
    string s;
    ss << c;
    ss >> s;
    cout << fulls;
    #ifdef X11 // Extended keyboard codes only supported for X11 for now
    if (keysym == XK_Right) {
        fulls += "&";
        searchBarInput.push_back(" . ");
    }
    #endif
    fulls += s;
    searchBarInput.push_back(s);
    
    drawscreen();
}

/**
 * Draws the street segments based on the zoom level
 * @param zoom level
 */
void drawStreetSegs(diffZoomLevels queryZoom){
    setlinestyle(SOLID);
    if (queryZoom == ZoomedOut || queryZoom == ZoomedInLvl1){
        setlinewidth(zoomFactor);

        for (unsigned i = 0; i < streetSegmappingL1.size(); i++){
        vector <t_point> streetSegCoords;
            streetSegCoords = streetSegmappingL1[i];
            if (streetSegCoords.size() > 1){
                for (unsigned j = 0; j < (streetSegCoords.size()-1); j++){
                    setcolor(YELLOW);
                    drawline(streetSegCoords[j], streetSegCoords[j+1]);
                }
            }      
        }
    }
    else if (queryZoom == ZoomedInLvl2 || queryZoom == ZoomedInLvl3 || queryZoom == ZoomedInLvl4){
        for (unsigned i = 0; i < streetSegmappingL2.size(); i++){
            vector <t_point> streetSegCoords;
            setlinewidth(zoomFactor*0.6);

            streetSegCoords = streetSegmappingL2[i];
            if (streetSegCoords.size() > 1){
                for (unsigned j = 0; j < (streetSegCoords.size()-1); j++){
                    setcolor(WHITE);
                    drawline(streetSegCoords[j], streetSegCoords[j+1]);
        }
            }      
        }
        for (unsigned i = 0; i < streetSegmappingL1.size(); i++){
            vector <t_point> streetSegCoords;
            setlinewidth(zoomFactor);

            streetSegCoords = streetSegmappingL1[i];
            if (streetSegCoords.size() > 1){
                for (unsigned j = 0; j < (streetSegCoords.size()-1); j++){
                    setcolor(YELLOW);
                    drawline(streetSegCoords[j], streetSegCoords[j+1]);
                }
            }      
        }
    }
    else{
        for (unsigned i = 0; i < streetSegmappingL3.size(); i++){
            vector <t_point> streetSegCoords;
            setlinewidth(zoomFactor*0.2);

            streetSegCoords = streetSegmappingL3[i];
            if (streetSegCoords.size() > 1){
                for (unsigned j = 0; j < (streetSegCoords.size()-1); j++){
                    setcolor(WHITE);
                    drawline(streetSegCoords[j], streetSegCoords[j+1]);
        }
            }      
        }
        for (unsigned i = 0; i < streetSegmappingL2.size(); i++){
            vector <t_point> streetSegCoords;
            setlinewidth(zoomFactor*0.6);

            streetSegCoords = streetSegmappingL2[i];
            if (streetSegCoords.size() > 1){
                for (unsigned j = 0; j < (streetSegCoords.size()-1); j++){
                    setcolor(WHITE);
                    drawline(streetSegCoords[j], streetSegCoords[j+1]);
        }
            }      
        }
        for (unsigned i = 0; i < streetSegmappingL1.size(); i++){
            vector <t_point> streetSegCoords;
            setlinewidth(zoomFactor);

            streetSegCoords = streetSegmappingL1[i];
            if (streetSegCoords.size() > 1){

                for (unsigned j = 0; j < (streetSegCoords.size()-1); j++){
                    setcolor(YELLOW);
                    drawline(streetSegCoords[j], streetSegCoords[j+1]);
                }
            }
        }
    }
    

}

/**
 * Draws the features based on zoom lvl, also draws them biggest->smallest
 * @param zoom lvl
 */

void drawFeatures(diffZoomLevels queryZoom){
    if (queryZoom == ZoomedOut || queryZoom == ZoomedInLvl1){
        for (auto iter = featuresmappingML1.rbegin(); iter != featuresmappingML1.rend(); iter++) {
            vector <t_point> featureCoords;
            t_color currentColor;
            if (featureType[((iter->second).second)] == Beach){
                currentColor=BEACH;
                setcolor(BEACH);
            }
            else if (featureType[((iter->second).second)] == Lake){
                currentColor=LAKE;
                setcolor(LAKE);
            }
            else if (featureType[((iter->second).second)] == Island){
                currentColor=ISLAND;
                setcolor(ISLAND);
            }
            else if (featureType[((iter->second).second)] == Greenspace){
                currentColor=GREENSPACE;
                setcolor(GREENSPACE);
            }
            featureCoords = (iter->second).first;
            unsigned sizee = featureCoords.size()-1;
            bool openFeature = true;
            if (featureCoords[0].x == featureCoords[sizee].x && featureCoords[0].y == featureCoords[sizee].y){
                openFeature = false;
            }
            
            if (openFeature == false){
                t_point* a = &featureCoords[0];
                fillpoly(a, featureCoords.size());
                //drawClosedFeatureName(featureCoords, 1, i, queryZoom);
            }
            
            else{
                setlinewidth(5);
                for (unsigned k = 0; k < (featureCoords.size()-1); k++){
                    setcolor(currentColor);
                    drawline(featureCoords[k], featureCoords[k+1]);
                    //drawOpenFeatureName(featureCoords, 1, i, queryZoom);

                }  
            }          
        } 
    }

    else if (queryZoom == ZoomedInLvl2 || queryZoom == ZoomedInLvl3 || queryZoom == ZoomedInLvl4){
        for (auto iter = featuresmappingML1.rbegin(); iter != featuresmappingML1.rend(); iter++) {
            vector <t_point> featureCoords;
            t_color currentColor;
            if (featureType[((iter->second).second)] == Beach){
                currentColor=BEACH;
                setcolor(BEACH);
            }
            else if (featureType[((iter->second).second)] == Lake){
                currentColor=LAKE;
                setcolor(LAKE);
            }
            else if (featureType[((iter->second).second)] == Island){
                currentColor=ISLAND;
                setcolor(ISLAND);
            }
            else if (featureType[((iter->second).second)] == Greenspace){
                currentColor=GREENSPACE;
                setcolor(GREENSPACE);
            }
            featureCoords = (iter->second).first;
            unsigned sizee = featureCoords.size()-1;
            bool openFeature = true;
            if (featureCoords[0].x == featureCoords[sizee].x && featureCoords[0].y == featureCoords[sizee].y){
                openFeature = false;
            }
            if (openFeature == false){
                t_point* a = &featureCoords[0];
                fillpoly(a, featureCoords.size());
                //drawClosedFeatureName(featureCoords, 1, i, queryZoom);
            }
            else{
                setlinewidth(5);
                for (unsigned k = 0; k < (featureCoords.size()-1); k++){
                    setcolor(currentColor);
                    drawline(featureCoords[k], featureCoords[k+1]);
                    //drawOpenFeatureName(featureCoords, 1, i, queryZoom);
                }    
            }
            featureCoords.clear();         
        } 
        for (auto iter = featuresmappingML2.rbegin(); iter != featuresmappingML2.rend(); iter++) {
            vector <t_point> featureCoords;
            t_color currentColor;      
            if (featureType[((iter->second).second)] == Park){
                currentColor=PARK;
                setcolor(PARK);
            }
            else if (featureType[((iter->second).second)] == River){
                currentColor=RIVER;
                setcolor(RIVER);
            }
            else if (featureType[((iter->second).second)] == Shoreline){
                currentColor=SHORELINE;
                setcolor(SHORELINE);
            }
            else if (featureType[((iter->second).second)] == Golfcourse){
                currentColor=GOLFCOURSE;
                setcolor(GOLFCOURSE);
            }
            
            featureCoords = (iter->second).first;
            unsigned sizee = featureCoords.size()-1;
            bool openFeature = true;
            if (featureCoords[0].x == featureCoords[sizee].x && featureCoords[0].y == featureCoords[sizee].y){
                openFeature = false;
            }
            
            if (openFeature == false){
                t_point* a = &featureCoords[0];
                fillpoly(a, featureCoords.size());
                //drawClosedFeatureName(featureCoords, 1, i, queryZoom);
            }
            
            else{
                setlinewidth(5);
                for (unsigned k = 0; k < (featureCoords.size()-1); k++){
                    setcolor(currentColor);
                    drawline(featureCoords[k], featureCoords[k+1]);
                    //drawOpenFeatureName(featureCoords, 1, i, queryZoom);

                }
            
            }
            featureCoords.clear();
            
        } 
    }
    else{
        for (auto iter = featuresmappingML1.rbegin(); iter != featuresmappingML1.rend(); iter++) {
            vector <t_point> featureCoords;
            t_color currentColor;
            if (featureType[((iter->second).second)] == Beach){
                currentColor=BEACH;
                setcolor(BEACH);
            }
            else if (featureType[((iter->second).second)] == Lake){
                currentColor=LAKE;
                setcolor(LAKE);
            }
            else if (featureType[((iter->second).second)] == Island){
                currentColor=ISLAND;
                setcolor(ISLAND);
            }
            else if (featureType[((iter->second).second)] == Greenspace){
                currentColor=GREENSPACE;
                setcolor(GREENSPACE);
            }
            featureCoords = (iter->second).first;
            unsigned sizee = featureCoords.size()-1;
            bool openFeature = true;
            if (featureCoords[0].x == featureCoords[sizee].x && featureCoords[0].y == featureCoords[sizee].y){
                openFeature = false;
            }
            
            if (openFeature == false){
                t_point* a = &featureCoords[0];
                fillpoly(a, featureCoords.size());
                //drawClosedFeatureName(featureCoords, 1, i, queryZoom);
            }
            
            else{
                setlinewidth(5);
                for (unsigned k = 0; k < (featureCoords.size()-1); k++){
                    setcolor(currentColor);
                    drawline(featureCoords[k], featureCoords[k+1]);
                    //drawOpenFeatureName(featureCoords, 1, i, queryZoom);

                } 
            }

            featureCoords.clear();
            
        } 
        for (auto iter = featuresmappingML2.rbegin(); iter != featuresmappingML2.rend(); iter++) {
            vector <t_point> featureCoords;
            t_color currentColor;
            
            if (featureType[((iter->second).second)] == Park){
                currentColor=PARK;
                setcolor(PARK);
            }
            else if (featureType[((iter->second).second)] == River){
                currentColor=RIVER;
                setcolor(RIVER);
            }
            else if (featureType[((iter->second).second)] == Shoreline){
                currentColor=SHORELINE;
                setcolor(SHORELINE);
            }
            else if (featureType[((iter->second).second)] == Golfcourse){
                currentColor=GOLFCOURSE;
                setcolor(GOLFCOURSE);
            }
            
            featureCoords = (iter->second).first;
            unsigned sizee = featureCoords.size()-1;
            bool openFeature = true;
            if (featureCoords[0].x == featureCoords[sizee].x && featureCoords[0].y == featureCoords[sizee].y){
                openFeature = false;
            }
            
            if (openFeature == false){
                t_point* a = &featureCoords[0];
                fillpoly(a, featureCoords.size());
                //drawClosedFeatureName(featureCoords, 1, i, queryZoom);
            }
            
            else{
                setlinewidth(5);
                for (unsigned k = 0; k < (featureCoords.size()-1); k++){
                    setcolor(currentColor);
                    drawline(featureCoords[k], featureCoords[k+1]);
                    //drawOpenFeatureName(featureCoords, 1, i, queryZoom);

                }
            }  

            featureCoords.clear();
            
        } 
        
        for (auto iter = featuresmappingML3.rbegin(); iter != featuresmappingML3.rend(); iter++) {
            vector <t_point> featureCoords;
            t_color currentColor;
            
            if (featureType[((iter->second).second)] == Building){
                currentColor=BUILDING;
                setcolor(BUILDING);
            }
            else if (featureType[((iter->second).second)] == Stream){
                currentColor=STREAM;
                setcolor(STREAM);
            }
            
            featureCoords = (iter->second).first;
            unsigned sizee = featureCoords.size()-1;
            bool openFeature = true;
            if (featureCoords[0].x == featureCoords[sizee].x && featureCoords[0].y == featureCoords[sizee].y){
                openFeature = false;
            }
            
            if (openFeature == false){
                t_point* a = &featureCoords[0];
                fillpoly(a, featureCoords.size());
                //drawClosedFeatureName(featureCoords, 1, i, queryZoom);
            }
            
            else{
                setlinewidth(5);
                for (unsigned k = 0; k < (featureCoords.size()-1); k++){
                    setcolor(currentColor);
                    drawline(featureCoords[k], featureCoords[k+1]);
                    //drawOpenFeatureName(featureCoords, 1, i, queryZoom);

                }            
            }

            featureCoords.clear();
            
        } 
    }
}

/**
 * parses in desired map name 
 * 
 */
bool mapParser () {
    string str;
    cout << "> ";  // Prompt for input
   
    getline(cin, str);
    stringstream linestream(str);  // Build a stringstream from the line.
      
    linestream >> map_path;
      
    if (!linestream.fail()) {
        cout << map_path << endl;
    }
    else {   // Extracting the command from the linestream failed.
        cout << "Error: invalid command\n";
        return false;
    }
    
    return true;
}


/**
 * Queries current zoom level, determines which level, updates global var
 */

diffZoomLevels currentzoomlevel(){
    double currY;
    diffZoomLevels currZoomLvl;
    t_bound_box currCoords = get_visible_world();
    currY = abs(currCoords.get_height());
    currentHights = currY;
    if ((currY/heightInit) >= 1){
        currZoomLvl = ZoomedOut;
        zoomFactor = 10;
    }
    else if ((currY/heightInit) >= 0.5 && (currY/heightInit) < 1){
        currZoomLvl = ZoomedInLvl1;
        zoomFactor = 10;
        //cout << currY << " Zoom L 1 ";
        //cout << heightInit << endl;
    }
    else if ((currY/heightInit) >= 0.25 && (currY/heightInit) < 0.5){
        currZoomLvl = ZoomedInLvl2;
        zoomFactor = 10;
        //cout << currY << " Zoom L 2 ";
        //cout << heightInit << endl;
    }
    else if ((currY/heightInit) >= 0.1875 && (currY/heightInit) < 0.25){
        currZoomLvl = ZoomedInLvl3;
        zoomFactor = 10;
        //cout << currY << " Zoom L 3 ";
        //cout << heightInit << endl;
    }
    else if ((currY/heightInit) >= 0.125 && (currY/heightInit) < 0.1875){
        currZoomLvl = ZoomedInLvl4;
        zoomFactor = 10;
        //cout << currY << " Zoom L 4 ";
        //cout << heightInit << endl;
    }
    else if ((currY/heightInit) >= 0.05 && (currY/heightInit) < 0.125){
        currZoomLvl = ZoomedInLvl5;
        zoomFactor = 10;
        //cout << currY << " Zoom L 5 ";
        //cout << heightInit << endl;
    }
    else if ((currY/heightInit) >= 0.0375 && (currY/heightInit) < 0.05){
        currZoomLvl = ZoomedInLvl6;
        zoomFactor = 10;
        //cout << currY << " Zoom L 6 ";
        //cout << heightInit << endl;
    }
    else if ((currY/heightInit) >= 0.025 && (currY/heightInit) < 0.0375){
        currZoomLvl = ZoomedInLvl7;
        zoomFactor = 10;
        //cout << currY << " Zoom L 7 ";
        //cout << heightInit << endl;
    }
    else if ((currY/heightInit) >= 0.0125 && (currY/heightInit) < 0.025){
        currZoomLvl = ZoomedInLvl8;
        zoomFactor = 10;
        //cout << currY << " Zoom L 8 ";
        //cout << heightInit << endl;
    }
    else{
        currZoomLvl = ZoomedInMax;
        zoomFactor = 10;
        //cout << currY << " Zoom L MAX ";
        //cout << heightInit << endl;
    }
    return currZoomLvl;
}


/**
 * autocompletes shortens street names, also does capitalization and whitespaces
 */
string autocomplete(string street) {
    
    string temp;
    stringstream streetstream;
    
    transform(street.begin(), street.end(), street.begin(), ::tolower); 
    
    stringstream linestream(street);
    while(!linestream.bad() && !linestream.eof()) {
        linestream >> temp;
        temp[0] = toupper(temp[0]);
        
        if((temp == "St.") || (temp == "St")) {
            temp = "Street";
        }
        else if((temp == "Ave.") || (temp == "Ave")) {
            temp = "Avenue";
        }
        else if((temp == "Rd.") || (temp == "Rd")) {
            temp = "Road";
        }
        else if((temp == "Blvd.") || (temp == "Blvd")) {
            temp = "Boulevard";
        }
        else if((temp == "Hwy.") || (temp == "Hwy")) {
            temp = "Highway";
        }
        else if(temp == "W") {
            temp = "West";
        }
        else if(temp == "N") {
            temp = "North";
        }
        else if(temp == "E") {
            temp = "East";
        }
        else if(temp == "S") {
            temp = "South";
        }
        streetstream << temp;

        
        if(!linestream.eof()) {
            streetstream << " ";
        }
        
    }
    getline(streetstream,street);
    cout << street << endl;
        
    return street;
}

void help(void (*drawscreen_ptr) (void)) {
    cout << "***** User Friendly Documentation *****" << endl;
    cout << "Thank you for choosing Team 65's GIS" << endl;
    cout << "Here are some quick tip about navigating our mapping tool: " << endl<<endl;
    cout << "1) We have included some toggle buttons to control what features " << endl;
    cout << "are displayed to reduce clutter." << endl<<endl;
    cout << "2) To use our find intersection function, click the find button" << endl;
    cout << "and go to the output terminal and enter the desired intersection." << endl<<endl;
    cout << "3) To find an intersection on the map, simply click and it will " << endl;
    cout << "highlight the nearest intersection in red and display the " << endl;
    cout << "name in the output terminal." << endl<<endl;
    cout << "4) To find a POI on the map, simply shift click and it will " << endl;
    cout << "highlight the nearest POI in blue and display the " << endl;
    cout << "name in the output terminal." << endl<<endl;
    cout << "5) To use our optimal path finding function, you can either " << endl;
    cout << "click PathFinding button and type the two intersections " << endl;
    cout << "in the output terminal, or control click two intersections" << endl;
    cout << "to find the fastest route. Directions are printed to the output terminal" << endl<<endl;
    cout << "6) To use our POI pathfinding function, click POI finding button" << endl;
    cout << "and enter the intersection start and POI destination." << endl<<endl;
    cout << "7) To change maps, simply click the Proceed button and go " << endl;
    cout << "to the output terminal and enter the desired location." << endl<<endl;
    
    
    
    drawscreen_ptr();
}

void togglePath(void (*drawscreen_ptr) (void)) {
    cout << "Path Finding ";
    if(togglePathFinding == false) {
        togglePathFinding = true;
        cout << "On" << endl;
        change_button_text("Path On", "Path Off");

    }
    else {
        togglePathFinding = false;
        cout << "Off" << endl;
        change_button_text("Path Off", "Path On");

    }
    drawscreen_ptr();
}

/**
 * Can turn features on or off
 */

void toggleFeatures(void (*drawscreen_ptr) (void)) {
    cout << "Features ";
    if(toggleFeaturesBool == false) {
        toggleFeaturesBool = true;
        cout << "On" << endl;
        change_button_text("Nature On", "Nature Off");

    }
    else {
        toggleFeaturesBool = false;
        cout << "Off" << endl;
        change_button_text("Nature Off", "Nature On");

    }
    drawscreen_ptr();
}

/**
 * Can turn street names on or off
 */
void toggleStreetNames(void (*drawscreen_ptr) (void)) {
    cout << "Street Names ";
    if(toggleStreetNamesBool == false) {
        toggleStreetNamesBool = true;
        cout << "On" << endl;
        change_button_text("Names On", "Names Off");
    }
    else {
        toggleStreetNamesBool = false;
        cout << "Off" << endl;
        change_button_text("Names Off", "Names On");
    }
    drawscreen_ptr();

}

/**
 * Can turn feature names on or off
 */

void toggleFeatureNames(void (*drawscreen_ptr) (void)) {
    cout << "Feature Names ";
    if(toggleFeatureNamesBool == false) {
        toggleFeatureNamesBool = true;
        cout << "On" << endl;
        change_button_text("F Names On", "F Names Off");
    }
    else {
        toggleFeatureNamesBool = false;
        cout << "Off" << endl;
        change_button_text("F Names Off", "F Names On");
    }
    
    drawscreen_ptr();
}

/**
 * Turn on/off one way street display
 */
void toggleArrows(void (*drawscreen_ptr) (void)) {
    cout << "Directions ";
    if(toggleArrowsBool == false) {
        toggleArrowsBool = true;
        cout << "On" << endl;
        change_button_text("Directions On", "Directions Off");
    }
    else {
        toggleArrowsBool = false;
        cout << "Off" << endl;
        change_button_text("Directions Off", "Directions On");
    }
    
    drawscreen_ptr();
}

/**
 * Enable/disable POI printing
 */
void togglePOI(void (*drawscreen_ptr) (void)) {
    cout << "POI ";
    if(togglePOIBool == false) {
        togglePOIBool = true;
        cout << "On" << endl;
        change_button_text("POI On", "POI Off");
    }
    else {
        togglePOIBool = false;
        cout << "Off" << endl;
        change_button_text("POI Off", "POI On");
    }
    
    drawscreen_ptr();
}

/**
 * Finds the intersection the user enters
 */
void find(void (*drawscreen_ptr) (void)) {    
    
    vector <unsigned> intersectionIDs;
    string streetOne, streetTwo;
    
    found_intersectionIDs.clear();
    
    cout << "Please enter an intersection. Press tab for autocomplete or exit or quit to quit. Make sure everything begins with quotations(\")" << endl;
    
    rl_bind_key('\t', rl_complete);
    rl_attempted_completion_function = command_completion;
    rl_completer_quote_characters = strdup("\"\'");
    
    char* buf;

    while((buf = readline("prompt>")) != NULL){
        
        if(strcmp(buf, "") != 0){
            add_history(buf);
        }
        
	if(strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0 || buf == NULL){
            cout<< "Thank you come again"<< endl;
            break;
	}
        else{
            string str(buf);
            str = str.substr(1, str.size() - 3);
            //cout << str << endl;
            intersectionIDs = interNametoInterGlobal[str];
    
            if (intersectionIDs.size() == 0) {
                cout << "These streets do not intersect!" << endl;
            }
            else {
                cout << "Found intersection at: " << endl;
                for(unsigned i = 0; i < intersectionIDs.size(); i++) {
                    cout << "X: " << interPositGlobal[intersectionIDs[i]].lat() << " Y: " << interPositGlobal[intersectionIDs[i]].lon() << endl;
                    found_intersectionIDs.push_back(latlontoxy(interPositGlobal[intersectionIDs[i]]));
                }
            }
    
            
            break;
        }
	free(buf);

	buf = NULL;

    }

    free(buf);

    
    // Re-draw the screen (a few squares are changing colour with time)
    drawscreen_ptr();
}

/**
 * draws street names at a matching angle by taking in the coordinates of 
 * both ends of the street segment
 */
void drawStreetName(vector <t_point> streetSegCoords, int layer, int i, diffZoomLevels queryZoom){
    
    
    double angle= 180/M_PI * atan((streetSegCoords[0].y-streetSegCoords[1].y)/(streetSegCoords[0].x-streetSegCoords[1].x));
    t_bound_box textsquare = t_bound_box(streetSegCoords[0], streetSegCoords[1]);
    setcolor(BLACK);
    settextattrs(8, 0);
    settextrotation(angle);
    double lengthSquared= pow((streetSegCoords[0].y-streetSegCoords[1].y),2) + pow((streetSegCoords[0].x-streetSegCoords[1].x),2);
    
    if(lengthSquared<0.0000000001){
        return;
    }
    
    
    if(layer==1&&streetSegMappingNamesL1[i]!="<unknown>"&&i%30==0&&
            (queryZoom==ZoomedInLvl2||queryZoom==ZoomedInLvl3||queryZoom==ZoomedInLvl4||queryZoom==ZoomedInLvl5||
            queryZoom==ZoomedInLvl6||queryZoom==ZoomedInLvl7||queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax)){
        drawtext(textsquare.get_center(), streetSegMappingNamesL1[i], FLT_MAX, FLT_MAX);
    } else if(layer==2&&streetSegMappingNamesL2[i]!="<unknown>"&&i%3==0&&
            (queryZoom==ZoomedInLvl6||queryZoom==ZoomedInLvl7||queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax)){
        drawtext(textsquare.get_center(),streetSegMappingNamesL2[i], FLT_MAX, FLT_MAX);
    } else if(layer==3&&streetSegMappingNamesL3[i]!="<unknown>"&&
            (queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax)){
        drawtext(textsquare.get_center(), streetSegMappingNamesL3[i], FLT_MAX, FLT_MAX);
    }
    
    
}


/**
 * draws open feature names similar to drawing street names, matches the angle
 * of the open feature by getting the coordinates of both ends of the feature 
 * segment
 */
void drawOpenFeatureName(vector <t_point> streetSegCoords, int layer, int i, diffZoomLevels queryZoom){
    
    
    double angle= 180/M_PI * atan((streetSegCoords[0].y-streetSegCoords[1].y)/(streetSegCoords[0].x-streetSegCoords[1].x));
    t_bound_box textsquare = t_bound_box(streetSegCoords[0], streetSegCoords[1]);
    setcolor(BLACK);
    settextattrs(8, 0);
    settextrotation(angle);
    
    
    if(layer==1&&featureTypeNameL1[i]!="<noname>"&&
             (queryZoom==ZoomedInLvl3||queryZoom==ZoomedInLvl4||queryZoom==ZoomedInLvl5||
            queryZoom==ZoomedInLvl6||queryZoom==ZoomedInLvl7||queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax)){
        drawtext(textsquare.get_center(), featureTypeNameL1[i], FLT_MAX, FLT_MAX);
    } else if(layer==2&&featureTypeNameL2[i]!="<noname>"&&
            (queryZoom==ZoomedInLvl7||queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax)){
        drawtext(textsquare.get_center(),featureTypeNameL2[i], FLT_MAX, FLT_MAX);
    } else if(layer==3&&featureTypeNameL3[i]!="<noname>"&&
            (queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax)){
        drawtext(textsquare.get_center(), featureTypeNameL3[i], FLT_MAX, FLT_MAX);
    }
    
}
/**
 * draws closed feature names by drawing the text and the first vertex of the
 * feature
 */
void drawClosedFeatureName(vector <t_point> streetSegCoords, int layer, int i, diffZoomLevels queryZoom){
   
    setcolor(BLACK);
    settextattrs(8, 0);
    
    if(layer==1&&featureTypeNameL1[i]!="<noname>"&&
             (queryZoom==ZoomedInLvl3||queryZoom==ZoomedInLvl4||queryZoom==ZoomedInLvl5||
            queryZoom==ZoomedInLvl6||queryZoom==ZoomedInLvl7||queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax)){
        drawtext(streetSegCoords[0], featureTypeNameL1[i], FLT_MAX, FLT_MAX);
    } else if(layer==2&&featureTypeNameL2[i]!="<noname>"&&
            (queryZoom==ZoomedInLvl7||queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax)){
        drawtext(streetSegCoords[0],featureTypeNameL2[i], FLT_MAX, FLT_MAX);
    } else if(layer==3&&featureTypeNameL3[i]!="<noname>"&&
            (queryZoom==ZoomedInMax)){
        drawtext(streetSegCoords[0], featureTypeNameL3[i], FLT_MAX, FLT_MAX);
    }
    
}

/**
 * draws one way street indicators according to the street segment data
 */

void drawArrow(diffZoomLevels queryZoom){
    
    
    
    for(unsigned i=0; i<toFromL1.size(); i++){
        
        vector <t_point> arrowDir=toFromL1[i];
        double angle= 180/M_PI * atan((arrowDir[1].y-arrowDir[0].y)/(arrowDir[1].x-arrowDir[0].x));
        if(arrowDir[1].x<arrowDir[0].x&&arrowDir[1].y>arrowDir[0].y){
            angle+=180;
        } else if(arrowDir[0].y>arrowDir[1].y&&arrowDir[0].x>arrowDir[1].x){
            angle+=180;
        }
            
        t_bound_box textsquare = t_bound_box(arrowDir[0], arrowDir[1]);
        setcolor(BLACK);
        settextattrs(8, 0);
        settextrotation(angle);
        
        if(i%3==0){
            continue;
        }
        
        
        if(queryZoom==ZoomedInLvl4||queryZoom==ZoomedInLvl5||
                queryZoom==ZoomedInLvl6||queryZoom==ZoomedInLvl7||queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax){
            drawtext(textsquare.get_center(), "->", FLT_MAX, FLT_MAX);
        }
        arrowDir.clear();
    }
        
     for(unsigned i=0; i<toFromL2.size(); i++){
        
        vector <t_point> arrowDir=toFromL2[i];
        double angle= 180/M_PI * atan((arrowDir[1].y-arrowDir[0].y)/(arrowDir[1].x-arrowDir[0].x));
        if(arrowDir[1].x<arrowDir[0].x&&arrowDir[1].y>arrowDir[0].y){
            angle+=180;
        }  else if(arrowDir[0].y>arrowDir[1].y&&arrowDir[0].x>arrowDir[1].x){
            angle+=180;
        }
        t_bound_box textsquare = t_bound_box(arrowDir[0], arrowDir[1]);
        setcolor(BLACK);
        settextattrs(8, 0);
        settextrotation(angle);
        
         if(i%3==0){
            continue;
        }
        
        if(queryZoom==ZoomedInLvl6||queryZoom==ZoomedInLvl7||queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax){
            drawtext(textsquare.get_center(),"->", FLT_MAX, FLT_MAX);
        }
        arrowDir.clear();
  
    }
     for(unsigned i=0; i<toFromL3.size(); i++){
        
        vector <t_point> arrowDir=toFromL3[i];
        double angle= 180/M_PI * atan((arrowDir[1].y-arrowDir[0].y)/(arrowDir[1].x-arrowDir[0].x));
        if(arrowDir[1].x<arrowDir[0].x&&arrowDir[1].y>arrowDir[0].y){
            angle+=180;
        } else if(arrowDir[0].y>arrowDir[1].y&&arrowDir[0].x>arrowDir[1].x){
            angle+=180;
        }
        t_bound_box textsquare = t_bound_box(arrowDir[0], arrowDir[1]);
        setcolor(BLACK);
        settextattrs(8, 0);
        settextrotation(angle);
        
         if(i%3==0){
            continue;
        }
        
       if(queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax){
            drawtext(textsquare.get_center(), "->", FLT_MAX, FLT_MAX);
        }   
        arrowDir.clear();
  
    }
    
}

/**
 * finds the area of a shape
 */
double areaOfFeature(vector <t_point> connectedDots){
    double area = 0;
    unsigned int sizer;
    sizer = connectedDots.size()-1;
    for(unsigned i = 0; i < sizer; i++){
        t_point point1 = connectedDots[i];
        t_point point2 = connectedDots[i+1];
        area += point1.x*point2.y - point1.y*point2.x;
    }
    t_point point1 = connectedDots[sizer];
    t_point point2 = connectedDots[0];
    area += point1.x*point2.y - point1.y*point2.x;
    area = abs(area/2);
    
    return area;
}
/**
 * convertss xy coord to LatLon coord
 * @param  x
 * @param  y
 */
LatLon xytolatlon(double x, double y){
    double point1, point2;
    point1 = (x/DEG_TO_RAD)/cos(latAvgGlobal * DEG_TO_RAD);
    point2 = y/DEG_TO_RAD;
    return LatLon(point2, point1);
}

/**
 * draws all feature names
 */
void drawAllFeatureNames(diffZoomLevels queryZoom){
    
    for (unsigned i = 0; i < featureTypeL1.size(); i++){
            vector <t_point> featureCoords;
            featureCoords = featuresmappingL1[i];
            unsigned sizee = featureCoords.size()-1;
            bool openFeature = true;
            if (featureCoords[0].x == featureCoords[sizee].x && featureCoords[0].y == featureCoords[sizee].y){
                openFeature = false;
            }
            if (openFeature == true){
               
                for (unsigned k = 0; k < (featureCoords.size()-1); k++){
                    drawOpenFeatureName(featureCoords, 1, i, queryZoom);
                }
            }
            else{
                drawClosedFeatureName(featureCoords, 1, i, queryZoom);
            }
            featureCoords.clear();
        }
        for (unsigned i = 0; i < featureTypeL2.size(); i++){
            vector <t_point> featureCoords;
            featureCoords = featuresmappingL2[i];
            unsigned sizee = featureCoords.size()-1;
            bool openFeature = true;
            if (featureCoords[0].x == featureCoords[sizee].x && featureCoords[0].y == featureCoords[sizee].y){
                openFeature = false;
            }
            if (openFeature == true){
                for (unsigned k = 0; k < (featureCoords.size()-1); k++){
                    drawOpenFeatureName(featureCoords, 2, i, queryZoom);
                }        
            }
            else{
                drawClosedFeatureName(featureCoords, 2, i, queryZoom);
            }
            featureCoords.clear();
        }
        
        for (unsigned i = 0; i < featureTypeL3.size(); i++){
            vector <t_point> featureCoords;
            featureCoords = featuresmappingL3[i];
            unsigned sizee = featureCoords.size()-1;
            bool openFeature = true;
            if (featureCoords[0].x == featureCoords[sizee].x && featureCoords[0].y == featureCoords[sizee].y){
                openFeature = false;
            }
            if (openFeature == true){
                setlinewidth(5);
                for (unsigned k = 0; k < (featureCoords.size()-1); k++){
                    drawOpenFeatureName(featureCoords, 3, i, queryZoom);
                }
            
            }
            else{
                drawClosedFeatureName(featureCoords, 3, i, queryZoom);
            }
            featureCoords.clear();
        }
    
}
/**
 * draws all street names
 */
void drawAllStreetNames(diffZoomLevels queryZoom){
    
    //draw all street names
    for (unsigned i = 0; i < streetSegmappingL3.size(); i++){
            vector <t_point> streetSegCoords;
            setlinewidth(3);
            
            streetSegCoords = streetSegmappingL3[i];
            if (streetSegCoords.size() > 1){
                for (unsigned j = 0; j < (streetSegCoords.size()-1); j++){
                    drawStreetName(streetSegCoords, 3, i, queryZoom);

                }
            }      
        }
    for (unsigned i = 0; i < streetSegmappingL2.size(); i++){
        vector <t_point> streetSegCoords;
            setlinewidth(8);
            
            streetSegCoords = streetSegmappingL2[i];
            if (streetSegCoords.size() > 1){
                for (unsigned j = 0; j < (streetSegCoords.size()-1); j++){   
                    drawStreetName(streetSegCoords, 2, i, queryZoom);

                }
            }      
        }
        for (unsigned i = 0; i < streetSegmappingL1.size(); i++){
            vector <t_point> streetSegCoords;
            setlinewidth(16);
            streetSegCoords = streetSegmappingL1[i];
            if (streetSegCoords.size() > 1){

                for (unsigned j = 0; j < (streetSegCoords.size()-1); j++){
                    drawStreetName(streetSegCoords, 1, i, queryZoom);
                }
            }
        }
}
/**
 * draws all POI

 */
void drawAllPOI(diffZoomLevels queryZoom){
    
    if(queryZoom == ZoomedInLvl7||queryZoom==ZoomedInLvl8||queryZoom==ZoomedInMax){
        for (unsigned int i = 0; i < POIPrinting.size(); i++){
            t_point tempLocation = POIPrinting[i];
            tempLocation.y += currentHights/100;
            setcolor(RED);
            fillarc(POIPrinting[i], currentHights/110, 0, 360);
            setcolor(BLACK);
            settextattrs(8, 0);
            drawtext (tempLocation.x, tempLocation.y, POINamesGlobal[i], FLT_MAX, FLT_MAX);
        }
    }     
}
/**
 * draws search bar

 */
void drawSearchBar(){
    set_coordinate_system(GL_SCREEN);
    setcolor(t_color(0,0,0,255/2));
    settextrotation(0);
    setlinestyle(SOLID);
    drawrect (10, 10, 400, 50);
    setcolor(t_color(220,224,232,255));
    fillrect (10, 10, 400, 50);
    set_coordinate_system(GL_WORLD);
}
/**
 * updates user typed words into search bar
*/
void drawUserWords(){
    set_coordinate_system(GL_SCREEN);
    setcolor(BLACK);
    settextrotation(0);
    for (unsigned int i = 0; i < searchBarInput.size(); i++){
        drawtext (55+i*7, 30, searchBarInput[i]);
    }
    
    set_coordinate_system(GL_WORLD);
}

// Button function for querying/finding the path between an intersection and a POI
void POIfindingQuery(void (*drawscreen_ptr) (void)) {
    vector <unsigned> temp;
    unsigned start;
    string end;

    pathFinding.clear();

    cout << "Finding path between an intersection and a POI. (Q to quit) " << endl;
    // first intersection
    cout << "Finding first intersection." << endl;
    do {
        temp = findReturn();
        if (exitFlag == true) {
            exitFlag = false;
            return;
        }
    } while(temp.size() == 0);
    
    start = temp[0];
    
    // second
    cout << "Finding POI." << endl;
    while(true) {
        cout << "Enter the name of the POI " << endl;
        cout << "> ";  // Prompt for input
        string temp;
        getline(cin,temp);
        stringstream linestream(temp);
        if(linestream.bad() || linestream.eof()) {
            linestream.clear();
        }
        else if (temp == "Q") {
                cout << "The pathfinding has been ended." << endl;
                return;
        }
        else {
            end = temp;
            break;
        }
    }
    
    pathFinding = find_path_to_point_of_interest(start, end, 0);
    
    if (pathFinding.size() == 0) {
        cout << "The path does not exist!" << endl;
    }
    else {
        cout << "The path has been drawn " << endl;
        print_travel_directions(pathFinding, start, 0);
        pathintersectionStart = latlontoxy(interPositGlobal[start]);
        pathintersectionEnd = latlontoxy(interPositGlobal[closestIntersectionPOI]);
    }
                
    // Re-draw the screen (a few squares are changing colour with time)
    drawscreen_ptr();
}

// Button function for querying/finding the path between two intersections
void pathfindingQuery(void (*drawscreen_ptr) (void)) {
    
    vector <unsigned> temp;
    unsigned start;
    unsigned end;

    pathFinding.clear();

    cout << "Finding path between intersections. " << endl;
    // first intersection
    cout << "Finding first intersection " << endl;
    do {
        temp = findReturn();
        if (exitFlag == true) {
            exitFlag = false;
            return;
        }
    } while(temp.size() == 0);
    
    start = temp[0];
    // second
    cout << "Finding second intersection. " << endl;
    do {
        temp = findReturn();
        if (exitFlag == true) {
            exitFlag = false;
            return;
        }
                 
    } while(temp.size() == 0);
    
    end = temp[0];
    
    pathFinding = find_path_between_intersections(start, end, 0);
    if (pathFinding.size() == 0) {
        cout << "The path is blocked!" << endl;
    }
    else {
        cout << "The path has been drawn " << endl;
        print_travel_directions(pathFinding, start, end);
        
        pathintersectionStart = latlontoxy(interPositGlobal[start]);
        pathintersectionEnd = latlontoxy(interPositGlobal[end]);
    }
                
    // Re-draw the screen (a few squares are changing colour with time)
    drawscreen_ptr();
}

// Parses user input for two street names, returning intersection ID
vector <unsigned> findReturn() {
    vector <unsigned> intersectionIDs;
    string streetOne, streetTwo;
    
    cout << "Please enter the an intersection. Press tab for autocomplete or exit or quit to quit. Make sure everything begins with quotations(\")" << endl;
    
    rl_bind_key('\t', rl_complete);
    rl_attempted_completion_function = command_completion;
    rl_completer_quote_characters = strdup("\"\'");
    
    char* buf;

    while((buf = readline("prompt>")) != NULL){
        
        if(strcmp(buf, "") != 0){
            add_history(buf);
        }
        
	if(strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0 || buf == NULL){
            cout<< "Thank you come again"<< endl;
            break;
	}
        else{
            string str(buf);
            str = str.substr(1, str.size() - 3);
            //cout << str << endl;
            intersectionIDs = interNametoInterGlobal[str];
    
            if (intersectionIDs.size() == 0) {
                cout << "These streets do not intersect!" << endl;
            }
            else {
                cout << "Found intersection at: " << endl;
                for(unsigned i = 0; i < intersectionIDs.size(); i++) {
                    cout << "X: " << interPositGlobal[intersectionIDs[i]].lat() << " Y: " << interPositGlobal[intersectionIDs[i]].lon() << endl;
                    found_intersectionIDs.push_back(latlontoxy(interPositGlobal[intersectionIDs[i]]));
                }
            }
    
            
            break;
        }
	free(buf);

	buf = NULL;

    }

    free(buf);
 
    return intersectionIDs;
}

char** command_completion (const char* stem_text, int start, int end){
	char**  matches = NULL;
	if (start != 0){
		matches = rl_completion_matches(stem_text, intersection_name_generator);
	}
	return matches;

}

char* intersection_name_generator(const char* stem_text, int state){

static int count;

	if(state == 0){
		count = -1;
	}

	int text_len = strlen(stem_text);
	while(count < (int) interNamesGlobalForA.size()-1){
		count++;
		if(strncmp(interNamesGlobalForA[count], stem_text, text_len) == 0){
			return strdup(interNamesGlobalForA[count]);
		}
	}

return NULL;
}

void multi_path(void (*drawscreen_ptr) (void)){
    
 
    multiPathsMULTI.clear();
    vector<unsigned> dropOffs;
    vector<unsigned> pickUps;
    vector<unsigned> depots;
    float turnPenalty;
    vector<DeliveryInfo> deliveries;

            
    cout << "Enter Pick Ups" << endl;
    int x=1;
    while(x>=0){
        
        cin >> x;
        if(x>=0){
            pickUps.push_back(x);
        }
    }
    
    cout << "Enter Drop Offs" << endl;
    int y=1;
    while(y>=0){
        
        cin >> y;
        if(y>=0){
            dropOffs.push_back(y);
        }
    }
    
    cout << "Enter Depots" << endl;
    int z=1;
    while(z>=0){
        
        cin >> z;
        if(z>=0){
            depots.push_back(z);
        }
    }
    
    cout<< "Enter Turn penalty"<< endl;
    

    cin>> turnPenalty;
    

    
    for(unsigned i=0; i<dropOffs.size(); i++){
        deliveries.push_back(DeliveryInfo(pickUps[i], dropOffs[i]));
        
    }
    
    
    pair <vector<unsigned>,vector<unsigned>> pathers = greedyAlgoAnother(deliveries, depots, turnPenalty);
    multiPathsMULTI=pathers.first;
    intersectionOrderMULTI= pathers.second;
//    vector<unsigned> intersectionOrder= traveling_courier(deliveriesMULTI,depotsMULTI, turnPenaltyMULTI);
//    intersectionOrderMULTI= intersectionOrder;
    return;
    
    
}

void drawMultiPath(){
    
    vector<t_point> multiPathCoords;
    
    for(unsigned i=0; i<multiPathsMULTI.size(); i++){
        
        StreetSegmentInfo aboutseg = streetSegInfoGlobal[multiPathsMULTI[i]];
        unsigned fromer= aboutseg.from;
        unsigned toer= aboutseg.to;
        vector <LatLon> temp = streetSegCurvesGlobal[multiPathsMULTI[i]];
        
        if (aboutseg.curvePointCount == 0){
            multiPathCoords.push_back(latlontoxy(interPositGlobal[fromer]));
            multiPathCoords.push_back(latlontoxy(interPositGlobal[toer]));
            
        }else {
            
            multiPathCoords.push_back(latlontoxy(interPositGlobal[fromer]));
            for(unsigned j=0; j<aboutseg.curvePointCount; j++){
                multiPathCoords.push_back(latlontoxy(temp[j]));
            }
            multiPathCoords.push_back(latlontoxy(interPositGlobal[toer]));
        }
        
    }
    
    
    
    for (unsigned j = 0; j < (multiPathCoords.size()-1); j++){
        setcolor(BLUE);
        drawline(multiPathCoords[j], multiPathCoords[j+1]);
    }
    
    
    for(unsigned i=0; i<intersectionOrderMULTI.size(); i++){
        string number= to_string(i+1);
        t_point tempLocation = latlontoxy(interPositGlobal[intersectionOrderMULTI[i]]);
        //tempLocation.y += currentHights/100;
        setcolor(GREEN);
        fillarc(latlontoxy(interPositGlobal[intersectionOrderMULTI[i]]), currentHights/110, 0, 360);
        setcolor(BLACK);
        settextattrs(8, 0);
        drawtext (tempLocation.x, tempLocation.y, number , FLT_MAX, FLT_MAX);
    }
    
    return;
}
