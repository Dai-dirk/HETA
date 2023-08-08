
#include "mapper/mapper_sa.h"


MapperSA::MapperSA(ADG* adg, int timeout, int maxIter, bool objOpt) : Mapper(adg){
    setTimeOut(timeout);
    setMaxIters(maxIter);
    setObjOpt(objOpt);
}

// MapperSA::MapperSA(ADG* adg, DFG* dfg) : Mapper(adg, dfg){}

MapperSA::MapperSA(ADG* adg, DFG* dfg, int timeout, int maxIter, bool objOpt) : Mapper(adg, dfg){
    setTimeOut(timeout);
    setMaxIters(maxIter);
    setObjOpt(objOpt);
}

MapperSA::~MapperSA(){}

// map the DFG to the ADG, mapper API
bool MapperSA::mapper(bool spatial, bool isHeteMapping){
    bool succeed;
    std::cout << "[HETA] End of initialization, begin to PnR!" <<std::endl;
    if(_objOpt){ // objective optimization
        succeed = pnrSyncOpt();
    }else{
        succeed = pnrSync(_maxIters, MAX_TEMP, false, spatial, isHeteMapping);
    }
    if(succeed){
        std::cout << "[HETA] PnR successfully with II = " << II << std::endl;
    }
    return succeed;
}


// PnR, Data Synchronization, and objective optimization
bool MapperSA::pnrSyncOpt(){
    int temp = MAX_TEMP; // temperature
    int maxItersMapSched = 3000; // pnrSync iteration number
    int maxItersNoImprv = 50;  // if not improved for maxItersNoImprv, end
    //int restartIters = 20;     // if not improved for restartIters, restart from the cached status
    int lastImprvIter = 0;
    int lastRestartIter = 0;
    int newObj;
    int oldObj = 0x7fffffff;
    int minObj = 0x7fffffff;
    bool succeed = false;
    bool isOpt = false;
    Mapping* bestMapping = new Mapping(getADG(), getDFG());
    Mapping* lastAcceptMapping = new Mapping(getADG(), getDFG());
    for(int iter = 0; iter < 15; iter++){
        if(runningTimeMS() > getTimeOut()){
            break;
        }
        if(!isOpt){
            maxItersMapSched = _maxIters;
        }else{
            maxItersMapSched = 50;
        }
        // PnR and Data Synchronization
        bool res = pnrSync(maxItersMapSched, temp, isOpt);
        if(!res){ // fail to map
            spdlog::debug("PnR and Data Synchronization failed!");
            continue;
        }
        std::cout << "[HETA] Start objective optimization! " << std::endl;
        succeed = true;
        isOpt = true;
        spdlog::info("PnR and Data Synchronization succeed, start optimization");
        // Objective function
        newObj = objFunc(_mapping);
        spdlog::debug("Object: {}", newObj);
        int difObj = newObj - oldObj;
        if(metropolis(difObj, temp)){ // accept new solution according to the Metropolis rule
            if(newObj < minObj){ // get better result
                minObj = newObj;
                *bestMapping = *_mapping; // cache better mapping status, ##### DEFAULT "=" IS OK #####
                lastImprvIter = iter;
                lastRestartIter = iter;
                temp = annealFunc(temp); //  annealling
                spdlog::warn("###### Better object: {} ######", newObj);
            }
            *lastAcceptMapping = *_mapping; // can keep trying based on current status
            oldObj = newObj;
        }else{
            *_mapping = *lastAcceptMapping; // restart from the cached status
        }
    }
    *_mapping = *bestMapping;
    delete bestMapping;
    delete lastAcceptMapping;
    if(succeed){
        spdlog::warn("######## Best object: {} ########", minObj);
        std::cout << "Best max latency: " << _mapping->maxLat() + 1 << std::endl; // time start from 0
    }
    return succeed;
}


// PnR and Data Synchronization
//@Yuan: isOpt : the mapping is successful before, just do optimize
bool MapperSA::pnrSync(int maxIters, float temp, bool isOpt, bool spatial, bool isHeteMapping){
    float initTemp = temp;
    ADG* adg = _mapping->getADG();
    DFG* dfg = _mapping->getDFG();
    Mapping* curMapping = new Mapping(adg, dfg);
    Mapping* lastAcceptMapping = new Mapping(adg, dfg);
    int numNodes = dfg->nodes().size();
    int maxItersNoImprv = 30; // if not improved for maxItersNoImprv, reset mapping status
    int maxItersToClear = II * 5; // if not improved for maxItersNoImprv, reset mapping status
    int maxItersToInsert = 20 + numNodes/5; // if not improved for maxItersNoImprv, insert pass node
    int extraHeteIter = isHeteMapping? 90 : 0;
    // int restartIters = 20;     // if not improved for restartIters, restart from the cached status
    int lastImprvIter = 0;
    int lastClearIter = 0;
    int curItersNosucced = 0; //@Yuan: if not succed (not means vio = 0), consider increasing II
    bool succeed = false;
    bool update = false;
    int newnumMapped;
    int oldnumMapped = 0;
    int maxnumMapped = 0;
    int bestVio = 0;
    int initII = II;
    int lastAcceptII= II;
    int iter = 0;
    int test = 0;
    while(true){
        if(runningTimeMS() > getTimeOut()){
            std::cout << "[HETA] stop! Time out!" <<std::endl;
            //_mapping->printAva();
            break;
        }
        iter += 1;
        // if(iter == 3) exit(0);
        if(iter > maxIters){
            std::cout << "[HETA] stop! Max iteration reach!" <<std::endl;
            //_mapping->printAva();
            break;
        }
        // PnR without latency scheduling of DFG nodes
        if((curItersNosucced > (350 - 35 * (II - 1) + extraHeteIter) && !isOpt) && !spatial){ // if fail for some time, increase II
            std::cout << "[HETA] Current II:  " << II <<" increases to: "<< II + 1 << std::endl;
            DFG* curDfg = curMapping->getDFG();
            delete curMapping;
            curMapping = new Mapping(adg, curDfg);
            curMapping->clearADGNodeUsage();
            II += 1;
            temp = initTemp;
            oldnumMapped = 0;
            curItersNosucced = 0;
            lastAcceptII = II;
            *lastAcceptMapping = *curMapping; // can keep trying based on current status
        }
        if(II > adg->maxII()){
            std::cout << "[HETA] Warning! Current II exceeds the maximum supported II, the final config data may be uncorrect!" << std::endl;
        }
        // std::cout << "onece!!!\n " << std::endl;
        int status = pnr(curMapping, temp);
        int newVio = curMapping->numVioEdge();
        if(curMapping->success() && newVio == 0){
            succeed = true;
            *_mapping = *curMapping; // keep better mapping status, ##### DEFAULT "=" IS OK #####
            std::cout << "PnR successful with test and iter: " << test << " , " << iter<< std::endl;
            latency = _mapping->maxLat();
            //_mapping->printAva();
            break;
        }
        curItersNosucced += 1;
        newnumMapped = curMapping->numNodeMapped() - 15*newVio; // current numbers of mapped nodes
        //newnumMapped = curMapping->numNodeMapped() - newVio; // current numbers of mapped nodes
        //std::cout << "_mapping->maxLat(): " << curMapping->maxLat()<< std::endl;
        int difVio = oldnumMapped - newnumMapped;
        //std::cout << "------------------difVio----------------- " <<difVio << std::endl;
        if(metropolis(difVio, temp)){ // accept new solution according to the Metropolis rule
            if(newnumMapped > maxnumMapped){ // get better result
                maxnumMapped = newnumMapped;
                *_mapping = *curMapping; // cache better mapping status, ##### DEFAULT "=" IS OK #####
                lastImprvIter = iter;
                temp = annealFunc(temp); //  annealling
                bestVio = newVio;
                update = true;
                spdlog::warn("#### Smaller violation: {} ####", maxnumMapped);
                //std::cout << "#### Smaller violation: {} ####" << maxnumMapped << std::endl;
            }
            //std::cout << "accept new solution!!!!!!!!!!!!!!" << std::endl;
            lastAcceptII = II;
            *lastAcceptMapping = *curMapping; // can keep trying based on current status
            oldnumMapped = newnumMapped;
        }else{
            II = lastAcceptII;
            *curMapping = *lastAcceptMapping;
            //exit(0);
            //std::cout << "reject new solution!!!!!!!!!!!!!!" << std::endl;
        }
        if(((II > 2*initII) && bestVio != 0 && spatial == 0) || (spatial == 1 && (iter - lastImprvIter) > maxItersToInsert && bestVio != 0)){
            if(isOpt){ // cannot modify DFG, stop iteration
                break;
            }
            std::cout << "[HETA] Insert the pass-through node, refresh the DFG! " << std::endl;
            DFG* newDfg = new DFG();
            int totalVio, maxVio;
            _mapping->insertPassDfgNodes(newDfg); // insert pass-through nodes into DFG
            spdlog::warn("Min total latency violation: {}", maxnumMapped);
            spdlog::warn("Current total latency violation: {}", totalVio);
            //spdlog::warn("Current max latency violation: {}", maxVio);
            spdlog::warn("Insert pass-through nodes into DFG");
            // newDfg->print();
            setDFG(newDfg, true);  //  update the _dfg and initialize
            //@Yuan: after modifying the DFG, we need to update the II
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            int ResMII = getResMII(getADG(), getDFG());
            std::cout << "[Modified Initial ResMII: " << ResMII << "]" << std::endl;
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            int RecMII = getRecMII(getDFG());
            std::cout << "[Modified Initial RecMII: " << RecMII << "]" << std::endl;
            std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
            II = ResMII > RecMII ? ResMII:RecMII;       // II = MAX(RecMII, ResMII)
            delete curMapping;
            curMapping = new Mapping(adg, newDfg);
            curMapping->clearADGNodeUsage();
            sortDfgNodeInPlaceOrder();
            *lastAcceptMapping = *curMapping;
            lastImprvIter = 0;
            lastAcceptII = II;
            iter = 0;
            temp = initTemp;
            bestVio = 0;
            oldnumMapped = 0;
            maxnumMapped = 0;
            curItersNosucced = 0;
        }
        // @Yuan: if not improved for long time, reset the mapping status
        if(iter - lastImprvIter > maxItersNoImprv){
        //if(curItersNosucced > maxItersNoImprv){
            // std::cout << "reset!~~~~~~~~~~~~~~~~~~" <<" iter: " << iter <<" lastImprvIter: "<<lastImprvIter<< std::endl;
            curMapping->reset();
            test = test + 1;
            lastImprvIter = iter;
            oldnumMapped = 0;
         }
         if(iter - lastClearIter > maxItersToClear){
            curMapping->clearADGNodeUsage();
            lastClearIter = iter;            
         }
    }
    delete curMapping;
    delete lastAcceptMapping;
    if(succeed){
        spdlog::warn("Max latency: {}", _mapping->maxLat());
        if(!isOpt){
            std::cout << "[HETA] Before object-opt max latency: " << _mapping->maxLat() + 1<< std::endl; // time start from 0
        }
    }
    _mapping->clearADGNodeUsage();
    return succeed;
}



// PnR with SA temperature(max = 100)
int MapperSA::pnr(Mapping* mapping, float temp){
    unmapSome(mapping, temp);
    return incrPnR(mapping);
}


// unmap some DFG nodes
void MapperSA::unmapSome(Mapping* mapping, float temp){
    mapping->setMaxLat(0);
    //int unmap_num = 0;
        //std::cout << "before unmap: " << mapping->numNodeMapped() << std::endl;
    for(auto& elem : mapping->getDFG()->nodes()){
        auto node = elem.second;
        if((randfloat()< temp/MAX_TEMP) && mapping->isMapped(node)){
            //unmap_num += 1;
            //std::cout <<"rand test: " << rand()%2 << "\n" << std::endl;
            if(rand()%2 == 1){
                mapping->unmapDfgNode(node);
    //
                //break;
            }
        }
    }
    //std::cout << "after unmap: " << mapping->numNodeMapped() << std::endl;
    //std::cout << "unmap num: " << unmap_num << std::endl;
}


// incremental PnR, try to map all the left DFG nodes based on current mapping status
int MapperSA::incrPnR(Mapping* mapping){
    auto dfg = mapping->getDFG();
    // // cache DFG node IDs
    // std::vector<int> dfgNodeIds;
    // for(auto node : dfg->topoNodes()){ // topological order
    //     dfgNodeIds.push_back(node->id());
    //     // std::cout << node->id() << ", ";
    // }
    // // std::cout << std::endl;
    // // sort DFG nodes according to their candidate numbers
    // // std::random_shuffle(dfgNodeIds.begin(), dfgNodeIds.end()); // randomly sort will cause long routing paths
    // std::stable_sort(dfgNodeIds.begin(), dfgNodeIds.end(), [&](int a, int b){
    //     return candidatesCnt[a] <  candidatesCnt[b];
    // });
    // for(auto id : dfgNodeIds){ // topological order
    //     std::cout << id << "(" << candidatesCnt[id] << ")" << ", ";
    // }
    // std::cout << std::endl;
    // start mapping
    bool succeed = true;
    // Candidate cdt(mapping, 50);
    // Candidate cdt(mapping, getADG()->numGpeNodes());
    //@Yuan: set mapping II
    //std::cout << "mappersa303 II: " << II << std::endl;
    mapping->setII(II);
   for(int id : dfgNodeIdPlaceOrder){
        auto unmapNode = dfg->node(id);
        //@Yuan: we handle the INPUT when mapping its dst node
        if(unmapNode->operation() == "INPUT")
            continue;
        //@Yuan: we also handle some LOAD units (this kind of LOAD has no src nodes, it more like INPUT)
        //if(unmapNode->operation() == "LOAD" && unmapNode->inputEdges().empty())
        //    continue;
        // std::cout << "mapping node: " << id << " ----------------------------" << std::endl;
        if(!mapping->isMapped(unmapNode)){
            // find candidate ADG nodes for this DFG node
            // cdt.findCandidates(dfgNode);
            // auto& nodeCandidates = cdt.candidates(dfgNode->id());
            auto nodeCandidates = findCandidates(mapping, unmapNode, 50);
            if(nodeCandidates.empty() || tryCandidates(mapping, unmapNode, nodeCandidates) == -1){
                // std::cout << "Cannot map DFG node " << nodeCandidates.size()<<std::endl;
                spdlog::debug("Cannot map DFG node {0} : {1}", unmapNode->id(), unmapNode->name());
                // for(auto& cdt : nodeCandidates){
                //     spdlog::debug("ADG node candidate {}", cdt->id());
                // }
                // Graphviz viz(mapping, "results");
                // viz.printDFGEdgePath();
                succeed = false;
                break;
            }
        }

          //mapping->printAva();
          //std::cout << "Mapping DFG node: " << unmapNode->name() << " id: " << id << " to ADG Node: " << mapping->mappedNode(unmapNode)->id() << std::endl;
        //spdlog::debug("Mapping DFG node {0} : {1} to ADG node {2}", dfgNode->name(), id, mapping->mappedNode(dfgNode)->id());
    }
    return succeed? 1 : -1;
}



// try to map one DFG node to one of its candidates
// return selected candidate index
int MapperSA::tryCandidates(Mapping* mapping, DFGNode* dfgNode, const std::vector<ADGNode*>& candidates){
    // // sort candidates according to their distances with the mapped src and dst ADG nodes of this DFG node
    // std::vector<int> sortedIdx = sortCandidates(mapping, dfgNode, candidates);
    int idx = 0;
    for(auto& candidate : candidates){
            //std::cout << "mapper_sa291: " << candidate->type() << "," <<  candidate->id() << std::endl;
        if(tryCandidate(mapping, dfgNode, candidate)){
            //std::cout << "mapper_sa291: " << candidate->type() << "," <<  candidate->id() << std::endl;
            return idx;
        }
        idx++;
    }
    return -1;
}
//new
// find candidates for one DFG node based on current mapping status
std::vector<ADGNode*> MapperSA::findCandidates(Mapping* mapping, DFGNode* dfgNode, int maxCandidates){
    std::vector<ADGNode*> candidates;
    DFG* dfg = mapping->getDFG();
    //get the ALAP value of the dfgNode
    // int ALAP = dfgNode->getALAP();
    for(auto& elem : mapping->getADG()->nodes()){
        auto adgNode = elem.second;
        auto type = adgNode->type();
        bool sameADGNode = false;
        //select GPE node
        if(type != "GPE"&& type != "LSU"&& type != "IB" && type != "OB"){
            continue;
        }
        if(type == "GPE"){
            GPENode* gpeNode = dynamic_cast<GPENode*>(adgNode);
            // check if the DFG node operationis supported
            if(!gpeNode->opCapable(dfgNode->operation())){
                continue;
            }
            // for(auto& elem : dfgNode->inputs()){
            //     int inNodeId = elem.second.first;
            //     auto inNode = dfg->node(inNodeId);
            //     auto SrcadgNode = mapping->mappedNode(inNode);
            //     if(SrcadgNode){
            //         if(SrcadgNode->id() == gpeNode->id()){ // avoid the same ADGNode be mapped to the connected DFGNode
            //             sameADGNode = true;
            //             break;
            //         }
            //     }
            // }
            // if(sameADGNode) continue;
            candidates.push_back(gpeNode);
        // @Yuan: check will excute later
        /*if(!mapping->isMapped(gpeNode)){
        }*/
        }else if(type == "LSU"){
          LSUNode* lsuNode = dynamic_cast<LSUNode*>(adgNode);
        // check if the DFG node operationis supported
            if(!lsuNode->opCapable(dfgNode->operation())){
                continue;
            }
            candidates.push_back(lsuNode);
        // @Yuan: check will excute later
        /*if(!mapping->isMapped(lsuNode)){
            candidates.push_back(lsuNode);
        }*/
        }else{
             IOBNode* iobNode = dynamic_cast<IOBNode*>(adgNode);
        // check if the DFG node operationis supported
             if(!iobNode->opCapable(dfgNode->operation())){
                 continue;
             }
             candidates.push_back(iobNode);
        }
    }
    // randomly select candidates
    std::random_shuffle(candidates.begin(), candidates.end());
    int num = std::min((int)candidates.size(), maxCandidates);
    candidates.erase(candidates.begin()+num, candidates.end());
    // sort candidates according to their distances with the mapped src and dst ADG nodes of this DFG node
    std::vector<int> sortedIdx = sortCandidates(mapping, dfgNode, candidates);
    std::vector<ADGNode*> sortedCandidates;
    for(int i = 0; i < num; i++){
        sortedCandidates.push_back(candidates[sortedIdx[i]]);
    }
    return sortedCandidates;
}


// get the shortest distance between ADG node and the available ADG input
/*int MapperSA::getAdgNode2InputDist(Mapping* mapping, int id){
    // shortest distance between ADG node (GPE node) and the ADG IO
    int minDist = 0x7fffffff;
    for(auto& jnode : getADG()->nodes()){
        if(jnode.second->type() == "IB" && mapping->isIBAvail(jnode.second)){
            minDist = std::min(minDist, getAdgNodeDist(jnode.first, id));
        }
    }
    return minDist;
}

// get the shortest distance between ADG node and the available ADG output
int MapperSA::getAdgNode2OutputDist(Mapping* mapping, int id){
    int minDist = 0x7fffffff;
    for(auto& jnode : getADG()->nodes()){
        if(jnode.second->type() == "OB" && mapping->isOBAvail(jnode.second)){
            minDist = std::min(minDist, getAdgNodeDist(id, jnode.first));
        }
    }
    return minDist;
}*/

// sort candidates according to their distances with the mapped src and dst ADG nodes of this DFG node
// return sorted index of candidates
std::vector<int> MapperSA::sortCandidates(Mapping* mapping, DFGNode* dfgNode, const std::vector<ADGNode*>& candidates){
    // mapped ADG node IDs of the source and destination node of this DFG node
    // std::cout << "candidates size: " << candidates.size() << std::endl;
    std::vector<int> srcAdgNodeId, dstAdgNodeId;
    DFG* dfg = mapping->getDFG();
    int num2io = 0;
    int num2ls = 0;
    int DFGNodeId = dfgNode->id();
    for(auto& elem : dfgNode->inputs()){
        int inNodeId = elem.second.first;
        auto inNode = dfg->node(inNodeId);
        auto adgNode = mapping->mappedNode(inNode);
        if(adgNode){
            srcAdgNodeId.push_back(adgNode->id());
        }else if(inNode->operation() == "INPUT" ){
            num2io += 1;
        }else if(inNode->operation() == "LOAD"){
            num2ls += 1;
        }
    }
    for(auto& elem : dfgNode->outputs()){
        for(auto outNode : elem.second){
            auto outnode = dfg->node(outNode.first);
           auto adgNode = mapping->mappedNode(outnode);
            if(adgNode){
                dstAdgNodeId.push_back(adgNode->id());
            }else if(outnode->operation() == "OUTPUT"){
                num2io += 1;
            }else if( outnode->operation() == "LOAD"|| outnode->operation() == "STORE"){
                num2ls += 1;
            }
        }
    }
    // std::cout << "srcAdgNodeId size: " << srcAdgNodeId.size() << std::endl;
    // sum distance between candidate and the srcAdgNode & dstAdgNode & IO
    //@Yuan: consider the PE usage as cost
    std::vector<int> sortedIdx, cost; // <candidate-index, sum-distance>
    for(int i = 0; i < candidates.size(); i++){
        int sumDist = 0;
        int usage = 0;
        int dis2io = 0;
        int dis2ls = 0;
        int Hisusage = 0;
        int cdtId = candidates[i]->id();
        for(auto id : srcAdgNodeId){
            // int Dist = getAdgNodeDist(id, cdtId);
            // if(id == cdtId){
            //     sumDist += INT_MAX;
            // }else{
            //     sumDist += Dist;
            // }
            sumDist += getAdgNodeDist(id, cdtId);
            // std::cout << "same node distance: " << getAdgNodeDist(id, cdtId) << std::endl;
            // if(id == cdtId) std::cout << "same node distance: " << getAdgNodeDist(id, cdtId) << std::endl;
        }
        for(auto id : dstAdgNodeId){
            sumDist += getAdgNodeDist(cdtId, id);
        }
        //std::cout << "mapper_sa sortcandidate sum: " <<candidates[i]->id()<<"," <<sum << std::endl;
        //@Yuan: calculate the candidate available time within II
        usage = mapping->numUsedTime(cdtId);
        //@yuan: get the distance between candidate node and IO 
        dis2io = num2io * getAdgNode2IODist(cdtId);
        //@yuan: get the distance between candidate node and IO 
        dis2ls = num2ls * getAdgNode2LSUDist(cdtId);
        //@yuan: gen the historical usage of the candidate node
        Hisusage = mapping->getADGNodeHisUsage(cdtId, DFGNodeId);
        // std::cout << "cdtId: " << cdtId<<" Hisusage: " << Hisusage <<" dis2io: "<<dis2io<<" dis2ls: " <<dis2ls<< " sumDist:"<<sumDist<<std::endl;
        // std::cout << "dis2io: " << dis2io << " dis2ls: " << dis2ls << std::endl;
        sumDist = (sumDist + dis2io + dis2ls - 2*usage)*3 + 1*Hisusage;
        // sumDist = sumDist + dis2io + dis2ls- 3*usage;
        // sumDist = sumDist ;
        //std::cout <<mapping->getADG()->node(cdtId)->type() <<"("<<mapping->getADG()->node(cdtId)->id() << ") sumDist and usage : " << sumDist << "," <<usage << std::endl;
        // cost.push_back(sumDist + 2*usage); 
        // cost.push_back(sumDist );
        cost.push_back(sumDist);
        sortedIdx.push_back(i); 
    }
    std::sort(sortedIdx.begin(), sortedIdx.end(), [&cost](int a, int b){
        return cost[a] < cost[b];
    });
    int from = 0;
    for(int i = 1; i < sortedIdx.size(); ++i){
        if(cost[sortedIdx[i - 1]] != cost[sortedIdx[i]]){  //@Yuan: for the candidates have same cost, disrupt their oder again
            std::random_shuffle(sortedIdx.begin() + from, sortedIdx.begin() + i);
            from = i;
        }
    }
    std::random_shuffle(sortedIdx.begin() + from, sortedIdx.begin() + sortedIdx.size());
    // std::cout << "~~~~~~~~~~~~~~" << std::endl;
    return sortedIdx;
}


// try to map one DFG node to one candidates
bool MapperSA::tryCandidate(Mapping* mapping, DFGNode* dfgNode, ADGNode* candidate){
    DFG* dfg = mapping->getDFG();
    bool succeed = true;
    bool ifAnynearDFGNodemapped = false;
    bool isVio = false;
    int delayUsage = 0;
    int startTime = INT_MAX;
    int endTime = INT_MAX;
    std::vector<DFGEdge*> routedEdges;
    bool test = false;
    //@Yuan: first we need to get the initial mapping time, if current node has ALAP > 0, its mapping time equals to the average mapping time of ALAP - 1
    int mappingtime = 0 ;
    int dfgNodeALAP = dfgNode->getALAP();
    if(dfgNodeALAP != 0){
      mappingtime = mapping->getALAPMapTime(dfgNodeALAP - 1);
    }
    std::set<int> allMappingTime;//@Yuan: because PE/LSU have delay unit, they may have multiple mapping time to use the delay unit
    std::map<int, int> edgeMapTime;//@Yuan: each input/output edge has own mapping time
    std::vector<int> vioDfgEdges; //@Yuan: cache the latency violation edge, if current dfgNode mapping succesfully, it will be record
    std::vector<DFGEdge*> inputEdges; //@Yuan: the edge connect to INPUT or LOAD
    std::vector<DFGEdge*> cycleEdges; //@Yuan: the edge belongs to cycle, we should handle it specially
    //@Yuan: if dfgNode does not have input edge, its mappingtime equals ALAP-1's time
    if(dfgNode->inputEdges().empty()){
        allMappingTime.emplace(mappingtime);
        startTime = mappingtime;
        for(int t = startTime; t < startTime + II; t++){
            bool isExcute = true;
            delayUsage = 0;
            if(mapping->mapDfgNode(dfgNode, candidate, t, isExcute, delayUsage, startTime)){
                mapping->setDelayuse(dfgNode, delayUsage);
                mapping->setALAPMapTime(dfgNodeALAP, dfgNode->id() ,t);
                return true;
            }

        }
        return false;
    }
    //if the dfgNode has input edges, its  mapping is determined by previous dfgNodes
    // route the src edges whose src nodes have been mapped or connected to DFG input port
    for(auto& elem : dfgNode->inputEdges()){
        DFGEdge* edge = dfg->edge(elem.second);
        DFGNode* inNode = dfg->node(edge->srcId());
        if(inNode->operation() == "INPUT"){
            inputEdges.push_back(edge);
            continue;
        }else if(inNode->operation() == "LOAD" && mapping->mappedNode(inNode) == nullptr){//@Yuan: the src LOAD unit has no input
            inputEdges.push_back(edge);
            continue;
        }
        ADGNode* adgNode = mapping->mappedNode(inNode);
        if(adgNode){
            ifAnynearDFGNodemapped = true;
            int time;
            int pre_time = *(mapping->dfgNodeAttr(inNode->id()).mappedtime.rbegin());  // get the preious node's mapping time
            succeed = mapping->routeDfgEdge(edge, adgNode, candidate, pre_time, &time); // route edge between candidate and adgNode
            if(succeed){
                routedEdges.push_back(edge); // cache the routed edge
                allMappingTime.emplace(time);
                edgeMapTime[edge->id()] = time;
                mapping->delVioEdge(edge->id());
                continue;
            }else{
                break;
            }
        }else if(edge->isCycle()){
            //@Yuan: if the edge is self-cycle, we route it now, otherwise we route it when mapping its srcNode
            if(inNode->id() == dfgNode->id()){
                cycleEdges.push_back(edge);
                routedEdges.push_back(edge); // cache the routed edge
                continue;
            }else if(inNode->getALAP() > dfgNodeALAP){
                cycleEdges.push_back(edge);
                continue;
            }else{
                succeed = false;
                break;
            }
        }else{
            succeed = false;
            break;
        }
    }
    if(succeed){
        //@Yuan: we map input here
        for(auto edge : inputEdges){
            //@Yuan: if srcNode has been mapped, the current node's time is certain
            if(ifAnynearDFGNodemapped){
               succeed = false;
               int curTime =  *(allMappingTime.rbegin()); // current latest mapping time
               for(int t = curTime; t >= 0; t--){
                   if(mapping->routeDfgEdge(edge, candidate, t)){
                      routedEdges.push_back(edge); // cache the routed edge
                      allMappingTime.emplace(t);
                      edgeMapTime[edge->id()] = t;
                      mapping->delVioEdge(edge->id());
                      succeed = true;
                      break;
                   }
               }
               if(!succeed)
                 break;
            }else{                                          //@Yuan: otherwise, using initial mapping time (the average mapping time of ALAP-1 nodes)
               succeed = false;
               for(int t = mappingtime; t < mappingtime + 7 ; t++){
                  if(mapping->routeDfgEdge(edge, candidate, t)){
                      routedEdges.push_back(edge); // cache the routed edge
                      allMappingTime.emplace(t);
                      edgeMapTime[edge->id()] = t;
                      mapping->delVioEdge(edge->id());
                      succeed = true;
                      break;
                   }
               }
               if(!succeed)
                 break;
            }

        }
    }
    //@Yuan: we handle cycle edges here, where the current dfgNode has a certain mapping time
    if(succeed && !cycleEdges.empty()){
    //get the latest mapping time of current dfgNode, the current iteration result will be generated at this time
        int resultTime = 0;
        if(allMappingTime.empty()){
            allMappingTime.emplace(mappingtime);
            resultTime = mappingtime;
        }else{
            resultTime = *(allMappingTime.rbegin());
        }
        //allMappingTime.clear();
        for(auto edge : cycleEdges){
            if(edge->srcId() == dfgNode->id()){ // self-cycle
                int selfCycleTime = resultTime + dfgNode->opLatency() - II;
                mapping->routeSelfDfgEdge(dfgNode, edge, candidate, selfCycleTime);
                mapping->delVioEdge(edge->id());
                edgeMapTime[edge->id()] = selfCycleTime;
                allMappingTime.emplace(selfCycleTime);
            }
        }
    }
    if(succeed){
        //@Yuan: we do synchronization here, and calculate the latency violation, just for GPE and LUS who has delay unit
        for(auto edge : routedEdges){
            if(candidate->type() == "GPE"){
                GPENode * gpe = dynamic_cast<GPENode*>(candidate);
                int maxDelay = gpe->maxDelay();
                int edgeId = edge->id();
                int latestTime = *(allMappingTime.rbegin());
                int vioEdgeTime = edgeMapTime[edgeId];
                int delay = latestTime - vioEdgeTime;      //actual delay usage
                if(delay > maxDelay){// actual delay usage bigger than maxdelay, needs to go back the path to fix the violation
                    //std::cout << "[HETA] timing violation occur~~~ " << std::endl;
                    int vio = delay - maxDelay;
                    int newTime;
                    newTime = mapping->tryFixVio(dfgNode, candidate, edge, latestTime);
                    edgeMapTime[edgeId] = newTime;     //after fixing, the edge that has violation need to refresh its end time
                    if(newTime == -1){
                        mapping->VioEdge(edgeId, vio);     //fail to fix, the edge still be the violated edge
                        //std::cout << "[HETA] fail to fix the violation! " << std::endl;
                        if(!edge->isCycle()){
                            isVio = true;
                            succeed = false;
                            break;
                        }else{
                            allMappingTime.erase(vioEdgeTime);
                            mapping->unrouteDfgEdge(edge, true);
                            routedEdges.erase(std::remove(routedEdges.begin(), routedEdges.end(), edge), routedEdges.end());
                            continue;
                        }
                    }else{
                        delay = abs(latestTime - newTime);  //after fixing the path, we will have new delay and violation
                        vio = delay - maxDelay;
                        if(vio > 0 ){//still has violation
                            succeed = false;
                            isVio = true;
                            int vioEdgeId;
                            for(auto edgeTimes : edgeMapTime){
                                if(edgeTimes.second == std::min(latestTime, newTime)){//find the earliest edge, it cause the violation
                                    mapping->VioEdge(edgeTimes.first, vio);
                                    vioEdgeId = edgeTimes.first;
                                    break;
                                }
                            }
                            if(!dfg->edge(vioEdgeId)->isCycle()){
                                succeed = false;
                                isVio = true;
                                break;
                            }else{
                                allMappingTime.erase(vioEdgeTime);
                                mapping->unrouteDfgEdge(edge, true);
                                routedEdges.erase(std::remove(routedEdges.begin(), routedEdges.end(), edge), routedEdges.end());
                                continue;
                            }
                            //std::cout << "[HETA] After fixing still has violation! " << std::endl;
                            break;
                        }else{
                            allMappingTime.erase(vioEdgeTime);
                            allMappingTime.emplace(newTime);
                            //std::cout << "[HETA] Fix violation successful!  " << std::endl;
                        }
                    }
                }else{
                    continue;
                }
            }else if(candidate->type() == "LSU"){
                LSUNode * lsu = dynamic_cast<LSUNode*>(candidate);
                int maxDelay = lsu->maxDelay();
                int edgeId = edge->id();
                int latestTime = *(allMappingTime.rbegin());  // candidate ADGNode latest mapping time
                int vioEdgeTime = edgeMapTime[edgeId];
                int delay = latestTime - vioEdgeTime;      //actual delay usage
                if(delay > maxDelay){// actual delay usage bigger than maxdelay, needs to go back the path to fix the violation
                    int vio = delay - maxDelay;
                    int newTime;
                    newTime = mapping->tryFixVio(dfgNode, candidate, edge, latestTime);
                    edgeMapTime[edgeId] = newTime;     //after fixing, the edge that has violation need to refresh its end time
                    if(newTime == -1){
                        mapping->VioEdge(edgeId, vio);
                        isVio = true;
                        succeed = false;
                        //std::cout << "[HETA] fail to fix the violation! " << std::endl;
                        break;
                    }else{
                        delay = abs(latestTime - newTime);  //after fixing the path, we will have new delay and violation
                        vio = delay - maxDelay;
                        if(vio > 0 ){//still has violation
                            succeed = false;
                            isVio = true;
                            for(auto edgeTimes : edgeMapTime){
                                if(edgeTimes.second == std::min(latestTime, newTime)){//find the earliest edge, it cause the violation
                                    mapping->VioEdge(edgeTimes.first, vio);
                                }
                            }
                            //std::cout << "[HETA] after fixing still has violation! " << std::endl;
                            break;
                        }else{
                            allMappingTime.erase(vioEdgeTime);
                            allMappingTime.emplace(newTime);
                            //std::cout << "[HETA] fix violation successful!  " << std::endl;
                        }
                    }
                }else{
                    continue;
                }
            }

        }
    }

    // map the DFG node to this candidate
    if(succeed){
        startTime = *(allMappingTime.begin());
        endTime = *(allMappingTime.rbegin());
        delayUsage =  endTime - startTime;
    for(int t = startTime; t <= endTime; t++){
        if(candidate->type() == "OB" ){// due to one OB can be mapped more than one dfgNode (OB doesn't have delay unit, so it maps more output nodes not because shares the delay unit), it needs to handle differently with GPE/LSU
            if(!mapping->mapOutDfgNode(dfgNode, candidate, t)){
                mapping->unmapDfgNode(dfgNode);
                succeed = false;
                break;
            }
        }else{
            bool isExecute = false;
            if(t == startTime || t == endTime)
                isExecute = true;
            if(!mapping->mapDfgNode(dfgNode, candidate, t, isExecute, delayUsage, startTime)){
                mapping->unmapDfgNode(dfgNode);
                succeed = false;
                 //std::cout << "mappersa609: increase time: "  << t << std::endl;
                break;
            }
        }
    }
    }
    //@Yuan: if fail at mapping nodes, we still need unmap the edge
    if(!succeed){
        //std::cout << "mappersa506 routedEdges size: " << routedEdges.size() << std::endl;
        for(auto re : routedEdges){ // unroute all the routed edges
            mapping->unrouteDfgEdge(re, isVio);
        }
        return false;
    }
    //@Yuan: if mapping successfully, record the latency violation edge and set the violation
    if(succeed){
        //@Yuan: if the current dfgNode is mapped, we should consider to route to its dstNodes, which is the up-stream node of the cycle
        // route the dst edge whose dst nodes have been mapped (it usually be a cycle)
        for(auto& elem : dfgNode->outputEdges()){
            //@Yuan: get the candidate ADGNode latest mapping time
            int starTime = 0;
            if(allMappingTime.empty()){
               starTime =  mappingtime;
            }else{
               starTime = *(allMappingTime.rbegin());
            }
            for(int outEdgeId : elem.second){  //to find the output edge that belongs to cycle (the dstnode of the edge is mapped)
                DFGEdge* edge = dfg->edge(outEdgeId);
                DFGNode* outNode = dfg->node(edge->dstId());
                ADGNode* adgNode = mapping->mappedNode(outNode);
                int time;
                int newDelayUsage = 0;
                int outNodeDelayUsage = 0;
                int maxDelay = 0;
                int vio = 0;
                int newTime = 0;
                bool doneMap = true;
                if(adgNode && outNode->getALAP() < dfgNodeALAP){
                    succeed = mapping->routeDfgEdge(edge, candidate, adgNode, starTime, &time);
                }else{
                    continue;
                }
                if(!succeed)
                    break;
                //mapping->delVioEdge(edge->id());
                //if route successful, begin to re-schedule the up-stream dstNode
                time = time - II;// back to the right time range
                int outNodelatestTime = mapping->getLatest(outNode->id());
                if(time > outNodelatestTime){
                    succeed = false;
                    break;
                }
                if(adgNode->type() == "GPE"){
                    GPENode * gpe = dynamic_cast<GPENode*>(adgNode);
                    maxDelay = gpe->maxDelay();
                }else{
                    LSUNode * lsu = dynamic_cast<LSUNode*>(adgNode);
                    maxDelay = lsu->maxDelay();
                }
                newDelayUsage = outNodelatestTime - time;
                if(newDelayUsage > maxDelay){ // has timing violation
                    vio = newDelayUsage - maxDelay;
                    doneMap = false;
                }else{// has no timing violation, try to re-map the up-stream dfgNode
                    for(int t = time; t <= outNodelatestTime; t++){
                        bool isExecute = false;
                        if( t == time || t == outNodelatestTime){
                            isExecute = true;
                        }
                        if(!mapping->mapDfgNode(outNode, adgNode, t, isExecute, newDelayUsage, time)){
                            for(int T = time; T < t -1; T++){
                                mapping->unuseADGNode(outNode, adgNode, T);
                            }
                            vio = outNodelatestTime - time;               // the vio is used to schedule the dfgNode that make mapping up-stream easier
                            doneMap = false;
                            break;
                        }
                    }
                }
                if(doneMap && vio == 0){  //map successful
                    mapping->setDelayuse(outNode, newDelayUsage);
                    mapping->updateEndTime(edge, time);
                    mapping->delVioEdge(edge->id());
                    continue;
                }else if( !doneMap && vio == 0){// not because of the delay unit, just map fail
                    mapping->VioEdge(edge->id(), vio);     //fail to fix, the edge still be the violated edge
                    succeed = false;
                    mapping->unmapDfgNode(outNode);
                    break;
                }else{// has timing violation or outnode can not use so much delay unit, try to fix
                    //std::cout << "[HETA] timing violation occur~~~ " << std::endl;
                    newTime = mapping->tryFixVio(outNode, adgNode, edge, outNodelatestTime + II);
                    if( newTime == -1){
                        mapping->VioEdge(edge->id(), vio);     //fail to fix, the edge still be the violated edge
                        //mapping->setVio(edge->id(), vio);
                        succeed = false;
                        //std::cout << "[HETA] fail to fix the violation! " << std::endl;
                        break;
                    }else{
                        newTime = newTime -II;
                        if(newTime > outNodelatestTime || newTime == time){
                            mapping->VioEdge(edge->id(), vio);     //fail to fix, the edge still be the violated edge
                            //mapping->setVio(edge->id(), vio);
                            succeed = false;
                            break;
                        }
                        newDelayUsage = outNodelatestTime - newTime;
                        vio = newDelayUsage - maxDelay;
                        if(vio > 0){ // after fixing still has timing violation
                            mapping->VioEdge(edge->id(), vio);     //fail to fix, the edge still be the violated edge
                            succeed = false;
                            //std::cout << "[HETA] after fixing still has violation! " << std::endl;
                            break;
                        }else{// has no timing violation, try to re-map the up-stream dfgNode
                            for(int t = newTime; t <= outNodelatestTime; t++){
                                bool isExecute = false;
                                if( t == newTime || t == outNodelatestTime){
                                isExecute = true;
                                }
                                if(!mapping->mapDfgNode(outNode, adgNode, t, isExecute, newDelayUsage, newTime)){// can not re-map the up-stream node
                                    mapping->unmapDfgNode(outNode);
                                    succeed = false;
                                    mapping->VioEdge(edge->id(), 0);     //fail to fix, the edge still be the violated edge
                                    //mapping->setVio(edge->id(), 0);
                                    break;
                                }
                            }
                            if(!succeed)
                                break;
                            //re-map successful
                                mapping->setDelayuse(outNode, newDelayUsage);
                                mapping->updateEndTime(edge, newTime);
                                mapping->delVioEdge(edge->id());
                                //std::cout << "[HETA] fix violation successful!  " << std::endl;
                        }
                    }
                }
            }
        if(!succeed)
            break;
    }
      //set the delay unit usage of current dfgNode
        //delayUsage =  *(allMappingTime.rbegin()) - *(allMappingTime.begin());
        mapping->setDelayuse(dfgNode, delayUsage);
        int sum = std::accumulate(allMappingTime.begin(),allMappingTime.end(), 0);
        int avrTime = int (sum / allMappingTime.size());
        mapping->setALAPMapTime(dfgNodeALAP, dfgNode->id() ,avrTime);
    }
    return succeed;
}




// objective funtion
int MapperSA::objFunc(Mapping* mapping){
    int maxEdgeLen = 0;
    int totalEdgeLen = 0;
    mapping->getEdgeLen(totalEdgeLen, maxEdgeLen);
    int obj = mapping->maxLat() * 100 +
              maxEdgeLen * 10 + totalEdgeLen;
    return obj;
}


// SA the probablity of accepting new solution
bool MapperSA::metropolis(double diff, double temp){
    if(diff < 0){
        return true;
    }else{
        return randfloat() < exp(-diff/temp);
    }
}


// annealing funtion
float MapperSA::annealFunc(float temp){
    float k = 0.80;
    return float(k*temp);
}

// get the shortest distance between ADG node and the available IOB
int MapperSA::getAdgNode2IODist(int id){
    // shortest distance between ADG node (GPE node) and the ADG IO(IB or OB)
    int minDist = 0x7fffffff;
    for(auto& jnode : getADG()->nodes()){            
        if(jnode.second->type() == "IB" || jnode.second->type() == "OB"){                
            minDist = std::min(minDist, getAdgNodeDist(jnode.first, id));
        }                   
    }
    return minDist;
}

// get the shortest distance between ADG node and the available LSU
int MapperSA::getAdgNode2LSUDist(int id){
    // shortest distance between ADG node (GPE node) and the ADG LSU
    int minDist = 0x7fffffff;
    for(auto& jnode : getADG()->nodes()){            
        if(jnode.second->type() == "LSU" ){                
            minDist = std::min(minDist, getAdgNodeDist(jnode.first, id));
        }                   
    }
    return minDist;
}
