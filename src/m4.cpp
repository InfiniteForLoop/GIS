/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

#include "m4helper.h"

using namespace std;



std::vector<unsigned> traveling_courier(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots, const float turn_penalty){
    vector <unsigned> finalPath = greedyAlgo(deliveries, depots, turn_penalty);
    return finalPath;
    
}


unsigned calculateStartBestDepot (const std::vector<unsigned>& depots, const std::set<unsigned>& pickUps){
    double dist = INF;
    unsigned bestDepot, whichDep;
    for (unsigned i = 0; i < depots.size(); i++){
        double tempDist = 0;
        double temporalDist = INF;
        unsigned tempDepot;
        set<unsigned>::iterator it = pickUps.begin();
        while (it != pickUps.end()){    
            tempDepot = depots[i];
            tempDist = findAnother(interPositGlobal[tempDepot], interPositGlobal[*it]);
            if (tempDist < temporalDist){
                temporalDist = tempDist;
            }
            it++;
        }
        if (temporalDist < dist){
            dist = tempDist;
            bestDepot = tempDepot;
            whichDep = i;
        }
    }

    if (interStreetSegsGlobal[bestDepot].size() == 0){
        return depots[whichDep+1];
    }
    return bestDepot;
    
}

unsigned calculateStartEndDepot (const std::vector<unsigned>& depots, unsigned currInter){
    bgi::rtree<LatLonPairCart, bgi::quadratic < 16 >> rtreeDepots;
    bgi::rtree<LatLonPairCart, bgi::quadratic < 16 >> rtreeDepotsforDep;
    vector <LatLonPairCart> bestEndDepots;
    for (unsigned i = 0; i < depots.size(); i++){
        point P1(interPositGlobal[depots[i]].lon(), interPositGlobal[depots[i]].lat());
        rtreeDepotsforDep.insert(make_pair(P1, depots[i]));  
    }
    
    point P2(interPositGlobal[currInter].lon(), interPositGlobal[currInter].lat());
    rtreeDepotsforDep.query(bgi::nearest(P2, 2), back_inserter(bestEndDepots));
    
    unsigned firstuu = bestEndDepots[0].second;
    unsigned suu = bestEndDepots[1].second;
    
    if (interStreetSegsGlobal[firstuu].size() == 0){
        rtreeDepotsforDep.clear();
        return suu;
    }
    rtreeDepotsforDep.clear();
    return firstuu;
}




std::vector<unsigned> greedyAlgo(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots, const float turn_penalty){
    bgi::rtree<LatLonPairCart, bgi::quadratic < 16 >> rtreeDepots;
    bgi::rtree<LatLonPairCart, bgi::quadratic < 16 >> rtreeDepotsforDep;
    vector <unsigned> interSectionOrder;
    set <unsigned> pickUps, dropOffs, toBeVisited;
    vector <vector<unsigned>> sortaPath;
    vector <unsigned> finalPath;
    unordered_map <unsigned, vector <unsigned>> pickUpToDropOffs, dropOfftoPickUps; 

    //loads the data into useful containers
    for (unsigned i = 0; i < deliveries.size(); i++){
        toBeVisited.insert(deliveries[i].pickUp);
        pickUps.insert(deliveries[i].pickUp);
        if (pickUpToDropOffs[deliveries[i].pickUp].size() == 0){
            vector <unsigned> temper;
            temper.push_back(deliveries[i].dropOff);
            pickUpToDropOffs[deliveries[i].pickUp] = temper;
            //vector <unsigned> pop = pickUpToDropOffs[deliveries[i].pickUp];
        }
        else{
            pickUpToDropOffs[deliveries[i].pickUp].push_back(deliveries[i].dropOff);
        }
        if (dropOfftoPickUps[deliveries[i].dropOff].size() == 0){
            vector <unsigned> temper;
            temper.push_back(deliveries[i].pickUp);
            dropOfftoPickUps[deliveries[i].dropOff] = temper;
            //vector <unsigned> pop = dropOfftoPickUps[deliveries[i].dropOff];
        }
        else{
            dropOfftoPickUps[deliveries[i].dropOff].push_back(deliveries[i].pickUp);
        }
    }
    
    //get the best start depot
    unsigned currInt = calculateStartBestDepot (depots, toBeVisited);
    interSectionOrder.push_back(currInt);
    

    while (!toBeVisited.empty()){
        set<unsigned>::iterator it = toBeVisited.begin();
        vector<LatLonPairCart> closestDep;
        if (it == toBeVisited.end()){
            unsigned nInt = *it;
            point P1(interPositGlobal[nInt].lon(), interPositGlobal[nInt].lat());
            rtreeDepots.insert(make_pair(P1, nInt));
        }
        while (it != toBeVisited.end()){
            unsigned nInt = *it;
            point P1(interPositGlobal[nInt].lon(), interPositGlobal[nInt].lat());
            rtreeDepots.insert(make_pair(P1, nInt));
            it++;
        }
        point P2(interPositGlobal[currInt].lon(), interPositGlobal[currInt].lat());
        rtreeDepots.query(bgi::nearest(P2, 1), back_inserter(closestDep));
        unsigned nextInt = closestDep[0].second;

        vector <unsigned> tempPath = find_path_between_intersections(currInt, nextInt, turn_penalty);
        sortaPath.push_back(tempPath);
        
//        find_another_path_between_intersections(currInt, nextInt, turn_penalty, finalPath);
//        sortaPath.push_back(tempPath);
        
        
        currInt = nextInt;
        interSectionOrder.push_back(currInt);
        toBeVisited.erase(nextInt);
        if (pickUpToDropOffs[currInt].size() != 0){
            vector <unsigned> dropOffsatPicks = pickUpToDropOffs[currInt];
            for (unsigned qq = 0; qq < dropOffsatPicks.size(); qq++){
                toBeVisited.insert(dropOffsatPicks[qq]);
            }
        }
        rtreeDepots.clear();
    }
 
    for (unsigned i = 0; i < sortaPath.size(); i++){
        unsigned Kappa = sortaPath[i].size();
        vector <unsigned> McDonalds = sortaPath[i];
        for (unsigned j = 0; j < Kappa; j++){
            finalPath.push_back(McDonalds[j]);
        }
    }
    
    unsigned endDep;
    if (depots.size() == 1){
        endDep = depots[0];
    }
    else{
        endDep = calculateStartEndDepot (depots, currInt);
    }
    interSectionOrder.push_back(endDep);
    vector <unsigned> tempPath = find_path_between_intersections(currInt, endDep, turn_penalty);
    if (tempPath.size() == 0){
        find_path_between_intersections(currInt, endDep, turn_penalty);
    }
    for (unsigned i = 0; i < tempPath.size(); i++){
        finalPath.push_back(tempPath[i]);
    }
    
    //TWO OPT HERE LUL
    
    vector <unsigned> Kappa = twoOpt (interSectionOrder, dropOfftoPickUps);

    
    finalPath = createPath(Kappa, turn_penalty);
    
    if (pickUps.size() != 0){
        pickUps.clear();
    }
    if (dropOffs.size() != 0){
        dropOffs.clear();
    }
    if (toBeVisited.size() != 0){
        toBeVisited.clear();
    }
    //cout << finalPath.size() << endl;
    return finalPath;
}

pair<unsigned, vector<unsigned>> betterPlace(const std::vector<unsigned>& considerThese, const float turn_penalty, unsigned currInt){
    vector <unsigned> bestPath;
    unsigned nextInt;
    double tempTTime = INF;
    for (unsigned i = 0; i < considerThese.size(); i++){
        unsigned nerxtInt = considerThese[i];
        vector <unsigned> tempPath = find_path_between_intersections(currInt, nerxtInt, turn_penalty);
        double teaTime = compute_path_travel_time(tempPath, turn_penalty);
        if (teaTime < tempTTime){
            tempTTime = teaTime;
            nextInt = considerThese[i];
            bestPath = tempPath;
        }
        
    }
    return make_pair(nextInt, bestPath);
     
}


double distoPath(vector <unsigned> intersectionInOrder){
    double dist = 0;
    for (unsigned i = 0; i < (intersectionInOrder.size()-1); i++){
        dist += findAnother(interPositGlobal[intersectionInOrder[i]], interPositGlobal[intersectionInOrder[i+1]]);
    }
    return dist;
}


vector <unsigned> twoOpt (vector <unsigned> intersectionInOrder, unordered_map <unsigned, vector <unsigned>> dropOfftoPickUps){
    
    
    double oldPathTime = distoPath(intersectionInOrder);
    vector <unsigned> potCand = intersectionInOrder;
    unsigned imporvement = 0;
    while (imporvement < 15){
        vector<unsigned>::iterator itersUno = potCand.begin();
        itersUno = itersUno+1;
        while (itersUno != (potCand.end()-2)){
            vector<unsigned>::iterator itersDos = itersUno+1;
            while (itersDos != (potCand.end()-1)){
                //vector <unsigned> backUpGuy = potCand;
                reverse(itersUno,itersDos); //potCan has been reversed lol
                if (isLegal(potCand, itersUno, itersDos, dropOfftoPickUps)){
                    //cout << "BANGO" << endl;
                    double newPathTime = distoPath(potCand);
                    if (newPathTime < oldPathTime){
                        oldPathTime = newPathTime;
                    //cout << "BINGO" << endl;
                    }
                    else{
                        reverse(itersUno,itersDos);
                    }
                }
                else{
                    reverse(itersUno,itersDos);
                }
                itersDos++;
            }
            itersUno++;
        }
        imporvement++;
    }
    return potCand;
}


bool isLegal (const vector <unsigned> intersectionOrder, vector<unsigned>::iterator itersUno, vector<unsigned>::iterator itersDos, unordered_map <unsigned, vector <unsigned>> dropOfftoPickUps){
    vector <unsigned> isThisLegal(itersUno, itersDos);
    vector <unsigned> pickUpLocs;
    //step through this vector to see if legal path
    
    for (unsigned i = 0; i < isThisLegal.size(); i++){
        unsigned currInt = isThisLegal[i];
        if (find(pickUpLocs.begin(), pickUpLocs.end(), currInt) != pickUpLocs.end()){
            return false; //not legal
        }
        if (dropOfftoPickUps[isThisLegal[i]].size() != 0){
            vector <unsigned> b = dropOfftoPickUps[isThisLegal[i]];
            pickUpLocs.insert(pickUpLocs.end(), b.begin(), b.end());
        }
    }
    return true;
    
}


vector <unsigned> createPath(const vector <unsigned> intersectionOrder, const float turn_penalty){
    vector <vector<unsigned>> kindaPath;
    vector <unsigned> finalePath;
    for (unsigned i = 0; i < (intersectionOrder.size()-1); i++){
        vector <unsigned> pathers = find_path_between_intersections(intersectionOrder[i], intersectionOrder[i+1], turn_penalty);
        kindaPath.push_back(pathers);
    }
    
    
    for (unsigned i = 0; i < kindaPath.size(); i++){
        unsigned Kappa = kindaPath[i].size();
        vector <unsigned> McDonalds = kindaPath[i];
        for (unsigned j = 0; j < Kappa; j++){
            finalePath.push_back(McDonalds[j]);
        }
    }
    
    return finalePath;
    
}



std::pair<vector<unsigned>, vector<unsigned>> greedyAlgoAnother(const std::vector<DeliveryInfo>& deliveries, const std::vector<unsigned>& depots, const float turn_penalty){
    bgi::rtree<LatLonPairCart, bgi::quadratic < 16 >> rtreeDepots;
    bgi::rtree<LatLonPairCart, bgi::quadratic < 16 >> rtreeDepotsforDep;
    vector <unsigned> interSectionOrder;
    set <unsigned> pickUps, dropOffs, toBeVisited;
    vector <vector<unsigned>> sortaPath;
    vector <unsigned> finalPath;
    unordered_map <unsigned, vector <unsigned>> pickUpToDropOffs, dropOfftoPickUps; 

    //loads the data into useful containers
    for (unsigned i = 0; i < deliveries.size(); i++){
        toBeVisited.insert(deliveries[i].pickUp);
        pickUps.insert(deliveries[i].pickUp);
        if (pickUpToDropOffs[deliveries[i].pickUp].size() == 0){
            vector <unsigned> temper;
            temper.push_back(deliveries[i].dropOff);
            pickUpToDropOffs[deliveries[i].pickUp] = temper;
            //vector <unsigned> pop = pickUpToDropOffs[deliveries[i].pickUp];
        }
        else{
            pickUpToDropOffs[deliveries[i].pickUp].push_back(deliveries[i].dropOff);
        }
        if (dropOfftoPickUps[deliveries[i].dropOff].size() == 0){
            vector <unsigned> temper;
            temper.push_back(deliveries[i].pickUp);
            dropOfftoPickUps[deliveries[i].dropOff] = temper;
            //vector <unsigned> pop = dropOfftoPickUps[deliveries[i].dropOff];
        }
        else{
            dropOfftoPickUps[deliveries[i].dropOff].push_back(deliveries[i].pickUp);
        }
    }
    
    //get the best start depot
    unsigned currInt = calculateStartBestDepot (depots, toBeVisited);
    interSectionOrder.push_back(currInt);
    

    while (!toBeVisited.empty()){
        set<unsigned>::iterator it = toBeVisited.begin();
        vector<LatLonPairCart> closestDep;
        if (it == toBeVisited.end()){
            unsigned nInt = *it;
            point P1(interPositGlobal[nInt].lon(), interPositGlobal[nInt].lat());
            rtreeDepots.insert(make_pair(P1, nInt));
        }
        while (it != toBeVisited.end()){
            unsigned nInt = *it;
            point P1(interPositGlobal[nInt].lon(), interPositGlobal[nInt].lat());
            rtreeDepots.insert(make_pair(P1, nInt));
            it++;
        }
        point P2(interPositGlobal[currInt].lon(), interPositGlobal[currInt].lat());
        rtreeDepots.query(bgi::nearest(P2, 1), back_inserter(closestDep));
        unsigned nextInt = closestDep[0].second;

        vector <unsigned> tempPath = find_path_between_intersections(currInt, nextInt, turn_penalty);
        sortaPath.push_back(tempPath);
        
//        find_another_path_between_intersections(currInt, nextInt, turn_penalty, finalPath);
//        sortaPath.push_back(tempPath);
        
        
        currInt = nextInt;
        interSectionOrder.push_back(currInt);
        toBeVisited.erase(nextInt);
        if (pickUpToDropOffs[currInt].size() != 0){
            vector <unsigned> dropOffsatPicks = pickUpToDropOffs[currInt];
            for (unsigned qq = 0; qq < dropOffsatPicks.size(); qq++){
                toBeVisited.insert(dropOffsatPicks[qq]);
            }
        }
        rtreeDepots.clear();
    }
 
    for (unsigned i = 0; i < sortaPath.size(); i++){
        unsigned Kappa = sortaPath[i].size();
        vector <unsigned> McDonalds = sortaPath[i];
        for (unsigned j = 0; j < Kappa; j++){
            finalPath.push_back(McDonalds[j]);
        }
    }
    
    unsigned endDep;
    if (depots.size() == 1){
        endDep = depots[0];
    }
    else{
        endDep = calculateStartEndDepot (depots, currInt);
    }
    interSectionOrder.push_back(endDep);
    vector <unsigned> tempPath = find_path_between_intersections(currInt, endDep, turn_penalty);
    if (tempPath.size() == 0){
        find_path_between_intersections(currInt, endDep, turn_penalty);
    }
    for (unsigned i = 0; i < tempPath.size(); i++){
        finalPath.push_back(tempPath[i]);
    }
    
    //TWO OPT HERE LUL
    
    vector <unsigned> Kappa = twoOpt (interSectionOrder, dropOfftoPickUps);

    
    finalPath = createPath(Kappa, turn_penalty);
    
    if (pickUps.size() != 0){
        pickUps.clear();
    }
    if (dropOffs.size() != 0){
        dropOffs.clear();
    }
    if (toBeVisited.size() != 0){
        toBeVisited.clear();
    }
    //cout << finalPath.size() << endl;
    return make_pair(finalPath,Kappa);
}