#include "m3helper.h"

using namespace std;

/**
 * takes in a path and turn penalty and returns the time it takes to travel
 */
double compute_path_travel_time(const vector<unsigned>& path, const double turn_penalty){
    double travTime = 0;
    unsigned turnPenCount = 0;
    if (path.size() == 0){
        return 0;
    }
    for (unsigned i = 0; i < (path.size()-1); i++){
        travTime += streetSegToTTimeGlobal[path[i]];
        if (streetSegInfoGlobal[path[i]].streetID != streetSegInfoGlobal[path[i+1]].streetID){
            turnPenCount += 1;
        }
    }
    travTime += streetSegToTTimeGlobal[path[path.size()-1]];
    travTime += turn_penalty*turnPenCount;

    return travTime;
    
}

/**
 * returns the fastest path between two intersections using A* algorithm
 */
vector<unsigned> find_path_between_intersections(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty){
    priority_queue< timeIntPair, vector <timeIntPair> , greater<timeIntPair> > pq;
    pair <vector<unsigned>,vector<unsigned>> interIDtoSSS;
    vector <unsigned> finalPath;
    vector <double> timeTo (noInterGlobal, INF);
    vector <vector<unsigned>> allPaths (noInterGlobal);
    pq.push(make_pair(0,intersect_id_start));
    vector <bool> visitedBool (noInterGlobal, false);   
    vector <unsigned> prevNode (noInterGlobal);
    vector <unsigned> prevSeg (noInterGlobal);
    
    
    timeTo[intersect_id_start]=0;
    
    while(!pq.empty()){
        double weightNew, weighter;
        unsigned currentNode=pq.top().second;
        double timeToCurrN = timeTo[currentNode];
        interIDtoSSS = find_adjacent_intersections_with_duples(currentNode);
        vector<unsigned> adjInter = interIDtoSSS.first;
        vector<unsigned> adjStreetSegs = interIDtoSSS.second;
        pq.pop();
        
        
        if(currentNode == intersect_id_end){
            //return allPaths[currentNode];
            return traceback (prevNode, prevSeg, intersect_id_start, intersect_id_end);
        }
       
        visitedBool[currentNode] = true;
        
        for(unsigned i=0; i<adjInter.size(); i++){
            unsigned adjNode= adjInter[i];
            //tempPath.push_back(adjStreetSegs[i]);
            if (currentNode == intersect_id_start){
                weighter = streetSegToTTimeGlobal[adjStreetSegs[i]];
            }
            else{
                weighter = timeToCurrN + perpscore(prevSeg[currentNode], adjStreetSegs[i], turn_penalty); //prevSeg[adjNode]
            }

            if(timeTo[adjNode] > weighter && visitedBool[adjNode]==false){
                weightNew = weighter + sqrt(heuristic(intersect_id_end, adjNode))*70;
                timeTo[adjNode]= weighter;
                pq.push(make_pair(weightNew,adjNode));
                //vector<unsigned> tempPath = allPaths[currentNode];
                //tempPath.push_back(adjStreetSegs[i]);
                //allPaths[adjNode] = tempPath;
                prevNode[adjNode] = currentNode;
                prevSeg[adjNode] = adjStreetSegs[i];

                //cout << weightNew << " without heur " << weighter << endl;

            }
        }
    }
   
    vector <unsigned> empty;
    return empty;
    
}

/**
 * returns the fastest path between an intersection and a desired POI
 */
vector<unsigned> find_path_to_point_of_interest(const unsigned intersect_id_start, const std::string point_of_interest_name, const double turn_penalty){
   
    bgi::rtree<LatLonPairCart, bgi::quadratic < 16 >> rtreePOIsingle;
    int count=0;
    
    for(unsigned i=0; i<noPOIGlobal; i++){
        
        if(POINamesGlobal[i]==point_of_interest_name){
            point P1(POICoordinatesGlobal[i].lon(), POICoordinatesGlobal[i].lat());
            rtreePOIsingle.insert(make_pair(P1, i));
            count++;
        }
    }
    vector<unsigned> pathFinal;
    
    if(count==0){
        return pathFinal;
    }
    int x;
    int threshold=8;
    
    if(count<threshold){
        x=count;
    } else x=threshold;
        
    
    vector <LatLonPairCart> closestPOI;
    
    point P2(getIntersectionPosition(intersect_id_start).lon(), getIntersectionPosition(intersect_id_start).lat());
    rtreePOIsingle.query(bgi::nearest(P2, x), back_inserter(closestPOI));

    
    double fastestTime=INF;
    
    
    for(unsigned i=0; i<closestPOI.size(); i++){
        
        vector<unsigned> path;
        unsigned closestInter=find_closest_intersection(POICoordinatesGlobal[closestPOI[i].second]);
       
        if(closestInter==intersect_id_start){
            vector<unsigned> pathFinalZero;
            return pathFinalZero;
        }
        
        path=find_path_between_intersections(intersect_id_start, closestInter, turn_penalty);
        
        //if there is no possible path, don't try to find the time and continue
        if(path.size()==0) continue;
        
        double currentTime= compute_path_travel_time(path, turn_penalty);
        if(currentTime<fastestTime){
            fastestTime=currentTime;
            pathFinal= path;
            closestIntersectionPOI=closestInter;
            
        }
        
    }
    
    rtreePOIsingle.clear();
    return pathFinal;
}

/**
 * prints out the travel directions to the destination
 */
void print_travel_directions(vector<unsigned> path, unsigned intersect_id_start, unsigned intersect_id_end){
    
    vector<unsigned> interPath= walkthruPath(path,intersect_id_start, intersect_id_end);
    int counter=1;
    double distance=0;
    double time=compute_path_travel_time(path, 15);
    time=time/60;
    cout << "Travel Directions to Destination: " << endl;
    cout << "Fastest Route takes " << time << " minutes." << endl;
    
    
    for(unsigned i=0; i<path.size()-1; i++){
        
        unsigned currentSeg=path[i];
        unsigned currentInter=interPath[i];
        unsigned nextInter=interPath[i+1];
        
        //continue if on same street as previous
        
        if(streetSegInfoGlobal[currentSeg].streetID==streetSegInfoGlobal[path[i+1]].streetID){
            distance+=find_street_segment_length(currentSeg);
            continue;
        }
        
        distance+=find_street_segment_length(currentSeg);
        
        t_point currentNode= latlontoxy(getIntersectionPosition(currentInter));
        t_point nextNode= latlontoxy(getIntersectionPosition(nextInter));
        
        double angle= atan((nextNode.y-currentNode.y)/(nextNode.x-currentNode.x));
        angle=angle/DEG_TO_RAD;
       
         if(nextNode.x<currentNode.x&&nextNode.y>currentNode.y){
            angle+=180;
        } else if(currentNode.y>nextNode.y&&currentNode.x>nextNode.x){
            angle+=180;
        }

        string direction;
        
        if(angle<22.5||angle>=337.5){
            direction = "East";
        }else if(angle<67.5&&angle>=22.5){
            direction = "North East";
        }else if(angle<112.5&&angle>=67.5){
            direction = "North";
        }else if(angle<157.5&&angle>=112.5){
            direction = "North West";
        }else if(angle<202.5&&angle>=157.5){
            direction = "West";
        }else if(angle<247.5&&angle>=202.5){
            direction = "South West";
        }else if(angle<292.5&&angle>=247.5){
            direction = "South";
        }else if(angle<337.5&&angle>=292.5){
            direction = "South East";
        }
        
        string currentStreet= streetNamesGlobal[streetSegInfoGlobal[currentSeg].streetID];
        
        cout << counter <<") Go " << direction << " on " << currentStreet <<" for "<< distance << "m"<< endl;
        counter++;
        distance=0;

    }
    
        unsigned currentSeg=path[path.size()-1];
        unsigned currentInter=interPath[path.size()-1];
        unsigned nextInter=interPath[path.size()];
        
        
        t_point currentNode= latlontoxy(getIntersectionPosition(currentInter));
        t_point nextNode= latlontoxy(getIntersectionPosition(nextInter));
        
        double angle= atan((nextNode.y-currentNode.y)/(nextNode.x-currentNode.x));
        angle=angle/DEG_TO_RAD;
       
         if(nextNode.x<currentNode.x&&nextNode.y>currentNode.y){
            angle+=180;
        } else if(currentNode.y>nextNode.y&&currentNode.x>nextNode.x){
            angle+=180;
        }

        string direction;
        
        if(angle<22.5||angle>=337.5){
            direction = "East";
        }else if(angle<67.5&&angle>=22.5){
            direction = "North East";
        }else if(angle<112.5&&angle>=67.5){
            direction = "North";
        }else if(angle<157.5&&angle>=112.5){
            direction = "North West";
        }else if(angle<202.5&&angle>=157.5){
            direction = "West";
        }else if(angle<247.5&&angle>=202.5){
            direction = "South West";
        }else if(angle<292.5&&angle>=247.5){
            direction = "South";
        }else if(angle<337.5&&angle>=292.5){
            direction = "South East";
        }
        
        distance+=find_street_segment_length(path[path.size()-1]);
        
        cout << counter <<") Go " << direction << " on " << streetNamesGlobal[streetSegInfoGlobal[path[path.size()-1]].streetID]<<" for "<< distance << "m" << endl;
    
    
    

}

/**
 * returns the intersections that the path goes through
 */
vector<unsigned> walkthruPath (vector<unsigned> Path, unsigned intersect_id_start, unsigned intersect_id_end){
    vector <unsigned> intersectNodes;
    intersectNodes.push_back(intersect_id_start);
    
    for (unsigned i = 0; i < Path.size()-1; i++){
        vector<unsigned> nodes;
        StreetSegmentInfo segInfo1 = streetSegInfoGlobal[Path[i]];
        StreetSegmentInfo segInfo2 = streetSegInfoGlobal[Path[i+1]];
        nodes.push_back(segInfo1.to);
        nodes.push_back(segInfo1.from);
        nodes.push_back(segInfo2.to);
        nodes.push_back(segInfo2.from);
        bool found=false;
        for (unsigned j=0; j<nodes.size(); j++){
            
            for (unsigned k=j+1; k<nodes.size(); k++){
                if(nodes[j]==nodes[k]){
                    intersectNodes.push_back(nodes[j]);
                    found=true;
                }
            }
            if(found==true){
                    break;
                }
            
        }
        
    }
    intersectNodes.push_back(intersect_id_end);
    return intersectNodes;
}

//find the distance
double heuristic (unsigned currNode, unsigned adjNode){
    LatLon currNodeLoc = interPositGlobal[currNode];
    LatLon adjNodeLoc = interPositGlobal[adjNode];
    return findAnother(currNodeLoc, adjNodeLoc);
    
}

double perpscore (unsigned prevSeg, unsigned thisSeg, const double turn_penalty){
    double perpScore = streetSegToTTimeGlobal[thisSeg];
    if (streetSegInfoGlobal[prevSeg].streetID != streetSegInfoGlobal[thisSeg].streetID){
        perpScore += turn_penalty;
    }
    
    return perpScore;
}

vector <unsigned> traceback (vector <unsigned> prevNodes, vector <unsigned> prevSegs, unsigned bgin, unsigned end){
    vector <unsigned> finalizedPath;
    unsigned currentNode = end;
    
    while (currentNode != bgin){
        finalizedPath.push_back(prevSegs[currentNode]);
        currentNode = prevNodes[currentNode];
    }
    reverse(finalizedPath.begin(), finalizedPath.end());
    return finalizedPath;
}



std::vector<unsigned> dijkstra(const unsigned intersect_id_start, const unsigned intersect_id_end, const double turn_penalty){
    priority_queue< timeIntPair, vector <timeIntPair> , greater<timeIntPair> > pq;
    pair <vector<unsigned>,vector<unsigned>> interIDtoSSS;
    vector <unsigned> finalPath;
    vector <double> timeTo (noInterGlobal, INF);
    vector <vector<unsigned>> allPaths (noInterGlobal);
    pq.push(make_pair(0,intersect_id_start));
    vector <bool> visitedBool (noInterGlobal, false);   
    vector <unsigned> prevNode (noInterGlobal);
    vector <unsigned> prevSeg (noInterGlobal);
    
    
    timeTo[intersect_id_start]=0;
    
    while(!pq.empty()){
        double weightNew, weighter;
        unsigned currentNode=pq.top().second;
        double timeToCurrN = timeTo[currentNode];
        interIDtoSSS = find_adjacent_intersections_with_duples(currentNode);
        vector<unsigned> adjInter = interIDtoSSS.first;
        vector<unsigned> adjStreetSegs = interIDtoSSS.second;
        pq.pop();
        
        
        if(currentNode == intersect_id_end){
            //return allPaths[currentNode];
            return traceback (prevNode, prevSeg, intersect_id_start, intersect_id_end);
        }
       
        visitedBool[currentNode] = true;
        
        for(unsigned i=0; i<adjInter.size(); i++){
            unsigned adjNode= adjInter[i];
            //tempPath.push_back(adjStreetSegs[i]);
            if (currentNode == intersect_id_start){
                weighter = streetSegToTTimeGlobal[adjStreetSegs[i]];
            }
            else{
                weighter = timeToCurrN + perpscore(prevSeg[currentNode], adjStreetSegs[i], turn_penalty); //prevSeg[adjNode]
            }

            if(timeTo[adjNode] > weighter && visitedBool[adjNode]==false){
                weightNew = weighter;
                timeTo[adjNode]= weighter;
                pq.push(make_pair(weightNew,adjNode));
                //vector<unsigned> tempPath = allPaths[currentNode];
                //tempPath.push_back(adjStreetSegs[i]);
                //allPaths[adjNode] = tempPath;
                prevNode[adjNode] = currentNode;
                prevSeg[adjNode] = adjStreetSegs[i];

                //cout << weightNew << " without heur " << weighter << endl;

            }
        }
    }
   
    vector <unsigned> empty;
    return empty;
}