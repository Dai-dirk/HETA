
#include "mapper/mapper.h"


Mapper::Mapper(ADG* adg): _adg(adg) {
    initializeAdg();
}

Mapper::Mapper(ADG* adg, DFG* dfg): _adg(adg), _dfg(dfg) {
    initializeAdg();
    initializeDfg();
    _mapping = new Mapping(adg, dfg);
    // initializeCandidates();
    _isDfgModified = false;
    //@Yuan: use topology order
    initialALAP();
    //sortDfgNodeInPlaceOrder();
}

Mapper::~Mapper(){
    if(_mapping != nullptr){
        delete _mapping;
    }
    if(_dfgModified != nullptr){
        delete _dfgModified;
    }
}

void Mapper::setADG(ADG* adg){
    _adg = adg;
    initializeAdg();
    if(_mapping != nullptr){
        delete _mapping;
    }
    _mapping = new Mapping(adg, _dfg);
}

// set DFG and initialize DFG
// modify: if the DFG is a modified one
void Mapper::setDFG(DFG* dfg, bool modify){
    _dfg = dfg;
    initializeDfg();
    if(_mapping != nullptr){
        delete _mapping;
    }
    _mapping = new Mapping(_adg, dfg);
    // initializeCandidates();
    if(modify){
        setDfgModified(dfg);
    } else{
        _isDfgModified = false;
    }
    //@Yuan: use topology order
    initialALAP();
    //pre-optimize for inserting pass node
    preOptDFG();
    //sortDfgNodeInFlexibilityOrder();
    // sortDfgNodeInPlaceOrder();
}


// set modified DFG and delete the old one
void Mapper::setDfgModified(DFG* dfg){
    if(_dfgModified != nullptr){
        delete _dfgModified;
    }
    _dfgModified = dfg;
    _isDfgModified = true;
}

// // set ADG and initialize ADG
// void Mapper::setADG(ADG* adg){
//     _adg = adg;
//     initializeAdg();
// }


// initialize mapping status of ADG
void Mapper::initializeAdg(){
    // std::cout << "Initialize ADG\n";
    calAdgNodeDist();
}


// initialize mapping status of DFG
void Mapper::initializeDfg(){
    // topoSortDfgNodes();
    _dfg->topoSortNodes();
}


// // initialize candidates of DFG nodes
// void Mapper::initializeCandidates(){
//     Candidate cdt(_mapping, 50);
//     cdt.findCandidates();
//     // candidates = cdt.candidates(); // RETURN &
//     candidatesCnt = cdt.cnt();
// }


// // sort dfg nodes in reversed topological order
// // depth-first search
// void Mapper::dfs(DFGNode* node, std::map<int, bool>& visited){
//     int nodeId = node->id();
//     if(visited.count(nodeId) && visited[nodeId]){
//         return; // already visited
//     }
//     visited[nodeId] = true;
//     for(auto& in : node->inputs()){
//         int inNodeId = in.second.first;
//         if(inNodeId == _dfg->id()){ // node connected to DFG input port
//             continue;
//         }
//         dfs(_dfg->node(inNodeId), visited); // visit input node
//     }
//     dfgNodeTopo.push_back(_dfg->node(nodeId));
// }

// // sort dfg nodes in reversed topological order
// void Mapper::topoSortDfgNodes(){
//     std::map<int, bool> visited; // node visited status
//     for(auto& in : _dfg->outputs()){
//         int inNodeId = in.second.first;
//         if(inNodeId == _dfg->id()){ // node connected to DFG input port
//             continue;
//         }
//         dfs(_dfg->node(inNodeId), visited); // visit input node
//     }
// }


// calculate the shortest path among ADG nodes
void Mapper::calAdgNodeDist(){
    // map ADG node id to continuous index starting from 0
    std::map<int, int> _adgNodeId2Idx;
    // distances among ADG nodes
    std::vector<std::vector<int>> _adgNodeDist; // [node-idx][node-idx]
    int i = 0;
    // if the ADG node with the index is GIB
    std::map<int, bool> adgNodeIdx2GIB;
    for(auto& node : _adg->nodes()){
        adgNodeIdx2GIB[i] = (node.second->type() == "GIB");
        _adgNodeId2Idx[node.first] = i++;
    }
    int n = i; // total node number
    int inf = 0x7fffffff;
    _adgNodeDist.assign(n, std::vector<int>(n, inf));
    for(auto& node : _adg->nodes()){
        int idx = _adgNodeId2Idx[node.first];
        _adgNodeDist[idx][idx] = INT_MAX;
        for(auto& src : node.second->inputs()){
            int srcId = src.second.first;
            if(srcId == _adg->id()){
                continue; // connected to ADG input port
            }
            int srcPort = src.second.second;
            ADGNode* srcNode = _adg->node(srcId);
            int dist = 1;
            if(srcNode->type() == "GIB" && node.second->type() == "GIB"){
                if(dynamic_cast<GIBNode*>(srcNode)->outReged(srcPort)){ // output port reged
                    dist = 2;
                }
            }
            int srcIdx = _adgNodeId2Idx[srcId];
            _adgNodeDist[srcIdx][idx] = dist;
        }
    }
    // Floyd algorithm
    for (int k = 0; k < n; ++k) {
        if(adgNodeIdx2GIB[k]){
            for (int i = 0; i < n; ++i) {
                for (int j = 0; j < n; ++j) {
                    if (_adgNodeDist[i][k] < inf && _adgNodeDist[k][j] < inf &&
                        _adgNodeDist[i][j] > _adgNodeDist[i][k] + _adgNodeDist[k][j]) {
                        _adgNodeDist[i][j] = _adgNodeDist[i][k] + _adgNodeDist[k][j];
                    }
                }
            }
        }
    }

    // shortest distance between two ADG nodes (GPE/IOB nodes)
    for(auto& inode : _adg->nodes()){
       // if(inode.second->type() == "GIB" || inode.second->type() == "OB"){
        if(inode.second->type() == "GIB" ){
            continue;
        }
        int i = _adgNodeId2Idx[inode.first];
        for(auto& jnode : _adg->nodes()){
            if(jnode.second->type() == "GIB" ||(inode.second->type() == "IB" && jnode.second->type() == "IB")\
            ||(inode.second->type() == "OB" && jnode.second->type() == "OB")||(inode.second->type() == "OB" && jnode.second->type() == "IB")||
              (inode.second->type() == "IB" && jnode.second->type() == "OB")){
                continue;
            }
            int j = _adgNodeId2Idx[jnode.first];
            _adgNode2NodeDist[std::make_pair(inode.first, jnode.first)] = _adgNodeDist[i][j];
            //std::cout <<inode.second->type() <<"(" <<inode.first <<")"<< "to" <<jnode.second->type() <<"("<< jnode.first << ") : " << _adgNodeDist[i][j] << std::endl;;
        }
        //std::cout << std::endl;
    }

    // // shortest distance between ADG node (GPE node) and the ADG IO
    // for(auto& inode : _adg->nodes()){
    //     if(inode.second->type() != "GPE"){
    //         continue;
    //     }
    //     int i = _adgNodeId2Idx[inode.first];
    //     int minDist2IB = inf;
    //     int minDist2OB = inf;
    //     for(auto& jnode : _adg->nodes()){
    //         auto jtype = jnode.second->type();
    //         int j = _adgNodeId2Idx[jnode.first];
    //         if(jtype == "IB"){
    //             minDist2IB = std::min(minDist2IB, _adgNodeDist[j][i]);
    //         }else if(jtype == "OB"){
    //             minDist2OB = std::min(minDist2OB, _adgNodeDist[i][j]);
    //         }
    //     }
    //     _adgNode2IODist[inode.first] = std::make_pair(minDist2IB, minDist2OB);
    //     // std::cout << inode.first << ": " << minDist2IB << "," << minDist2OB << std::endl;
    // }
}


// get the shortest distance between two ADG nodes
int Mapper::getAdgNodeDist(int srcId, int dstId){
    // return _adgNodeDist[_adgNodeId2Idx[srcId]][_adgNodeId2Idx[dstId]];
    return _adgNode2NodeDist[std::make_pair(srcId, dstId)];
}

// // get the shortest distance between ADG node and ADG input
// int Mapper::getAdgNode2InputDist(int id){
//     return _adgNode2IODist[id].first;
// }

// // get the shortest distance between ADG node and ADG input
// int Mapper::getAdgNode2OutputDist(int id){
//     return _adgNode2IODist[id].second;
// }

// new
// calculate the number of the candidates for one DFG node
int Mapper::calCandidatesCnt(DFGNode* dfgNode, int maxCandidates){
    int candidatesCnt = 0;
    for(auto& elem : _adg->nodes()){
        auto adgNode = elem.second;
        //select GPE node
        if(adgNode->type() != "GPE" && adgNode->type() != "LSU"){
            continue;
        }
        if(adgNode->type() == "GPE"){
          GPENode* gpeNode = dynamic_cast<GPENode*>(adgNode);
          // check if the DFG node operationis supported
          if(gpeNode->opCapable(dfgNode->operation())){
            candidatesCnt++;
        }
        }else{
          LSUNode* lsuNode = dynamic_cast<LSUNode*>(adgNode);
          // check if the DFG node operationis supported
          if(lsuNode->opCapable(dfgNode->operation())){
            candidatesCnt++;
        }
        }

    }
    return std::min(candidatesCnt, maxCandidates);
}

// sort the DFG node IDs in placing order (ALAP order)
void Mapper::sortDfgNodeInPlaceOrder(){
    std::map<int, int> ALAP; // <dfgnode-id, ALAP>
    std::map<int, int> flexibility; // <dfgnode-id, flexibility>
    std::map<int, int> out_degree; // <dfgnode-id, flexibility>
    std::vector<int> sortedIdx; // <candidate-index, sum-distance>
    dfgNodeIdPlaceOrder.clear();
    // topological order
    for(auto node : _dfg->topoNodes()){
        dfgNodeIdPlaceOrder.push_back(node->id());
        ALAP[node->id()] = node->getALAP();
        flexibility[node->id()] = node->getALAP() - node->getASAP();
        for(auto elem : node->outputEdges()){
           out_degree[node->id()] = elem.second.size();
        }
        for(auto elem : node->inputEdges()){
           out_degree[node->id()] += 1;
        }
        //std::cout << "node: " << node->name() << " out_dgree: " << out_degree[node->id()] << std::endl;
    }
    std::stable_sort(dfgNodeIdPlaceOrder.begin(), dfgNodeIdPlaceOrder.end(), [&](int a, int b){
        return ALAP[a] <  ALAP[b];
    });
    for (int node: dfgNodeIdPlaceOrder) {
        sortedIdx.push_back(node);
    //std::cout << "OLD Node ID: " << _dfg->node(node)->name()<<"("<<_dfg->node(node)->id() <<") ALAP: " <<_dfg->node(node)->getALAP()<< " flexibility: "<< flexibility[_dfg->node(node)->id()]<<std::endl;
    }
    int from = 0;
    for(int i = 1; i < sortedIdx.size(); ++i){
        if(ALAP[sortedIdx[i - 1]] != ALAP[sortedIdx[i]]){  //for the nodes has same ALAP, sort by flexibility
            std::stable_sort(dfgNodeIdPlaceOrder.begin() + from, dfgNodeIdPlaceOrder.begin() + i , [&](int a, int b){
                return out_degree[a] > out_degree[b];
            });
            from = i;
        }
    }
    std::stable_sort(dfgNodeIdPlaceOrder.begin() + from, dfgNodeIdPlaceOrder.end() , [&](int a, int b){
                return out_degree[a] > out_degree[b];
            });
    /*int from = 0;
    for(int i = 1; i < sortedIdx.size(); ++i){
        if(ALAP[sortedIdx[i - 1]] != ALAP[sortedIdx[i]]){  //for the nodes has same ALAP, sort by flexibility
            std::stable_sort(dfgNodeIdPlaceOrder.begin() + from, dfgNodeIdPlaceOrder.begin() + i , [&](int a, int b){
                return flexibility[a] > flexibility[b];
            });
            from = i;
        }
    }
    std::stable_sort(dfgNodeIdPlaceOrder.begin() + from, dfgNodeIdPlaceOrder.end() , [&](int a, int b){
                return flexibility[a] > flexibility[b];
            });*/
    for (int node: dfgNodeIdPlaceOrder) {
    //std::cout << "Node ID: " << _dfg->node(node)->name()<<"("<<_dfg->node(node)->id() <<") ALAP: " <<_dfg->node(node)->getALAP()<< " flexibility: "<< flexibility[_dfg->node(node)->id()]<<std::endl;
    std::cout << "Node ID: " << _dfg->node(node)->name()<<"("<<_dfg->node(node)->id() <<") ALAP: " <<_dfg->node(node)->getALAP()<< " out_dgree: "<< out_degree[_dfg->node(node)->id()]<<std::endl;
    }

}
//new for module schedule
void Mapper::initialALAP(){
    //first we calculate the ALAP value
    std::map< DFGNode *, int> ALAPvalue;   // <dfgnode-id, ALAP>
    int maxValue = 0;
   for(auto node = _dfg->topoNodes().rbegin(); node !=_dfg->topoNodes().rend(); ++node){
    //for(DFGNode * node :_dfg->topoNodes()){
       int level = 0;
       for(auto outs : (**node).outputs()){   // all outputs
          for(auto out : outs.second){       //get <output node id , output node port>
             if(out.first != 0){
               if(level < _dfg->node(out.first)->getALAP() + (**node).opLatency()){ // dstALAP + self oplatency
                 level =  _dfg->node(out.first)->getALAP() + (**node).opLatency();
                }
              }
            }
        }
       (**node).setALAP(level);
       if (maxValue < level) {
          maxValue = level;
        }
    }
    for (DFGNode* node: _dfg->topoNodes()) {
    node->setALAP(maxValue - node->getALAP());
    }
     //second we calculate the ASAP value
    // int low_flex = 0;
     for(DFGNode * node :_dfg->topoNodes()){
         int level = 0;
         for(auto ins : node->inputs()){
            if(ins.second.first != 0){
                if(level < _dfg->node(ins.second.first)->getASAP() + _dfg->node(ins.second.first)->opLatency() )
                level = _dfg->node(ins.second.first)->getASAP() + _dfg->node(ins.second.first)->opLatency() ;
            }
         node->setASAP(level);
         }
    //get the lowest flexibility
         /*if(low_flex > node->getALAP() - node->getASAP())
           low_flex = node->getALAP() - node->getASAP();*/
     }
    /*for (DFGNode* node: _dfg->topoNodes()) {
    std::cout << "Node ID: " << node->name()<<"("<<node->id() <<") ASAP:" << node->getASAP() << std::endl;
    }*/

}

//pre-optimized the DFG
void Mapper::preOptDFG(){
     std::map<int, int> Vioedges;// <edge id, pass num>
    int maxDelay = 0;
    for(auto elem: _adg->nodes()){
        if(elem.second->type() == "GPE" ){
            maxDelay = std::max(dynamic_cast<GPENode*>(elem.second)->maxDelay(), maxDelay);
        }else if(elem.second->type() == "LSU"){
            maxDelay = std::max(dynamic_cast<LSUNode*>(elem.second)->maxDelay(), maxDelay);
        }
    }
     for(auto & node : _dfg->topoNodes()){
        int curASAP = node->getASAP();
        int curALAP = node->getALAP();
        //std::cout << "curNode and maxDelay and inedgesize: " << node->name() << "," << maxDelay <<","<<node->inputEdges().size()<< std::endl;
        for(auto inEdge: node->inputEdges()){
            int inEdgeId = inEdge.second;
            //std::cout << "edge id :" << inEdgeId << std::endl;
            int srcId = _dfg->edge(inEdgeId)->srcId();
            DFGNode* srcNode = _dfg->node(srcId);
            int srcASAP = srcNode->getASAP();
            int srcALAP = srcNode->getALAP();
            int ALAI_dif  = std::min(std::abs(curALAP - srcALAP),std::abs(curASAP - srcASAP));
            if(ALAI_dif <= 1)// only consider bigger difference value between ALAP
                continue;
            int max_size = std::max(_adg->rows(),_adg->colums());
            //get the  maximum delay number of ADGNode 
            if(ALAI_dif > (maxDelay + 0.5 * max_size)){
                int num = std::floor(ALAI_dif / (maxDelay + 0.5 * max_size));
            std::cout << "ALAI_dif: " << ALAI_dif << "," << max_size<< std::endl;
                Vioedges[inEdgeId] = num;
                continue;
                
            }else{
                continue;
            }
        }
     }
     if(!Vioedges.empty()){
        for(auto elem : Vioedges){
                int maxNodeId = _dfg->nodes().rbegin()->first; // std::map auto sort the key
                int maxEdgeId = _dfg->edges().rbegin()->first;
                DFGEdge* e = _dfg->edge(elem.first);
                int srcId = e->srcId();
                int dstId = e->dstId();
                DFGNode* srcnode = _dfg->node(srcId);
                DFGNode* dstnode = _dfg->node(dstId);
                std::cout << "insertnode from: " << srcnode->name() << " to: " << dstnode->name() << " pass_num: " << elem.second <<std::endl;
                int srcPortIdx = e->srcPortIdx();
                int dstPortIdx = e->dstPortIdx();
                _dfg->delEdge(elem.first);
                int lastId = srcId;
                int lastPort = srcPortIdx;
                for(int i = 0; i < elem.second; i++){
                    DFGNode* newNode = new DFGNode();
                    newNode->setId(++maxNodeId);
                    newNode->setName("pass"+std::to_string(maxNodeId));
                    newNode->setOperation("PASS");
                    _dfg->addNode(newNode);
                    DFGEdge* e1 = new DFGEdge(lastId, maxNodeId);
                    e1->setId(++maxEdgeId);
                    e1->setSrcPortIdx(lastPort);
                    e1->setDstPortIdx(0);
                    _dfg->addEdge(e1);
                    lastId = maxNodeId;
                    lastPort = 0;
                    std::cout << "[HETA] Pre-optimize, inserting one pass node!" << std::endl;
                }
                DFGEdge* e2 = new DFGEdge(maxNodeId, dstId);
                e2->setId(++maxEdgeId);
                e2->setSrcPortIdx(0);
                e2->setDstPortIdx(dstPortIdx);
                _dfg->addEdge(e2);
        }
        initializeDfg();
        initialALAP();
     }
}



// ===== timestamp functions >>>>>>>>>
void Mapper::setStartTime(){
    _start = std::chrono::steady_clock::now();
}


void Mapper::setTimeOut(double timeout){
    _timeout = timeout;
}


//get the running time in millisecond
double Mapper::runningTimeMS(){
    auto end = std::chrono::steady_clock::now();
    double runTime = std::chrono::duration_cast<std::chrono::milliseconds>(end-_start).count();
    setRunTime(runTime);
    return std::chrono::duration_cast<std::chrono::milliseconds>(end-_start).count();
}



// ==== map functions below >>>>>>>>
// check if the DFG can be mapped to the ADG according to the resources
bool Mapper::preMapCheck(ADG* adg, DFG* dfg){
    // first, check the I/O port number
    if(adg->numInputs() < dfg->numInputs() || adg->numOutputs() < dfg->numOutputs()){
        std::cout << "This DFG has too many I/O port!\n";
        return false;
    }
    // second, check the total node number
    if((adg->numGpeNodes()+adg->numLSUNodes()) < dfg->nodes().size()){                    //new, number of GPE Nodes + number of LSU Nodes
        std::cout << "This DFG has too many nodes!\n";
        return false;
    }
    // third, check if there are enough ADG nodes that can map the DFG nodes
    // supported operation count of ADG
    std::map<std::string, int> adgOpCnt;
    for(auto& elem : adg->nodes()){
        if(elem.second->type() == "GPE"){
            auto node = dynamic_cast<GPENode*>(elem.second);
            for(auto& op : node->operations()){
                if(adgOpCnt.count(op)){
                    adgOpCnt[op] += 1;
                } else {
                    adgOpCnt[op] = 1;
                }
            }
        }else if(elem.second->type() == "LSU"){
            auto node = dynamic_cast<LSUNode*>(elem.second);
            for(auto& op : node->operations()){
                if(adgOpCnt.count(op)){
                    adgOpCnt[op] += 1;
                } else {
                    adgOpCnt[op] = 1;
                }
            }
        }else if(elem.second->type() == "IB"){
            auto node = dynamic_cast<IOBNode*>(elem.second);
            for(auto& op : node->operations()){
                std::cout << "op: " << op <<std::endl;
                if(adgOpCnt.count(op)){
                    adgOpCnt[op] += 1;
                } else {
                    adgOpCnt[op] = 1;
                }
            }
        }else if(elem.second->type() == "OB"){
            auto node = dynamic_cast<IOBNode*>(elem.second);
            for(auto& op : node->operations()){
                std::cout << "op: " << op <<std::endl;
                if(adgOpCnt.count(op)){
                    adgOpCnt[op] += 1;
                } else {
                    adgOpCnt[op] = 1;
                }
            }
        }
    }
    // operation count of DFG
    std::map<std::string, int> dfgOpCnt;
    //int lsOpCnt=0;                          //new
    for(auto& elem : dfg->nodes()){
        auto op = elem.second->operation();
        assert(!op.empty());
           if(dfgOpCnt.count(op)){
            dfgOpCnt[op] += 1;
           } else {
            dfgOpCnt[op] = 1;
           }

    }
    //std::cout << "mapper335 precheck done: " << std::endl;
    for(auto& elem : dfgOpCnt){
        if(adgOpCnt[elem.first] < elem.second){
            std::cout << "No enough ADG nodes to support " << elem.first << std::endl;
            return false; // there should be enough ADG nodes that support this operation
        }
    }
    /*if(adg->numLSUNodes() < lsOpCnt){
        std::cout << "No enough LSU nodes to support LOAD and STORE operations " << std::endl;
        return false;
    }*/
    return true;
}

// // map the DFG to the ADG
// bool Mapper::mapping(){

// }


// mapper with running time
bool Mapper::mapperTimed(bool spatial, bool isHeteMapping){
    setStartTime();
    //new
    if(!spatial){
    // Initialize the II
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        int ResMII = getResMII(getADG(), getDFG());
        std::cout << "[Initial ResMII: " << ResMII << "]" << std::endl;
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        int RecMII = getRecMII(getDFG());
        std::cout << "[Initial RecMII: " << RecMII << "]" << std::endl;
        std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
        II = ResMII > RecMII? ResMII :RecMII;       // II = MAX(RecMII, ResMII)
        MII = II;
    }else{
        if(!preMapCheck(getADG(), getDFG())){
        return false;
        }
        II = 1;
        std::cout << "Pre-map checking passed!\n";
    }
    // check if the DFG can be mapped to the ADG according to the resources
    // new: In temporal mapping, there is no need to preMapCheck
    /*if(!preMapCheck(getADG(), getDFG())){
        return false;
    }
    std::cout << "Pre-map checking passed!\n";*/
    //@Yuan: sort the DFGNodes in ALAP value, mapping order will use ALAP order (just for cycle, insert pass node will has incorrect ALAP)
    sortDfgNodeInPlaceOrder();
    bool succeed = mapper(spatial, isHeteMapping);
    std::cout << "Running time(s): " << runningTimeMS()/1000 << std::endl;
    return succeed;
}


// execute mapping, timing sceduling, visualizing, config getting
// dumpConfig : dump configuration file
// dumpMappedViz : dump mapped visual graph
// resultDir: mapped result directory
bool Mapper::execute(bool dumpConfig, bool dumpMappedViz, std::string resultDir, bool spatial, bool dumpUsage, bool isHeteMapping){
    std::cout << "Start mapping >>>>>>\n";
    bool res = mapperTimed(spatial, isHeteMapping);
    std::cout << "**************\n";
    if(res){
        std::string dir;
        if(!resultDir.empty()){
            dir = resultDir;
        }else{
            dir = "results"; // default directory
        }
        if(dumpMappedViz){
            Graphviz viz(_mapping, dir);
            viz.drawDFG();
            std::cout << "[HETA] Draw mapped DFG successfully!" << std::endl;
            //@Yuan TODO: draw mapped ADG according to II
            //viz.drawADG();
            viz.dumpDFGIO();
            std::cout << "[HETA] Dump mapped DFG IO message successfully!" << std::endl;
        }
        if(dumpConfig){
            Configuration cfg(_mapping);
            std::ofstream ofs(dir + "/config.bit");
            cfg.dumpCfgData(ofs);
            std::cout << "[HETA] Dump config data successfully!" << std::endl;
        }
        // bool dumpUsage = true;
        if(dumpUsage){
        std::string path = "./result/Usage.txt";
        //[HETA]: update the Units usage
        updateUsg(path);
        }
        std::cout << "Succeed to map DFG to ADG!<<<<<<\n" << std::endl;
    } else{
        std::cout << "Fail to map DFG to ADG!<<<<<<\n" << std::endl;
    }
    return res;
}
//new
//calculate the ResMII= (DFGNode nums) / (ADGNode nums)
int Mapper::getResMII(ADG* adg, DFG* dfg){
  //calculate available resources
  //@Yuan: both IOB/GPE/LSU will dominate the ResMII, find the biggest among them
  int availableGPE = adg->numGpeNodes();
  int availableLSU = adg->numLSUNodes();
  int availableIOB = adg->numIOBNodes();
  int numGPE = 0;
  int numIOB = 0;
  int numLSU = 0;
  for(auto elem : dfg->nodes()){
    //std::cout << elem.second->operation() << std::endl;
    if(elem.second->operation() == "INPUT" || elem.second->operation() == "OUTPUT"){
        numIOB += 1 ;
    }else if(elem.second->operation() == "LOAD" || elem.second->operation() == "STORE"){
        numLSU += 1 ;
    }else{
        numGPE += 1 ;
    }
  }
  int ResMII_GPE = ceil(float(numGPE) / availableGPE);
  int ResMII_LSU = ceil(float(numLSU) / availableLSU);
  int ResMII_IOB = ceil(float(numIOB) / availableIOB);
  int ResMII = std::max(ResMII_GPE, ResMII_LSU);
  ResMII = std::max(ResMII, ResMII_IOB);
  std::cout << "Available GPE resources: " << availableGPE << " DFG size (functional nodes): " << numGPE <<" ResMII_GPE: " << ResMII_GPE << std::endl;
  std::cout << "Available LSU resources: " << availableLSU << " DFG size (load/store nodes): " << numLSU <<" ResMII_LSU: " << ResMII_LSU << std::endl;
  std::cout << "Available IOB resources: " << availableIOB << " DFG size (input/output nodes): " << numIOB <<" ResMII_IOB: " << ResMII_IOB << std::endl;
  return ResMII;
}
//new
//calculate the RecMII
//TODO: RecMII = MAX (delay(c) / distance(c))
int Mapper::getRecMII( DFG* dfg){
  float RecMII = 0.0;
  float temp_RecMII = 0.0;
  std::list<std::list<DFGEdge*>*>* cycles = dfg->getCycles();
  std::cout << "[HETA] number of cycles: " << cycles->size() << std::endl;
  for( auto cycle: *cycles) {
    temp_RecMII = float(cycle->size()) / 1.0;
    if(temp_RecMII > RecMII)
      RecMII = temp_RecMII;
  }
  return ceil(RecMII);
}

void Mapper::updateUsg(std::string resultDir){
    ADG* adg = _adg;
    // Mapping* mapping = _mapping;
    // <group idx, usage>
    std::map<int, double> GIBusage;
    std::map<int, int> AddSubUsage;
    std::map<int, int> MulUsage;
    std::map<int, int> CmpUsage;
    std::map<int, int> LogicUsage;
    std::map<int, int> ShiftUsage;
    std::map<int, int> GPEDelayUsage;
    std::map<int, int> LSUDelayUsage;
    // int II = mapping->getII();
    int rows = adg->rows();
    int colums = adg->colums();
    std::ofstream ofs(resultDir);
    for(auto& elem : adg->nodes()){
        if(elem.second->type() == "GIB"){
            GIBNode* gib = (dynamic_cast<GIBNode*>(elem.second));
            gib->updateUti(II);
            int xIdx = gib->x() - 2;
            int yIdx = gib->y()> colums? 1:0;
            int gibIdx = xIdx + yIdx;
            if(GIBusage.count(gibIdx)){
                GIBusage[gibIdx] += gib->getUti();
            }else{
                GIBusage[gibIdx] = gib->getUti();
            }
            // adg->setGIBgroup(gibIdx, elem.second);
            // int connectionUtil = 0;
            // int GIBId = gib->id();
            // auto t_adgNodeAttr = _mapping->t_adgNodeAttr();
            // ADGNodeAttr adgNodeAttr;
            // for(int t = 0; t < II ; t++){
            //     if(t_adgNodeAttr.count(t)){
            //         if(t_adgNodeAttr[t].count(GIBId)){
            //             adgNodeAttr = t_adgNodeAttr[t][GIBId];
            //             connectionUtil += adgNodeAttr.dfgEdgePass.size();
            //         }else{
            //             continue;
            //         }
            //     }else{
            //         continue;
            //     }
            // }
            // // gib->setUti(connectionUtil);
            // std::cout << "node name: " << elem.second->type() << " id: " << elem.second->id() <<" num_ava_connect: " << elem.second->avaConnect()<< std::endl;
            // std::cout << "node name: " << elem.second->type() << " id: " << elem.second->id() <<" x: " << elem.second->x() << " y: "<<elem.second->y() <<" gibIdx: "<<gibIdx<< " connect_util: " <<gib->getUti()<< std::endl;
        }else if (elem.second->type() == "GPE"){
            GPENode* gpe = (dynamic_cast<GPENode*>(elem.second));
            gpe->updateMaxDly();
            gpe->updateOpUsage();
            int xIdx = gpe->y() - 2;
            int yIdx = gpe->x()> (rows+1)? 1:0;
            int gpeIdx = xIdx + yIdx;
            // adg->setGPEgroup(gpeIdx, elem.second);
            auto opset = gpe->opUsage();
            if(AddSubUsage.count(gpeIdx)){
                if(gpe->opCapable("ADD")){// just one operation as idx, since the add and sub are added together
                    AddSubUsage[gpeIdx] += opset[ADDSUB];
                }else{
                    AddSubUsage[gpeIdx] = INT_MAX;
                }                
            }else{
                if(gpe->opCapable("ADD")){
                    AddSubUsage[gpeIdx] = opset[ADDSUB];
                }else{
                    AddSubUsage[gpeIdx] = INT_MAX;
                }   
            }
            if(MulUsage.count(gpeIdx)){
                if(gpe->opCapable("MUL")){
                    MulUsage[gpeIdx] += opset[MUL];
                }else{
                    MulUsage[gpeIdx] = INT_MAX;
                }                
            }else{
                if(gpe->opCapable("MUL")){
                    MulUsage[gpeIdx] = opset[MUL];
                }else{
                    MulUsage[gpeIdx] = INT_MAX;
                }   
            }
            if(CmpUsage.count(gpeIdx)){
                if(gpe->opCapable("EQ")){
                    CmpUsage[gpeIdx] += opset[CMP];
                }else{
                    CmpUsage[gpeIdx] = INT_MAX;
                }                
            }else{
                if(gpe->opCapable("EQ")){
                    CmpUsage[gpeIdx] = opset[CMP];
                }else{
                    CmpUsage[gpeIdx] = INT_MAX;
                }   
            }
            if(LogicUsage.count(gpeIdx)){
                if(gpe->opCapable("AND")){
                    LogicUsage[gpeIdx] += opset[LOGIC];
                }else{
                    LogicUsage[gpeIdx] = INT_MAX;
                }                
            }else{
                if(gpe->opCapable("AND")){
                    LogicUsage[gpeIdx] = opset[LOGIC];
                }else{
                    LogicUsage[gpeIdx] = INT_MAX;
                }   
            }
            if(ShiftUsage.count(gpeIdx)){
                if(gpe->opCapable("SHL")){
                    ShiftUsage[gpeIdx] += opset[SHIF];
                }else{
                    ShiftUsage[gpeIdx] = INT_MAX;
                }                
            }else{
                if(gpe->opCapable("SHL")){
                    ShiftUsage[gpeIdx] = opset[SHIF];
                }else{
                    ShiftUsage[gpeIdx] = INT_MAX;
                }   
            }
            if(GPEDelayUsage.count(gpeIdx)){
                GPEDelayUsage[gpeIdx] += gpe->getMaxDly();
            }else{
                GPEDelayUsage[gpeIdx] == gpe->getMaxDly();
            }
            // std::cout << "node name: " << elem.second->type() << " id: " << elem.second->id() <<" x: " << elem.second->x() << " y: "<<elem.second->y()<<" gpeIdx: "<<gpeIdx<< " delay_usage: " <<gpe->getMaxDly()<< std::endl;
            // std::cout << "using ADD/SUB: " << opset[ADDSUB] << std::endl;
            // std::cout << "using MUL: " << opset[MUL]<< std::endl;
            // std::cout << "using CMP: " << opset[CMP]<< std::endl;
            // std::cout << "using LOGIC: " << opset[LOGIC]<< std::endl;
            // std::cout << "using SHIF: " << opset[SHIF]<< std::endl;
        }else if (elem.second->type() == "LSU"){
            LSUNode* lsu = (dynamic_cast<LSUNode*>(elem.second));
            lsu->updateMaxDly();
            int yIdx = lsu->y()==0? 0 : 2;
            int xIdx = lsu->x()>(rows + 1)? 1:0;
            int lsuIdx = yIdx + xIdx;
            if(LSUDelayUsage.count(lsuIdx)){
                LSUDelayUsage[lsuIdx] += lsu->getMaxDly();
            }else{
                LSUDelayUsage[lsuIdx] = lsu->getMaxDly();
            }
            // adg->setGPEgroup(lsuIdx, elem.second);
            // std::cout << "node name: " << elem.second->type() << " id: " << elem.second->id() <<" x: " << elem.second->x() << " y: "<<elem.second->y()<<" lsuIdx: "<<lsuIdx<< " delay_usage: " <<lsu->getMaxDly()<< std::endl;
        }      
        
    }
    std::vector<int>GIBSortedIdx, GIBSumUsage;
    // double minGIBUsage = INT_MAX;
    // int minGIBGroupIdx = 0;
    for(auto& elem : GIBusage){
        // std::cout << "GIBGIdx: " << elem.first << " Usage: " << elem.second << std::endl;
        GIBSortedIdx.push_back(elem.first);
        GIBSumUsage.push_back(elem.second);
        // if(elem.second < minGIBUsage){
        //     minGIBGroupIdx = elem.first;
        //     minGIBUsage = elem.second;
        // }
    }
    std::sort(GIBSortedIdx.begin(), GIBSortedIdx.end(), [&GIBSumUsage](int a, int b){
        return GIBSumUsage[a] < GIBSumUsage[b];
    });
    ofs << "GIB: ";
    for(auto& elem:GIBSortedIdx){
        // std::cout<< "GIB Usage order: " << elem << std::endl;
        ofs << elem << " ";
    }
    ofs << "\n";
    // std::cout << "minGIBGroupIdx: " << minGIBGroupIdx << " minGIBUsage: " << minGIBUsage<<std::endl;
    // int minAddSubUsage = INT_MAX;
    // int minAddSubGroupIdx = 0;
    std::vector<int>AddSubSortedIdx, AddSubSumUsage;
    for(auto& elem : AddSubUsage){
        // std::cout << "AddSubGIdx: " << elem.first << " Usage: " << elem.second << std::endl;
        AddSubSortedIdx.push_back(elem.first);
        AddSubSumUsage.push_back(elem.second);
        // if(elem.second < minAddSubUsage){
        //     minAddSubGroupIdx = elem.first;
        //     minAddSubUsage = elem.second;
        // }
    }
    std::sort(AddSubSortedIdx.begin(), AddSubSortedIdx.end(), [&AddSubSumUsage](int a, int b){
        return AddSubSumUsage[a] < AddSubSumUsage[b];
    });
    bool isUsed = false;
    for(auto & elem : AddSubSumUsage){
        if(elem != INT_MAX && elem != 0){
            isUsed =true;
            break;
        }
        // std::cout << "LogicSumUsage: " << elem << std::endl;
    }
    // int AddSubMaxUsage = AddSubSumUsage[*AddSubSortedIdx.rbegin()];
    // int AddSubMinUsage = AddSubSumUsage[*AddSubSortedIdx.begin()];
    // std::cout << "idx: " <<*AddSubSortedIdx.rbegin()<<" addsub max: " << AddSubMaxUsage <<" add min: " <<AddSubMinUsage<< std::endl;
    ofs << "ADDSUB: ";
    if(!isUsed){
        ofs << "-1";
    }else{
        for(auto& elem:AddSubSortedIdx){
            ofs << elem << " ";
        }
    }
    ofs << "\n";
    // std::cout << "minAddSubGroupIdx: " << minAddSubGroupIdx << " minAddSubUsage: " << minAddSubUsage<<std::endl;
    // int minMulUsage = INT_MAX;
    // int minMUlGroupIdx = 0;
    std::vector<int>MulSortedIdx, MulSumUsage;
    for(auto& elem : MulUsage){
        // std::cout << "MUlGIdx: " << elem.first << " Usage: " << elem.second << std::endl;
        MulSortedIdx.push_back(elem.first);
        MulSumUsage.push_back(elem.second);
        // if(elem.second < minMulUsage){
        //     minMUlGroupIdx = elem.first;
        //     minMulUsage = elem.second;
        // }
    }
    std::sort(MulSortedIdx.begin(), MulSortedIdx.end(), [&MulSumUsage](int a, int b){
        return MulSumUsage[a] < MulSumUsage[b];
    });
    isUsed = false;
    for(auto & elem : MulSumUsage){
        if(elem != INT_MAX && elem != 0){
            isUsed =true;
            break;
        }
        // std::cout << "LogicSumUsage: " << elem << std::endl;
    }
    // int MulMaxUsage = MulSumUsage[*MulSortedIdx.rbegin()];
    // int MulMinUsage = MulSumUsage[*MulSortedIdx.begin()];
    // std::cout << "idx: " <<*MulSortedIdx.rbegin()<<" mul max: " << MulMaxUsage <<" mul min: "<<MulMinUsage<< std::endl;
    ofs << "MUL: ";
    if(!isUsed){
        ofs << "-1";
    }else{
        for(auto& elem:MulSortedIdx){
            ofs << elem << " ";
        }
    }
    ofs << "\n";
    // std::cout << "minMUlGroupIdx: " << minMUlGroupIdx << " minMulUsage: " << minMulUsage<<std::endl;
    // int minCmpUsage = INT_MAX;
    // int minCmpGroupIdx = 0;
    std::vector<int>CmpSortedIdx, CmpSumUsage;
    for(auto& elem : CmpUsage){
        CmpSortedIdx.push_back(elem.first);
        CmpSumUsage.push_back(elem.second);
        // if(elem.second < minCmpUsage){
        //     minCmpGroupIdx = elem.first;
        //     minCmpUsage = elem.second;
        // }
    }
    std::sort(CmpSortedIdx.begin(), CmpSortedIdx.end(), [&CmpSumUsage](int a, int b){
        return CmpSumUsage[a] < CmpSumUsage[b];
    });
    isUsed = false;
    for(auto & elem : CmpSumUsage){
        if(elem != INT_MAX && elem != 0){
            isUsed =true;
            break;
        }
        // std::cout << "LogicSumUsage: " << elem << std::endl;
    }
    // int CmpMaxUsage = CmpSumUsage[*CmpSortedIdx.rbegin()];
    // int CmpMinUsage = CmpSumUsage[*CmpSortedIdx.begin()];
    // std::cout << "idx: " <<*CmpSortedIdx.rbegin()<<" cmp max: " << CmpMaxUsage <<" cmp min: "<<CmpMinUsage<< std::endl;
    ofs << "CMP: ";
    if(!isUsed){
        ofs << "-1";
    }else{
        for(auto& elem:CmpSortedIdx){
            ofs << elem << " ";
        }
    }
    ofs << "\n";
    // std::cout << "minCmpGroupIdx: " << minCmpGroupIdx << " minCmpUsage: " << minCmpUsage<<std::endl;
    // int minLogicUsage = INT_MAX;
    // int minLogicGroupIdx = 0;
    std::vector<int>LogicSortedIdx, LogicSumUsage;
    // std::cout << "logicusage size : " << LogicUsage.size() << std::endl;
    for(auto& elem : LogicUsage){
        LogicSortedIdx.push_back(elem.first);
        LogicSumUsage.push_back(elem.second);
        // if(elem.second < minLogicUsage){
        //     minLogicGroupIdx = elem.first;
        //     minLogicUsage = elem.second;
        // }
    }
    std::sort(LogicSortedIdx.begin(), LogicSortedIdx.end(), [&LogicSumUsage](int a, int b){
        return LogicSumUsage[a] < LogicSumUsage[b];
    });
    isUsed = false;
    for(auto & elem : LogicSumUsage){
        if(elem != INT_MAX && elem != 0){
            isUsed =true;
            break;
        }
        // std::cout << "LogicSumUsage: " << elem << std::endl;
    }
    // int LogicMaxUsage = LogicSumUsage[*LogicSortedIdx.rbegin()];
    // int LogicMinUsage = LogicSumUsage[*LogicSortedIdx.begin()];
    // std::cout << "idx: " <<*LogicSortedIdx.rbegin()<<" logic max: " << LogicMaxUsage <<" logc min: "<<LogicMinUsage<< std::endl;
    ofs << "LOGIC: ";
    if(!isUsed){
        ofs << "-1";
    }else{
        for(auto& elem:LogicSortedIdx){
            ofs << elem << " ";
        }
    }
    ofs << "\n";
    // std::cout << "minLogicGroupIdx: " << minLogicGroupIdx << " minLogicUsage: " << minLogicUsage<<std::endl;
    // int minShiftUsage = INT_MAX;
    // int minShiftGroupIdx = 0;
    std::vector<int>ShiftSortedIdx, ShiftSumUsage;
    for(auto& elem : ShiftUsage){
        ShiftSortedIdx.push_back(elem.first);
        ShiftSumUsage.push_back(elem.second);
        // if(elem.second < minShiftUsage){
        //     minShiftGroupIdx = elem.first;
        //     minShiftUsage = elem.second;
        // }
    }
    std::sort(ShiftSortedIdx.begin(), ShiftSortedIdx.end(), [&ShiftSumUsage](int a, int b){
        return ShiftSumUsage[a] < ShiftSumUsage[b];
    });
    isUsed = false;
    for(auto & elem : ShiftSumUsage){
        if(elem != INT_MAX && elem != 0){
            isUsed =true;
            break;
        }
        // std::cout << "LogicSumUsage: " << elem << std::endl;
    }
    // int ShiftMaxUsage = ShiftSumUsage[*ShiftSortedIdx.rbegin()];
    // int ShiftMinUsage = ShiftSumUsage[*ShiftSortedIdx.begin()];
    // std::cout << "idx: " <<*ShiftSortedIdx.rbegin()<<" shift max: " << ShiftMaxUsage <<" shift min: "<<ShiftMinUsage<< std::endl;
    ofs << "SHIFT: ";
    if(!isUsed){
        ofs << "-1";
    }else{
        for(auto& elem:ShiftSortedIdx){
            ofs << elem << " ";
        }
    }
    ofs << "\n";
    // std::cout << "minShiftGroupIdx: " << minShiftGroupIdx << " minShiftUsage: " << minShiftUsage<<std::endl;
    // int minGPEDlyUsage = INT_MAX;
    // int minGPEDlyGroupIdx = 0;
    std::vector<int>GPEDlySortedIdx, GPEDlySumUsage;
    for(auto& elem : GPEDelayUsage){
        GPEDlySortedIdx.push_back(elem.first);
        GPEDlySumUsage.push_back(elem.second);
        // if(elem.second < minGPEDlyUsage){
        //     minGPEDlyGroupIdx = elem.first;
        //     minGPEDlyUsage = elem.second;
        // }
    }
    std::sort(GPEDlySortedIdx.begin(), GPEDlySortedIdx.end(), [&GPEDlySumUsage](int a, int b){
        return GPEDlySumUsage[a] < GPEDlySumUsage[b];
    });
    ofs << "GPEDly: ";
    for(auto& elem:GPEDlySortedIdx){
        ofs << elem << " ";
    }
    ofs << "\n";
    // std::cout << "minGPEDlyGroupIdx: " << minGPEDlyGroupIdx << " minGPEDlyUsage: " << minGPEDlyUsage<<std::endl;
    // int minLSUDlyUsage = INT_MAX;
    // int minLSUDlyGroupIdx = 0;
    std::vector<int>LSUDlySortedIdx, LSUDlySumUsage;
    for(auto& elem : LSUDelayUsage){
        LSUDlySortedIdx.push_back(elem.first);
        LSUDlySumUsage.push_back(elem.second);
        // if(elem.second < minLSUDlyUsage){
        //     minLSUDlyGroupIdx = elem.first;
        //     minLSUDlyUsage = elem.second;
        // }
    }
    std::sort(LSUDlySortedIdx.begin(), LSUDlySortedIdx.end(), [&LSUDlySumUsage](int a, int b){
        return LSUDlySumUsage[a] < LSUDlySumUsage[b];
    });
    ofs << "LSUDly: ";
    for(auto& elem:LSUDlySortedIdx){
        ofs << elem << " ";
    }
    ofs << "\n";
    // std::cout << "minLSUDlyGroupIdx: " << minGPEDlyGroupIdx << " minLSUDlyUsage: " << minLSUDlyUsage<<std::endl;

    // std::cout << "II: " << II << std::endl;
}



// void Mapper::updateUsg(){
//     ADG* adg = _adg;
//     Mapping* mapping = _mapping;
//     int II = mapping->getII();
//     for(auto& elem : adg->nodes()){
//         if(elem.second->type() == "GIB"){
//             GIBNode* gib = (dynamic_cast<GIBNode*>(elem.second));
//             int connectionUtil = 0;
//             int GIBId = gib->id();
//             auto t_adgNodeAttr = _mapping->t_adgNodeAttr();
//             ADGNodeAttr adgNodeAttr;
//             for(int t = 0; t < II ; t++){
//                 if(t_adgNodeAttr.count(t)){
//                     if(t_adgNodeAttr[t].count(GIBId)){
//                         adgNodeAttr = t_adgNodeAttr[t][GIBId];
//                         connectionUtil += adgNodeAttr.dfgEdgePass.size();
//                     }else{
//                         continue;
//                     }
//                 }else{
//                     continue;
//                 }
//             }
//             gib->setUti(connectionUtil);
//             std::cout << "node name: " << elem.second->type() << " id: " << elem.second->id() << " num_used_connect: " <<connectionUtil<<" num_ava_connect: " << elem.second->avaConnect()<< std::endl;
//         }else if (elem.second->type() == "GPE"){
//             GPENode* gpe = (dynamic_cast<GPENode*>(elem.second));
//             int GPEId = gpe->id();
//             auto t_adgNodeAttr = _mapping->t_adgNodeAttr();
//             ADGNodeAttr adgNodeAttr;
//             for(int t = 0; t < II ; t++){
//                 if(t_adgNodeAttr.count(t)){
//                     if(t_adgNodeAttr[t].count(GPEId)){
//                         adgNodeAttr = t_adgNodeAttr[t][GPEId];
//                         connectionUtil += adgNodeAttr.dfgEdgePass.size();
//                     }else{
//                         continue;
//                     }
//                 }else{
//                     continue;
//                 }
//             }
//             std::cout << "node name: " << elem.second->type() << " id: " << elem.second->id() << std::endl;
//         }else if(elem.second->type() == "LSU"){

//         }
        
//     }
//     // std::cout << "II: " << II << std::endl;
// }
