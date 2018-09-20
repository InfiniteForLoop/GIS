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

#include "m1helper.h"
#include "m2helper.h"

using namespace std;

// necessary to declare with use of  in m1helper.h

//intersection data
vector <string> interNamesGlobal;
vector <LatLon> interPositGlobal;
vector <vector <unsigned>> interStreetSegsGlobal;
vector <set <unsigned>> streetIDToIntersectsGlobal;
vector <vector <unsigned>> distanceFromCenterInterGlobal;
unordered_map <string, vector<unsigned>> streetNameToAllInterGlobal;
vector <const char*> interNamesGlobalForA;
unordered_map <string,vector<unsigned>> interNametoInterGlobal;

//street segment data
vector <StreetSegmentInfo> streetSegInfoGlobal;
vector <vector <LatLon>> streetSegCurvesGlobal;
vector <double> streetSegtoLengthGlobal;
vector <vector <unsigned>> streetToSegmentsGlobal;
vector <double> streetSegToTTimeGlobal;

//street data
vector <string> streetNamesGlobal;
unordered_map <string, vector<unsigned>> streetNameToIDGlobal;

// POI data
vector <LatLon> POICoordinatesGlobal;
vector <vector <unsigned>> distanceFromCenterPOIGlobal; //0th element within 10 km, 1st element within 20km etc
vector <string> POINamesGlobal;


// boost data
bgi::rtree<LatLonPairCart, bgi::quadratic < 16 >> rtreePOIGlobal, rtreeInterGlobal;

// count data
unsigned noStreetsGlobal, noSegsGlobal, noInterGlobal, noPOIGlobal, noFeatGlobal;


//for m2
vector <vector <t_point>> streetSegmapping;
vector <vector <t_point>> featuresmapping;
vector <FeatureType> featureType;
vector <double> streetsegSpeeds;
double maxlat, minlat, maxlon, minlon, latAvgGlobal;
double bottomleftx, bottomlefty, toprightx, toprighty;
vector <vector <t_point>> streetSegmappingL1, streetSegmappingL2, streetSegmappingL3;
vector <vector <t_point>> featuresmappingL1, featuresmappingL2, featuresmappingL3;
multimap <double, pair<vector<t_point>, unsigned>> featuresmappingML1, featuresmappingML2, featuresmappingML3;

vector <string> streetSegMappingNamesL1, streetSegMappingNamesL2, streetSegMappingNamesL3;
vector <string> featureTypeName;
vector <FeatureType> featureTypeL1, featureTypeL2, featureTypeL3;
vector <string> featureTypeNameL1, featureTypeNameL2, featureTypeNameL3;
vector <vector<t_point>> toFromL1, toFromL2, toFromL3; 
vector <t_point> POIPrinting;
vector <string> POINames;


//for m3
vector <unsigned> interToStreetSegOrder;


bool load_map(string fn) {
    bool load_successful;
    
    if (loadStreetsDatabaseBIN(fn) == false) {
        return false;
    }

    noStreetsGlobal = getNumberOfStreets();
    noSegsGlobal = getNumberOfStreetSegments();
    noInterGlobal = getNumberOfIntersections();
    noPOIGlobal = getNumberOfPointsOfInterest();
    noFeatGlobal = getNumberOfFeatures();
    maxlat = -100000;
    minlat = 100000;
    maxlon = -100000;
    minlon = 100000;

    for (unsigned i = 0; i < noInterGlobal; i++) {
        if (getIntersectionPosition(i).lat() > maxlat){
            maxlat = getIntersectionPosition(i).lat();
        }
        else if ((getIntersectionPosition(i).lat() < minlat)){
            minlat = getIntersectionPosition(i).lat();
        }  
        if (getIntersectionPosition(i).lon() > maxlon){
            maxlon = getIntersectionPosition(i).lon();
        }
        else if ((getIntersectionPosition(i).lon() < minlon)){
            minlon = getIntersectionPosition(i).lon();
        } 
    }
    latAvgGlobal = (minlat + maxlat)/2;
 
    for (unsigned i = 0; i < noStreetsGlobal; i++) {
        vector <unsigned> placeholderLOL;
        set <unsigned> setplaceholderLOL;
        vector <unsigned> placeholderstrID;
        streetNamesGlobal.push_back(getStreetName(i));
        streetToSegmentsGlobal.push_back(placeholderLOL);
        streetIDToIntersectsGlobal.push_back(setplaceholderLOL);
            
        //load streetNameToID 
        unordered_map <string, vector<unsigned>>::iterator it;
        it = streetNameToIDGlobal.find(streetNamesGlobal[i]);
        if (it == streetNameToIDGlobal.end()) {
            placeholderstrID.push_back(i);
            streetNameToIDGlobal.insert({streetNamesGlobal[i], placeholderstrID});
        } else {
            it->second.push_back(i);
        }
        placeholderstrID.clear();

    }

    //loads interNames, interPosit, interStreetSegs and rTreeInter
    for (unsigned i = 0; i < noInterGlobal; i++) {
        unsigned count = getIntersectionStreetSegmentCount(i);
        interNamesGlobal.push_back(getIntersectionName(i));
        string haHaaString = getIntersectionName(i);
        const char *cstr = haHaaString.c_str();
        interNamesGlobalForA.push_back(cstr);
        interPositGlobal.push_back(getIntersectionPosition(i));
        if (interNametoInterGlobal.count(haHaaString) == 0){
            vector <unsigned> kindaTemp;
            kindaTemp.push_back(i);
            interNametoInterGlobal.insert({haHaaString, kindaTemp});
        }
        else {
            interNametoInterGlobal[haHaaString].push_back(i);
        }
        
        vector <unsigned> Streetsegid;
        
        
        for (unsigned j = 0; j < count; j++) {
            Streetsegid.push_back(getIntersectionStreetSegment(i, j));
        }
        interStreetSegsGlobal.push_back(Streetsegid);

        point P1(interPositGlobal[i].lon(), interPositGlobal[i].lat());
        rtreeInterGlobal.insert(make_pair(P1, i));

        Streetsegid.clear();

    }

    
    //loads street segment data
    for (unsigned i = 0; i < noSegsGlobal; i++) {
        vector <LatLon> temp;
        vector <t_point> importantPoints, importantPointsL1, importantPointsL2, importantPointsL3;
        vector <t_point> oneWayVecL1, oneWayVecL2, oneWayVecL3;
        double length = 0;
        unsigned fromer, toer;
        StreetSegmentInfo aboutseg;
        unsigned noCurves;
        vector <LatLon> insertCurves;
        streetSegInfoGlobal.push_back(getStreetSegmentInfo(i));
        noCurves = streetSegInfoGlobal[i].curvePointCount;
        
        for (unsigned j = 0; j < noCurves; j++) {
            insertCurves.push_back(getStreetSegmentCurvePoint(i, j));
        }

        streetToSegmentsGlobal[streetSegInfoGlobal[i].streetID].push_back(i);
        streetIDToIntersectsGlobal[streetSegInfoGlobal[i].streetID].insert(streetSegInfoGlobal[i].from);
        streetIDToIntersectsGlobal[streetSegInfoGlobal[i].streetID].insert(streetSegInfoGlobal[i].to);
       
        
        
        streetSegCurvesGlobal.push_back(insertCurves);
        aboutseg = streetSegInfoGlobal[i];
        toer = aboutseg.to;
        fromer = aboutseg.from;
        temp = streetSegCurvesGlobal[i];
       
        if (aboutseg.curvePointCount != 0) {
            // loads street segment positions into layers and a multimap as well as the names
            importantPoints.push_back(latlontoxy(interPositGlobal[fromer]));
            importantPoints.push_back(latlontoxy(temp[0]));
            if (aboutseg.speedLimit >= 80){
                importantPointsL1.push_back(latlontoxy(interPositGlobal[fromer]));
                importantPointsL1.push_back(latlontoxy(temp[0]));
                if (aboutseg.oneWay == true){
                    oneWayVecL1.push_back(latlontoxy(interPositGlobal[fromer]));
                    oneWayVecL1.push_back(latlontoxy(temp[0]));
                }
            }
            else if(aboutseg.speedLimit >= 50 && aboutseg.speedLimit < 80){
                importantPointsL2.push_back(latlontoxy(interPositGlobal[fromer]));
                importantPointsL2.push_back(latlontoxy(temp[0]));
                if (aboutseg.oneWay == true){
                    oneWayVecL2.push_back(latlontoxy(interPositGlobal[fromer]));
                    oneWayVecL2.push_back(latlontoxy(temp[0]));
                }
            }
            else {
                importantPointsL3.push_back(latlontoxy(interPositGlobal[fromer]));
                importantPointsL3.push_back(latlontoxy(temp[0]));
                if (aboutseg.oneWay == true){
                    oneWayVecL3.push_back(latlontoxy(interPositGlobal[fromer]));
                    oneWayVecL3.push_back(latlontoxy(temp[0]));
                }
            }
            length = find_distance_between_two_points(interPositGlobal[fromer], temp[0]);

            for (unsigned j = 0; j < (streetSegCurvesGlobal[i].size() - 1); j++) {
                length += find_distance_between_two_points(temp[j], temp[j + 1]);
                if (aboutseg.speedLimit >= 80 ){
                    importantPointsL1.push_back(latlontoxy(temp[j+1]));
                }
                else if(aboutseg.speedLimit >= 50 && aboutseg.speedLimit < 80){
                    importantPointsL2.push_back(latlontoxy(temp[j+1]));
                }
                else {
                    importantPointsL3.push_back(latlontoxy(temp[j+1]));
                }
                importantPoints.push_back(latlontoxy(temp[j+1]));
                
            }
            length += find_distance_between_two_points(temp[(streetSegCurvesGlobal[i].size() - 1)], interPositGlobal[toer]);
            if (aboutseg.speedLimit >= 80 ){
                importantPointsL1.push_back(latlontoxy(temp[(streetSegCurvesGlobal[i].size() - 1)]));
                importantPointsL1.push_back(latlontoxy(interPositGlobal[toer]));
            }
            else if(aboutseg.speedLimit >= 50 && aboutseg.speedLimit < 80){
                importantPointsL2.push_back(latlontoxy(temp[(streetSegCurvesGlobal[i].size() - 1)]));
                importantPointsL2.push_back(latlontoxy(interPositGlobal[toer]));
            }
            else {
                importantPointsL3.push_back(latlontoxy(temp[(streetSegCurvesGlobal[i].size() - 1)]));
                importantPointsL3.push_back(latlontoxy(interPositGlobal[toer]));
            }
            importantPoints.push_back(latlontoxy(temp[(streetSegCurvesGlobal[i].size() - 1)]));
            importantPoints.push_back(latlontoxy(interPositGlobal[toer]));
            
        } 
        else {
            // loads street segment positions into layers and a multimap as well as the names
            if (aboutseg.speedLimit >= 80 ){
                importantPointsL1.push_back(latlontoxy(interPositGlobal[fromer]));
                importantPointsL1.push_back(latlontoxy(interPositGlobal[toer]));
                if (aboutseg.oneWay == true){
                    oneWayVecL1.push_back(latlontoxy(interPositGlobal[fromer]));
                    oneWayVecL1.push_back(latlontoxy(interPositGlobal[toer]));
                }
            }
            else if(aboutseg.speedLimit >= 50 && aboutseg.speedLimit < 80){
                importantPointsL2.push_back(latlontoxy(interPositGlobal[fromer]));
                importantPointsL2.push_back(latlontoxy(interPositGlobal[toer]));
                if (aboutseg.oneWay == true){
                    oneWayVecL2.push_back(latlontoxy(interPositGlobal[fromer]));
                    oneWayVecL2.push_back(latlontoxy(interPositGlobal[toer]));
                }
            }
            else {
                importantPointsL3.push_back(latlontoxy(interPositGlobal[fromer]));
                importantPointsL3.push_back(latlontoxy(interPositGlobal[toer]));
                if (aboutseg.oneWay == true){
                    oneWayVecL3.push_back(latlontoxy(interPositGlobal[fromer]));
                    oneWayVecL3.push_back(latlontoxy(interPositGlobal[toer]));
                }
            }
            importantPoints.push_back(latlontoxy(interPositGlobal[fromer]));
            importantPoints.push_back(latlontoxy(interPositGlobal[toer]));
            length = find_distance_between_two_points(interPositGlobal[fromer], interPositGlobal[toer]);
        }
        
        //loading StreetSegMapping containers
        if(aboutseg.speedLimit>=80){
            streetSegMappingNamesL1.push_back(getStreetName(aboutseg.streetID));
        } else if(aboutseg.speedLimit<80&&aboutseg.speedLimit>=50){
            streetSegMappingNamesL2.push_back(getStreetName(aboutseg.streetID));
        } else {
            streetSegMappingNamesL3.push_back(getStreetName(aboutseg.streetID));
        }
        
        //only pushing back non empty vectors
        if(importantPointsL1.size()>0){
            streetSegmappingL1.push_back(importantPointsL1);
        }
        if(importantPointsL2.size()>0){
            streetSegmappingL2.push_back(importantPointsL2);
        }
        if(importantPointsL3.size()>0){
            streetSegmappingL3.push_back(importantPointsL3);
        }

        if(oneWayVecL1.size()>0){
            toFromL1.push_back(oneWayVecL1);
        }
        if(oneWayVecL2.size()>0){
            toFromL2.push_back(oneWayVecL2);
        }
        if(oneWayVecL3.size()>0){
            toFromL3.push_back(oneWayVecL3);
        }



        
        streetSegtoLengthGlobal.push_back(length);
        streetSegToTTimeGlobal.push_back(3.6 * streetSegtoLengthGlobal[i] / streetSegInfoGlobal[i].speedLimit);
        streetsegSpeeds.push_back(streetSegInfoGlobal[i].speedLimit);
        insertCurves.clear();
        importantPoints.clear();
        importantPointsL1.clear();
        importantPointsL2.clear();
        importantPointsL3.clear();
        oneWayVecL3.clear();
        oneWayVecL2.clear();
        oneWayVecL1.clear();
    }

    //loads street name data
    for (unsigned i = 0; i < streetNamesGlobal.size(); i++) {
        vector<unsigned> street_id = streetNameToIDGlobal.find(streetNamesGlobal[i])->second;
        vector<unsigned> tempInterInserted;

        for (unsigned j = 0; j < street_id.size(); j++) {
            vector<unsigned> tempInterAdd;
            tempInterAdd = find_all_street_intersections(street_id[j]);
            tempInterInserted.insert(tempInterInserted.end(), tempInterAdd.begin(), tempInterAdd.end());
            tempInterAdd.clear();

        }
        streetNameToAllInterGlobal.insert({streetNamesGlobal[i], tempInterInserted});
        tempInterInserted.clear();
    }

    //loads POI data
    for (unsigned i = 0; i < noPOIGlobal; i++) {
        
        POINamesGlobal.push_back(getPointOfInterestName(i));
        POICoordinatesGlobal.push_back(getPointOfInterestPosition(i));
        point P1(POICoordinatesGlobal[i].lon(), POICoordinatesGlobal[i].lat());
        rtreePOIGlobal.insert(make_pair(P1, i));
        POIPrinting.push_back(latlontoxy(POICoordinatesGlobal[i]));
        
    }
    
    //load feature type data
    for (unsigned i = 0; i < noFeatGlobal; i++) {
        vector <t_point> shapeFeature;
        FeatureType whatFeature;
        double area;
        pair<vector<t_point>, unsigned> printThisFeat;
        unsigned numberofCoords = getFeaturePointCount(i);
        whatFeature = getFeatureType(i);
        featureType.push_back(whatFeature);
        featureTypeName.push_back(getFeatureName(i));
        if (whatFeature == Beach || whatFeature == Lake || whatFeature == Island || whatFeature == Greenspace){
            for (unsigned j = 0; j < numberofCoords; j++){
                shapeFeature.push_back(latlontoxy(getFeaturePoint(i,j)));
            }
            featuresmappingL1.push_back(shapeFeature);
            featureTypeL1.push_back(whatFeature);
            featureTypeNameL1.push_back(getFeatureName(i));
            area = areaOfFeature(shapeFeature);
            printThisFeat = make_pair(shapeFeature, i);
            featuresmappingML1.insert(pair<double, pair<vector<t_point>, unsigned>>(area, printThisFeat));
            featuresmappingL1.push_back(shapeFeature);
        }
        else if (whatFeature == Park || whatFeature == River || whatFeature == Golfcourse || whatFeature == Shoreline){
            for (unsigned j = 0; j < numberofCoords; j++){
                shapeFeature.push_back(latlontoxy(getFeaturePoint(i,j)));
            }
            featuresmappingL2.push_back(shapeFeature);
            featureTypeL2.push_back(whatFeature);
            featureTypeNameL2.push_back(getFeatureName(i));
            area = areaOfFeature(shapeFeature);
            printThisFeat = make_pair(shapeFeature, i);
            featuresmappingML2.insert(pair<double, pair<vector<t_point>, unsigned>>(area, printThisFeat));
        }
        else{
            for (unsigned j = 0; j < numberofCoords; j++){
                shapeFeature.push_back(latlontoxy(getFeaturePoint(i,j)));
            }
            featuresmappingL3.push_back(shapeFeature);
            featureTypeL3.push_back(whatFeature);
            featureTypeNameL3.push_back(getFeatureName(i));
            area = areaOfFeature(shapeFeature);
            printThisFeat = make_pair(shapeFeature, i);
            featuresmappingML3.insert(pair<double, pair<vector<t_point>, unsigned>>(area, printThisFeat));
        }
        
        
        shapeFeature.clear();
    }
//    sortbysize(featuresmappingL1);
//    sortbysize(featuresmappingL2);
//    sortbysize(featuresmappingL3);
    //setting coordinate system for converting to x,y
    bottomleftx = minlon * cos(latAvgGlobal * DEG_TO_RAD) * DEG_TO_RAD;
    bottomlefty = minlat * DEG_TO_RAD;
    toprightx = maxlon * cos(latAvgGlobal * DEG_TO_RAD) * DEG_TO_RAD;
    toprighty = maxlat * DEG_TO_RAD;
    load_successful = true;
    return load_successful;
    
}

void close_map() {
    streetToSegmentsGlobal.clear();
    interNamesGlobal.clear();
    interPositGlobal.clear();
    interStreetSegsGlobal.clear();
    streetSegInfoGlobal.clear();
    streetNamesGlobal.clear();
    streetSegCurvesGlobal.clear();
    POICoordinatesGlobal.clear();
    streetIDToIntersectsGlobal.clear();
    closeStreetDatabase();
    streetSegToTTimeGlobal.clear();
    streetNameToAllInterGlobal.clear();
    streetNameToIDGlobal.clear();
    rtreePOIGlobal.clear();
    rtreeInterGlobal.clear();
    streetSegtoLengthGlobal.clear();
    
    streetSegmapping.clear();
    featuresmapping.clear();
    streetsegSpeeds.clear();
    streetSegmappingL3.clear();
    streetSegmappingL2.clear();
    streetSegmappingL1.clear();
    streetSegMappingNamesL3.clear();
    streetSegMappingNamesL2.clear();
    streetSegMappingNamesL1.clear();
    featuresmappingML1.clear();
    featuresmappingML2.clear();
    featuresmappingML3.clear();
    featuresmappingL1.clear();
    featuresmappingL2.clear();
    featuresmappingL3.clear();
    featureType.clear();
    featureTypeName.clear();
    featureTypeL1.clear();
    featureTypeL2.clear(); 
    featureTypeL3.clear();
    featureTypeNameL1.clear();
    featureTypeNameL2.clear(); 
    featureTypeNameL3.clear();
    toFromL1.clear();
    toFromL2.clear();
    toFromL3.clear();
    POIPrinting.clear();
    interNametoInterGlobal.clear();
    interNamesGlobalForA.clear();
    
    
}

//Returns street id(s) for the given street name
//If no street with this name exists, returns a 0-length vector.
vector<unsigned> find_street_ids_from_name(string street_name) {
    unordered_map<string, vector<unsigned>>::const_iterator listOfStreetIDs = streetNameToIDGlobal.find(street_name);
    vector <unsigned> temp;
    if (listOfStreetIDs == streetNameToIDGlobal.end())
        return temp;
    else
        return listOfStreetIDs->second;
}

//Returns the street segments for the given intersection 
std::vector<unsigned> find_intersection_street_segments(unsigned intersection_id) {
    return interStreetSegsGlobal[intersection_id];
}

//Returns the street names at the given intersection (includes duplicate street 
//names in returned vector)
std::vector<std::string> find_intersection_street_names(unsigned intersection_id) {
    std::vector<string> nameList;
    std::vector<unsigned> segListID = interStreetSegsGlobal[intersection_id];
    unsigned count = getIntersectionStreetSegmentCount(intersection_id);

    for (unsigned i = 0; i < count; i++) {
        nameList.push_back(streetNamesGlobal[streetSegInfoGlobal[segListID[i]].streetID]);
    }
    return nameList;
}

//Returns true if you can get from intersection1 to intersection2 using a single 
//street segment (hint: check for 1-way streets too)
//corner case: an intersection is considered to be connected to itself
bool are_directly_connected(unsigned intersection_id1, unsigned intersection_id2) {
    if (intersection_id1 == intersection_id2) {
        return true;
    } else {
        vector<unsigned> intSeg1, intSeg2;
        intSeg1 = interStreetSegsGlobal[intersection_id1];
        intSeg2 = interStreetSegsGlobal[intersection_id2];
        for (unsigned i = 0; i < intSeg1.size(); i++) {
            for (unsigned j = 0; j < intSeg2.size(); j++) {
                if (intSeg1[i] == intSeg2[j]) {
                    StreetSegmentInfo sameSeg;
                    bool isOneWay;
                    IntersectionIndex fromer, toer;
                    sameSeg = streetSegInfoGlobal[intSeg1[i]];
                    isOneWay = sameSeg.oneWay;
                    fromer = sameSeg.from;
                    toer = sameSeg.to;
                    if (isOneWay == false) {
                        return true;
                    } else {
                        if (fromer == intersection_id1 && toer == intersection_id2) {
                            return true;
                        }
                        return false;
                    }
                }
            }
        }
    }
    return false;
}

//Returns all intersections reachable by traveling down one street segment 
//from given intersection (hint: you can't travel the wrong way on a 1-way street)
//the returned vector should NOT contain duplicate intersections
std::vector<unsigned> find_adjacent_intersections(unsigned intersection_id) {
    vector <unsigned> attachedStreetSegs;
    set <unsigned> adjIntersects;
    attachedStreetSegs = interStreetSegsGlobal[intersection_id];
    
    
    for (unsigned i = 0; i < attachedStreetSegs.size(); i++) {
        StreetSegmentInfo segInfo = streetSegInfoGlobal[attachedStreetSegs[i]];
        if (segInfo.oneWay == false) {
            if (segInfo.to != intersection_id) {
                adjIntersects.insert(segInfo.to);
            } else if (segInfo.from != intersection_id) {
                adjIntersects.insert(segInfo.from);
            }
        } else if (segInfo.from == intersection_id) {
            adjIntersects.insert(segInfo.to);
        }
    }
    vector <unsigned> adjIntersections(adjIntersects.begin(), adjIntersects.end());
    return adjIntersections;
}

//Returns all street segments for the given street
std::vector<unsigned> find_street_street_segments(unsigned street_id) {
    return streetToSegmentsGlobal[street_id];
}

// Returns all intersections along the a given street
std::vector<unsigned> find_all_street_intersections(unsigned street_id) {
    set <unsigned> listOfStreetInter;
    listOfStreetInter = streetIDToIntersectsGlobal[street_id];
    vector <unsigned> v(listOfStreetInter.begin(), listOfStreetInter.end());

    return v;
}

//Return all intersection ids for two intersecting streets
//This function will typically return one intersection id.
//However street names are not guarenteed to be unique, so more than 1 intersection id
//may exist
vector<unsigned> find_intersection_ids_from_street_names(string street_name1, string street_name2) {
    unordered_map <string, vector<unsigned>>::iterator it1 = streetNameToAllInterGlobal.find(street_name1);
    unordered_map <string, vector<unsigned>>::iterator it2 = streetNameToAllInterGlobal.find(street_name2);

    vector<unsigned> intersections;
    vector<unsigned> allIntersection = it1->second;
    allIntersection.insert(allIntersection.end(), it2->second.begin(), it2->second.end());

    sort(allIntersection.begin(), allIntersection.end());
    for (unsigned i = 0; i < allIntersection.size() - 1; i++) {
        if (allIntersection[i] == allIntersection[i + 1]) {
            intersections.push_back(allIntersection[i]);
            i++;
        }
    }

    return intersections;
}

//Returns the distance between two coordinates in meters
double find_distance_between_two_points(LatLon point1, LatLon point2) {
    double x1, y1, x2, y2, latAvg, dist;
    latAvg = (point1.lat() + point2.lat()) / 2;
    x1 = point1.lon() * cos(latAvg * DEG_TO_RAD) * DEG_TO_RAD;
    y1 = point1.lat() * DEG_TO_RAD;
    x2 = point2.lon() * cos(latAvg * DEG_TO_RAD) * DEG_TO_RAD;
    y2 = point2.lat() * DEG_TO_RAD;

    dist = sqrt((y2 - y1)*(y2 - y1)+(x2 - x1)*(x2 - x1)) * EARTH_RADIUS_IN_METERS ;

    return dist;

}

//Returns the length of the given street segment in meters
double find_street_segment_length(unsigned street_segment_id) {
    return streetSegtoLengthGlobal[street_segment_id];
}

//Returns the length of the specified street in meters
double find_street_length(unsigned street_id) {
    vector <unsigned> streetsegs;
    double length = 0;

    streetsegs = streetToSegmentsGlobal[street_id];
    for (unsigned i = 0; i < streetsegs.size(); i++) {
        length += find_street_segment_length(streetsegs[i]);
    }
    
    return length;
}

//Returns the travel time to drive a street segment in seconds 
//(time = distance/speed_limit)
double find_street_segment_travel_time(unsigned street_segment_id) {
    return streetSegToTTimeGlobal[street_segment_id];
}

//Returns the nearest point of interest to the given position
unsigned find_closest_point_of_interest(LatLon my_position) {
    unsigned current_Closest = 0;
    double distance = EARTH_RADIUS_IN_METERS ;
    double omega;

    vector <LatLonPairCart> closestPOI;
    point P2(my_position.lon(), my_position.lat());
    rtreePOIGlobal.query(bgi::nearest(P2, 100), back_inserter(closestPOI));

    for (unsigned i = 0; i < closestPOI.size(); i++) {
        omega = findAnother(my_position, POICoordinatesGlobal[closestPOI[i].second]);
        if (distance > omega) {
            current_Closest = closestPOI[i].second;
            distance = omega;
        }
    }
    
    return current_Closest;
}

//Returns the the nearest intersection to the given position
unsigned find_closest_intersection(LatLon my_position) {
    unsigned current_Closest = 0;
    double distance = EARTH_RADIUS_IN_METERS ;
    double omega;

    vector <LatLonPairCart> closestInters;
    point P2(my_position.lon(), my_position.lat());
    rtreeInterGlobal.query(bgi::nearest(P2, 150), back_inserter(closestInters));

    for (unsigned i = 0; i < closestInters.size(); i++) {
        omega = findAnother(my_position, interPositGlobal[closestInters[i].second]);
        if (distance > omega) {
            current_Closest = closestInters[i].second;
            distance = omega;
        }
    }
    
    return current_Closest;
}

// returns distance between two points without square root
double findAnother(LatLon point1, LatLon point2) {
    double x1, y1, x2, y2, latAvg, dist;
    latAvg = (point1.lat() + point2.lat()) / 2;
    x1 = point1.lon() * cos(latAvg * DEG_TO_RAD) * DEG_TO_RAD;
    y1 = point1.lat() * DEG_TO_RAD;
    x2 = point2.lon() * cos(latAvg * DEG_TO_RAD) * DEG_TO_RAD;
    y2 = point2.lat() * DEG_TO_RAD;

    dist = ((y2 - y1)*(y2 - y1)+(x2 - x1)*(x2 - x1)) * EARTH_RADIUS_IN_METERS ;
    
    return dist;
}



t_point latlontoxy(LatLon point1) {
    double x, y;
    x = point1.lon() * cos(latAvgGlobal * DEG_TO_RAD) * DEG_TO_RAD;
    y = point1.lat() * DEG_TO_RAD;
    return t_point(x,y);
}



void sortbysize(vector <vector <t_point>> &tobeSorted){
    
    unsigned int sizee = tobeSorted.size()-1;
    bool swapped;
    vector <t_point> temp;
    
    for (unsigned int i = 0; i < sizee; i++){
        swapped = false;
        for (unsigned int j = 0; j < sizee-i-1; j++){
            if (areaOfFeature(tobeSorted[j]) < areaOfFeature(tobeSorted[j+1])){
                temp = tobeSorted[j];
                tobeSorted[j] = tobeSorted[j+1];
                tobeSorted[j+1] = temp;
                swapped = true;
            }
        }
        if (swapped == false){
            break;
        }
    }
   
} 



pair <vector<unsigned>,vector<unsigned>> find_adjacent_intersections_with_duples(unsigned intersection_id) {
    pair <vector<unsigned>,vector<unsigned>> interIDtoSS;
    vector <unsigned> attachedStreetSegs;
    attachedStreetSegs = interStreetSegsGlobal[intersection_id];
    vector <unsigned> adjIntersections;
    vector <unsigned> adjSS;
    
    
    for (unsigned i = 0; i < attachedStreetSegs.size(); i++) {
        StreetSegmentInfo segInfo = streetSegInfoGlobal[attachedStreetSegs[i]];
        if (segInfo.oneWay == false) {
            if (segInfo.to != intersection_id) {
                adjIntersections.push_back(segInfo.to);
                adjSS.push_back(attachedStreetSegs[i]);
            } else if (segInfo.from != intersection_id) {
                adjIntersections.push_back(segInfo.from);
                adjSS.push_back(attachedStreetSegs[i]);
            }
        } else if (segInfo.from == intersection_id) {
            adjIntersections.push_back(segInfo.to);
            adjSS.push_back(attachedStreetSegs[i]);
        }
    }
    
    interIDtoSS.first = adjIntersections;
    interIDtoSS.second = adjSS;
    return interIDtoSS;
}