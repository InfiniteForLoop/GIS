#ifndef M1HELPER_H
#define M1HELPER_H

#include "m1.h"
#include "m2helper.h"
#include "graphics.h"
#include "StreetsDatabaseAPI.h"
#include <vector>
#include <string>
#include <unordered_map>
#include <map>
#include <unordered_set>
#include <math.h>
#include <cmath>
#include <set>
#include <algorithm>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/register/point.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/core/cs.hpp>
#include <limits>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;
using namespace std;

typedef bg::model::point <float, 2, bg::cs::spherical_equatorial<bg::degree>> point;
typedef pair<point, unsigned> LatLonPairCart; 

//intersection data
extern vector <string> interNamesGlobal;
extern vector <LatLon> interPositGlobal;
extern vector <vector <unsigned>> interStreetSegsGlobal;
extern vector <set <unsigned>> streetIDToIntersectsGlobal;
extern vector <vector <unsigned>> distanceFromCenterInterGlobal;
extern unordered_map <string,vector<unsigned>> streetNameToAllInterGlobal; 
extern vector <const char*> interNamesGlobalForA;
extern unordered_map <string,vector<unsigned>> interNametoInterGlobal; 

//street segment data
extern vector <StreetSegmentInfo> streetSegInfoGlobal; 
extern vector <vector <LatLon>> streetSegCurvesGlobal;
extern vector <double> streetSegtoLengthGlobal;
extern vector <vector <unsigned>> streetToSegmentsGlobal;
extern vector <double> streetSegToTTimeGlobal;

//street data
extern vector <string> streetNamesGlobal;
extern unordered_map <string, vector<unsigned>> streetNameToIDGlobal;

// POI data
extern vector <LatLon> POICoordinatesGlobal;
extern vector <vector <unsigned>> distanceFromCenterPOIGlobal; //0th element within 10 km, 1st element within 20km etc
extern vector <string> POINamesGlobal;

// boost data
extern bgi::rtree<LatLonPairCart,bgi::quadratic<16>> rtreePOIGlobal, rtreeInterGlobal;
extern double findAnother (LatLon point1, LatLon point2);


// count data
extern unsigned noStreetsGlobal, noSegsGlobal, noInterGlobal, noPOIGlobal, noFeatGlobal;


//for m2
extern double maxlat, minlat, maxlon, minlon, latAvgGlobal;
extern double bottomleftx, bottomlefty, toprightx, toprighty;
extern t_point latlontoxy (LatLon point1);
extern void sortbysize(vector <vector <t_point>> &tobeSorted);
extern vector <vector <t_point>> streetSegmapping;
extern vector <vector <t_point>> featuresmapping;
extern vector <FeatureType> featureType;
extern vector <double> streetsegSpeeds;
extern vector <vector <t_point>> streetSegmappingL1, streetSegmappingL2, streetSegmappingL3;
extern vector <vector <t_point>> featuresmappingL1, featuresmappingL2, featuresmappingL3;
extern multimap <double, pair<vector<t_point>, unsigned>> featuresmappingML1, featuresmappingML2, featuresmappingML3;

extern vector <string> streetSegMappingNamesL1, streetSegMappingNamesL2, streetSegMappingNamesL3;

extern vector <FeatureType> featureTypeL1, featureTypeL2, featureTypeL3;
extern vector <string> featureTypeNameL1, featureTypeNameL2, featureTypeNameL3;
extern vector <string> featureTypeName;

extern vector <vector<t_point>> toFromL1, toFromL2, toFromL3; 
extern vector <t_point> POIPrinting;

//for m3
extern pair <vector<unsigned>,vector<unsigned>> find_adjacent_intersections_with_duples(unsigned intersection_id);


//L1  Beach Lake Island Greenspace
//L2 Golfcourse Park River Shoreline
//L3 Stream Buildings 
#endif /* M1HELPER_H */

