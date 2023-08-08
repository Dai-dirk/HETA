
#include "mapper/mapping.h"
#include <stack>
#include <climits>

// reset mapping intermediate result and status
void Mapping::reset(){
    _dfgNodeAttr.clear();
    _dfgEdgeAttr.clear();
    _t_adgNodeAttr.clear();
    _vioDfgEdges.clear();
    _ALAPTime.clear();
    // _adgLinkAttr.clear();
    _totalViolation = 0;
    _numNodeMapped = 0;
    // clearADGNodeUsage();
}


// if this input port of this ADG node is used
/*bool Mapping::isAdgNodeInPortUsed(int nodeId, int portIdx){
    if(_adgNodeAttr.count(nodeId)){
        auto& status = _adgNodeAttr[nodeId].inPortUsed;
        if(status.count(portIdx)){
            return status[portIdx];
        }
    }
    return false;
}


// if this output port of this ADG node is used
bool Mapping::isAdgNodeOutPortUsed(int nodeId, int portIdx){
    if(_adgNodeAttr.count(nodeId)){
        auto& status = _adgNodeAttr[nodeId].outPortUsed;
        if(status.count(portIdx)){
            return status[portIdx];
        }
    }
    return false;
}*/
//@Yuan: check in given time
// if this output port of this ADG node is used
bool Mapping::isAdgNodeOutPortUsed(int time, int nodeId, int portIdx){
    if(_t_adgNodeAttr.count(time)){
        auto& status = _t_adgNodeAttr[time][nodeId].outPortUsed;
        if(status.count(portIdx)){
            return status[portIdx];
        }
    }
    return false;
}

// if this input port of this ADG node is used
bool Mapping::isAdgNodeInPortUsed(int time, int nodeId, int portIdx){
    if(_t_adgNodeAttr.count(time)){
        auto& status = _t_adgNodeAttr[time][nodeId].inPortUsed;
        if(status.count(portIdx)){
            return status[portIdx];
        }
    }
    return false;
}
//@Yuan: for GPE, the ALU port that connect to input port can not be reused
// if this input port of this ADG node is used
bool Mapping::isAdgNodeInPortUsed(int time, int nodeId, int dstNodeId, int portIdx){
    auto ADGNode = _adg->node(nodeId);
    auto dstNode = _dfg->node(dstNodeId);
    for(auto elem : _dfgNodeAttr[dstNodeId].srcPort){
        if(ADGNode->type() == "GPE"){
            GPENode * gpe = dynamic_cast<GPENode*>(ADGNode);
            if(gpe->getOperandIdx(elem) == gpe->getOperandIdx(portIdx)){
                return true;
            }
            }else{
                LSUNode * lsu = dynamic_cast<LSUNode*>(ADGNode);
                if(lsu->getOperandIdx(elem) == lsu->getOperandIdx(portIdx)){
                    return true;
                }
            }
    }
    if(_t_adgNodeAttr.count(time)){
        auto& status = _t_adgNodeAttr[time][nodeId].inPortUsed;
        if(status.count(portIdx)){
            return status[portIdx];
        }
    }
    return false;
}
// if the DFG node is already mapped
bool Mapping::isMapped(DFGNode* dfgNode){
    if(_dfgNodeAttr.count(dfgNode->id())){
        return _dfgNodeAttr[dfgNode->id()].adgNode != nullptr;
    }
    return false;

}

// if the ADG node is already mapped
/*bool Mapping::isMapped(ADGNode* adgNode){
    if(_adgNodeAttr.count(adgNode->id())){
        return _adgNodeAttr[adgNode->id()].dfgNode[0]!= nullptr;
    }
    return false;
}

// if the IB node is available
bool Mapping::isIBAvail(ADGNode* adgNode){
    assert(adgNode->type() == "IB");
    int id = adgNode->id();
    if(_adgNodeAttr.count(id)){
        auto& inPortUsed = _adgNodeAttr[id].inPortUsed;
        for(int i = 0; i < adgNode->numInputs(); i++){
            if(!inPortUsed.count(i) || !inPortUsed[i]){
                return true;
            }
        }
        return false;
    }
    return true;
}

// if the OB node is available
bool Mapping::isOBAvail(ADGNode* adgNode){
    assert(adgNode->type() == "OB");
    int id = adgNode->id();
    if(_adgNodeAttr.count(id)){
        auto& outPortUsed = _adgNodeAttr[id].outPortUsed;
        for(int i = 0; i < adgNode->numOutputs(); i++){
            if(!outPortUsed.count(i) || !outPortUsed[i]){
                return true;
            }
        }
        return false;
    }
    return true;
}

// if the DFG input port is mapped
bool Mapping::isDfgInputMapped(int idx){
    if(_dfgInputAttr.count(idx) && !_dfgInputAttr[idx].routedEdgeIds.empty()){
        return true;
    }
    return false;
}

// if the DFG output port is mapped
bool Mapping::isDfgOutputMapped(int idx){
    if(_dfgOutputAttr.count(idx) && !_dfgOutputAttr[idx].routedEdgeIds.empty()){
        return true;
    }
    return false;
}*/

// occupied ADG node of this DFG node
ADGNode* Mapping::mappedNode(DFGNode* dfgNode){
    if(!_dfgNodeAttr.count(dfgNode->id())){
        return nullptr;
    }
    return _dfgNodeAttr[dfgNode->id()].adgNode;
}


// mapped DFG node of this ADG node
/*DFGNode* Mapping::mappedNode(ADGNode* adgNode){
    return _adgNodeAttr[adgNode->id()].dfgNode[0];
}*/

//@Yuan: reuse the delay unit
// pipeline map DFG node to ADG node, not assign ports
// check if there is constant operand
bool Mapping::mapDfgNode(DFGNode* dfgNode, ADGNode* adgNode, int mappingTime, bool isExecute, int delayUsage, int startTime){
    int dfgNodeId = dfgNode->id();
    int adgNodeId = adgNode->id();
    //@Yuan: for the dfg has self-cycle, there may cause mappingTime < 0, here we should make it back to 0 ~ II - 1
    int Time =  mappingTime < 0 ? mappingTime + II : mappingTime;
    if(_dfgNodeAttr.count(dfgNodeId)){//dfgNode is already mapped before
        if(_dfgNodeAttr[dfgNodeId].adgNode != nullptr && _dfgNodeAttr[dfgNodeId].adgNode != adgNode){
            return false;
        }
    }

    if(isExecute && _t_adgNodeAttr[Time % II][adgNodeId].executeNode != dfgNode && _t_adgNodeAttr[Time % II][adgNodeId].executeNode != nullptr){//one ADGNode can not be mapped at 2 node's excute time
        return false;
    }

    for(auto mappedDFGNode: _t_adgNodeAttr[Time % II][adgNodeId].dfgNode){
        if(mappedDFGNode == dfgNode){
            /*std::cout << "[HETA] set DFGNode " << dfgNode->name() << " at ADGNode " << adgNode->type() << "(" << adgNode->id() << ") at time: "\
            << mappingTime << " with II = " << II << std::endl;*/
            _dfgNodeAttr[dfgNodeId].mappedtime.emplace(mappingTime);
            if(isExecute && _t_adgNodeAttr[Time % II][adgNodeId].executeNode == nullptr){
                _t_adgNodeAttr[Time % II][adgNodeId].executeNode = dfgNode;
            }
            if(mappingTime + dfgNode->opLatency() > maxLat()){
                _maxLat = mappingTime + dfgNode->opLatency();
            }
            return true;
        }else if(mappedDFGNode != nullptr && mappedDFGNode != dfgNode){
            if(_dfgNodeAttr[mappedDFGNode->id()].DelayUsage != delayUsage){    //mapping at same ADGNode at same time, these dfgNodes's delayusage must be same
                return false;
            }
            //check if dfgNodes have same ALU input that using delay unit
            int delayEdgeId = 0;
            int inputPort = INT_MAX;
            for(auto inputEdges : mappedDFGNode->inputEdges()){
                if(_dfgEdgeAttr[inputEdges.second].endTime == *(_dfgNodeAttr[mappedDFGNode->id()].mappedtime.begin())){
                    delayEdgeId = inputEdges.second;
                    inputPort = (_dfgEdgeAttr[delayEdgeId].edgeLinks.rbegin())->srcPort;
                    break;
                }
            }
            int oprand = 0;
            int curDelayEdgeId = 0;
            int curinPort = INT_MAX;
            for(auto inputEdges : dfgNode->inputEdges()){
                if(_dfgEdgeAttr[inputEdges.second].endTime == startTime){
                    curDelayEdgeId = inputEdges.second;
                    curinPort = (_dfgEdgeAttr[curDelayEdgeId].edgeLinks.rbegin())->srcPort;
                    break;
                }

            }
            if(adgNode->type() == "GPE"){
                GPENode * gpe = dynamic_cast<GPENode*>(adgNode);
                if(gpe->getOperandIdx(inputPort) != gpe->getOperandIdx(curinPort)){
                    return false;
                }
            }else{
                LSUNode * lsu = dynamic_cast<LSUNode*>(adgNode);
                if(lsu->getOperandIdx(inputPort) != lsu->getOperandIdx(curinPort)){
                    return false;
                }
            }
        }
    }

    ADGNodeAttr adgAttr =  _t_adgNodeAttr[Time % II][adgNodeId];
  // one operand is immediate and the operands are not commutative
  // if the operands are commutative, there is no need to set all the input ports connected to this operand are used
  // just find one available port when generate config data
  // only execute node need to occupied the input ports

    if(dfgNode->hasImm() && isExecute){ // assign constant port //new
        if(dfgNode->operation() == "LOAD" || dfgNode->operation() == "STORE"){
            LSUNode* lsuNode = dynamic_cast<LSUNode*>(adgNode);
            std::set<int> operandUsed;
            for( auto edge : dfgNode->inputEdges()){
                  int inPortused = (_dfgEdgeAttr[edge.second].edgeLinks.rbegin())->srcPort;
                  operandUsed.emplace(lsuNode->getOperandIdx(inPortused));
              }
            for(auto inPort : lsuNode->inputs()){
                if(operandUsed.empty())
                    break;
                if(operandUsed.count(lsuNode->getOperandIdx(inPort.first))){
                    continue;
                }
                if(adgAttr.inPortUsed.count(inPort.first) && adgAttr.inPortUsed[inPort.first] == true){ // already used
                  return false;
                }
            adgAttr.inPortUsed[inPort.first] = true; // set all the input ports connetced to this operand as used
            }
        }else{
              GPENode* gpeNode = dynamic_cast<GPENode*>(adgNode);
              std::set<int> operandUsed;
              for( auto edge : dfgNode->inputEdges()){
                  if(_dfgEdgeAttr.count(edge.second)){
                    int inPortused = (_dfgEdgeAttr[edge.second].edgeLinks.rbegin())->srcPort;
                    operandUsed.emplace(gpeNode->getOperandIdx(inPortused));
                  }
              }
              for(auto inPort : gpeNode->inputs()){
                if(operandUsed.empty())
                    break;
                if(operandUsed.count(gpeNode->getOperandIdx(inPort.first))){
                    continue;
                }
                if(adgAttr.inPortUsed.count(inPort.first) && adgAttr.inPortUsed[inPort.first] == true){ // already used
                  return false;
                }
            adgAttr.inPortUsed[inPort.first] = true; // set all the input ports connetced to this operand as used
            }
        }
    }
   // as same as const, if the operands are not commutative, set the input ports are used (the ports connect to the operand that self-cycle connect to)
    if(dfgNode->selfloop() != -1 && isExecute){
        if(dfgNode->operation() != "LOAD" && dfgNode->operation() != "STORE"){
            GPENode* gpeNode = dynamic_cast<GPENode*>(adgNode);
            for(auto edges : dfgNode->inputEdges()){
                DFGEdge* edge = _dfg->edge(edges.second);
                if(!edge->isCycle())
                    continue;
                if(_dfgEdgeAttr.count(edges.second)){
                    int operandUsed = gpeNode->getOperandIdx((_dfgEdgeAttr[edges.second].edgeLinks.rbegin())->srcPort);
                    for(auto inPort : gpeNode->operandInputs(operandUsed)){
                        if(adgAttr.inPortUsed.count(inPort) && adgAttr.inPortUsed[inPort] == true){ // already used
                            return false;
                        }
                        adgAttr.inPortUsed[inPort] = true; // set all the input ports connetced to this operand as used
                    }
                }
            }
        }
    }

    adgAttr.dfgNode.push_back(dfgNode);
    if(_dfgNodeAttr[dfgNodeId].adgNode != nullptr && _dfgNodeAttr[dfgNodeId].adgNode == adgNode){
        _dfgNodeAttr[dfgNodeId].mappedtime.emplace(mappingTime);
    }else{
        DFGNodeAttr dfgAttr;
        dfgAttr.adgNode = adgNode;
        dfgAttr.mappedtime.emplace(mappingTime);   //@Yuan: record the initial mapping time
        _dfgNodeAttr[dfgNodeId] = dfgAttr;
        _numNodeMapped++;
        adgNode->addUsage(dfgNodeId);
    }
    if(isExecute){
        adgAttr.executeNode = dfgNode;
    }
    _t_adgNodeAttr[Time % II][adgNodeId] = adgAttr;
    if(mappingTime + dfgNode->opLatency() > maxLat()){
        _maxLat = mappingTime + dfgNode->opLatency();
    }
    /*std::cout << "[HETA] set DFGNode " << dfgNode->name() << " at ADGNode " << adgNode->type() << "(" << adgNode->id() << ") at time: "\
    << mappingTime << " with II = " << II << std::endl;*/
    return true;
}
//@Yuan: no pipeline map dfg nodes
/*bool Mapping::mapDfgNode(DFGNode* dfgNode, ADGNode* adgNode, int mappingTime, bool isExecute){
    int dfgNodeId = dfgNode->id();
    int test = 0;
    if(_dfgNodeAttr.count(dfgNodeId)){//dfgNode is already mapped before
        if(_dfgNodeAttr[dfgNodeId].adgNode != nullptr && _dfgNodeAttr[dfgNodeId].adgNode != adgNode){
            return false;
        }
    }

    if(isExecute && _t_adgNodeAttr[abs(mappingTime%II)][adgNode->id()].executeNode != dfgNode && _t_adgNodeAttr[abs(mappingTime%II)][adgNode->id()].executeNode != nullptr){//one ADGNode can not be mapped at 2 node's excute time
        return false;
    }

    for(auto mappedDFGNode: _t_adgNodeAttr[abs(mappingTime%II)][adgNode->id()].dfgNode){
        if(mappedDFGNode == dfgNode){
            std::cout << "[HETA] set DFGNode " << dfgNode->name() << " at ADGNode " << adgNode->type() << "(" << adgNode->id() << ") at time: "\
            << mappingTime << " with II = " << II << std::endl;
            _dfgNodeAttr[dfgNode->id()].mappedtime.emplace(mappingTime);
            if(isExecute && _t_adgNodeAttr[abs(mappingTime%II)][adgNode->id()].executeNode == nullptr){
                _t_adgNodeAttr[abs(mappingTime%II)][adgNode->id()].executeNode = dfgNode;
            }
            return true;
        }else if(mappedDFGNode != nullptr && mappedDFGNode != dfgNode){//the adgNode is used by other dfgNodes
            return false;
        }
    }

    //@Yuan: check whether configmem is available
    /*bool canMapped = true;
    int maxConfigmem = adgNode->MaxConfigMem();
    if(_adgNodeUsageAttr.count(adgNode->id())){
        if(_adgNodeUsageAttr[adgNode->id()]._currentConfigMem + 1 > maxConfigmem){
            canMapped = false;
        }
    }

    if(!canMapped){
        std::cout << "mapping237 fail due to configmem limitation!" << std::endl;
        return false;
    }*/

    /*ADGNodeAttr adgAttr =  _t_adgNodeAttr[abs(mappingTime%II)][adgNode->id()];
  // one operand is immediate and the operands are not commutative
    if(dfgNode->hasImm() && !dfgNode->commutative()){ // assign constant port //new
        if(dfgNode->operation() == "LOAD" || dfgNode->operation() == "STORE"){
           LSUNode* lsuNode = dynamic_cast<LSUNode*>(adgNode);
        for(auto inPort : lsuNode->operandInputs(dfgNode->immIdx())){
            if(adgAttr.inPortUsed.count(inPort) && adgAttr.inPortUsed[inPort] == true){ // already used
                return false;
            }
            adgAttr.inPortUsed[inPort] = true; // set all the input ports connetced to this operand as used
        }
        }else{
              GPENode* gpeNode = dynamic_cast<GPENode*>(adgNode);
              for(auto inPort : gpeNode->operandInputs(dfgNode->immIdx())){
              if(adgAttr.inPortUsed.count(inPort) && adgAttr.inPortUsed[inPort] == true){ // already used
                  return false;
                }
            adgAttr.inPortUsed[inPort] = true; // set all the input ports connetced to this operand as used
            }
        }
    }
    adgAttr.dfgNode.push_back(dfgNode);
    if(_dfgNodeAttr[dfgNodeId].adgNode != nullptr && _dfgNodeAttr[dfgNodeId].adgNode == adgNode){
        _dfgNodeAttr[dfgNodeId].mappedtime.emplace(mappingTime);
    }else{
        DFGNodeAttr dfgAttr;
        dfgAttr.adgNode = adgNode;
        dfgAttr.mappedtime.emplace(mappingTime);   //@Yuan: record the initial mapping time
        _dfgNodeAttr[dfgNodeId] = dfgAttr;
        _numNodeMapped++;
    }
    if(isExecute){
        adgAttr.executeNode = dfgNode;
    }
    _adgNodeAttr[adgNode->id()] = adgAttr;
    _t_adgNodeAttr[abs(mappingTime%II)][adgNode->id()] = adgAttr;
    ADGNodeUsageAttr adgusageattr;
    adgusageattr._currentConfigMem = adgusageattr._currentConfigMem + 1;
    _adgNodeUsageAttr[adgNode->id()] = adgusageattr;
    //@Yuan: set the ADGNode is used and record the corresponding DFGNode
    //setOccupied(mappingTime, adgNode, dfgNode);
    std::cout << "[HETA] set DFGNode " << dfgNode->name() << " at ADGNode " << adgNode->type() << "(" << adgNode->id() << ") at time: "\
    << mappingTime << " with II = " << II << std::endl;
    std::cout << "mapping 364 _numNodemapped: " << _numNodeMapped << std::endl;
    /*if(test == 1 ){
        std::cout << "map: " ;
        for(int t =0; t<3 ; t++ ){
            std::cout << "\n";
        }
        std::cout << std::endl;
    }*/

    /*return true;
}*/


// map DFG node to ADG node, not assign ports
bool Mapping::mapOutDfgNode(DFGNode* dfgNode, ADGNode* adgNode, int mappingTime){
    int dfgNodeId = dfgNode->id();
    int adgNodeId = adgNode->id();
    if(_dfgNodeAttr.count(dfgNodeId)){
        if(_dfgNodeAttr[dfgNodeId].adgNode == adgNode){
            return true;
        } else if(_dfgNodeAttr[dfgNodeId].adgNode != nullptr){
            return false;
        }
    }
    //check whether the inport is available
    int inport = *(_dfgNodeAttr[dfgNodeId].srcPort.begin());
    DFGNodeAttr dfgAttr;
    dfgAttr.adgNode = adgNode;
    dfgAttr.mappedtime.emplace(mappingTime);   //@Yuan: record the initial mapping time
    ADGNodeAttr adgAttr;
    if(_t_adgNodeAttr[mappingTime%II].count(adgNodeId)){ //@Yuan: in OB, it may be mapped before
        adgAttr = _t_adgNodeAttr[mappingTime%II][adgNodeId];
    }
    adgAttr.dfgNode.push_back(dfgNode); //@Yuan: in some situation, an OB can map more than one output
    //find a valid inport-outport pair
    int outports = -1;
    for(auto outport: adgNode->in2outs(inport)){
    if (!isAdgNodeOutPortUsed(mappingTime%II,adgNodeId,outport)){ // find one available outport
           outports = outport ;
           break;
        }
    }
    if(outports == -1){
           return false;
    }
       adgAttr.inPortUsed[inport] = true;                              // a valid inport-outport pair
       adgAttr.outPortUsed[outports] = true;
       dfgAttr.srcPort.emplace(inport);
       dfgAttr.dstPort = outports;
       //attach the ADG IO
       dfgAttr.adgIOPort= adgNode->output(outports).begin()->second;               // the Output DFGNode is mapped to the corresponding ADG Output

    _dfgNodeAttr[dfgNodeId] = dfgAttr;
    _t_adgNodeAttr[mappingTime%II][adgNodeId] = adgAttr;
    //@Yuan: set the ADGNode is used and record the corresponding DFGNode
    //setOccupied(mappingTime, adgNode, dfgNode);
    /*ADGNodeUsageAttr adgusageattr;
    adgusageattr._currentConfigMem = adgusageattr._currentConfigMem + 1;
    _adgNodeUsageAttr[adgNode->id()] = adgusageattr;*/
    /*std::cout << "[HETA] set DFGNode " << dfgNode->name() << " at ADGNode " << adgNode->type() << "(" << adgNode->id() << ") at time: "\
    << mappingTime << " with II = " << II << std::endl;*/
    if(mappingTime + dfgNode->opLatency()> maxLat()){
        _maxLat = mappingTime + dfgNode->opLatency();
    }
    _numNodeMapped++;
    adgNode->addUsage(dfgNodeId);
    return true;
}

//@Yuan:
// map DFG INPUT node to ADG node
//@Yuan: isTry: is try to route, do not change the routing status
bool Mapping::mapInDfgNode(DFGNode* dfgNode, ADGNode* adgNode, int mappingTime, int srcPort, int dstPort, int adgIOPort, bool isTry){
    int dfgNodeId = dfgNode->id();
    int adgNodeId = adgNode->id();
    if(_dfgNodeAttr.count(dfgNodeId)){
        if(_dfgNodeAttr[dfgNodeId].adgNode == adgNode){
            return true;
        } else if(_dfgNodeAttr[dfgNodeId].adgNode != nullptr){
            return false;
        }
    }
    if(isTry){
        return true;
    }
    DFGNodeAttr dfgAttr;
    dfgAttr.adgNode = adgNode;
    dfgAttr.mappedtime.emplace(mappingTime);   //@Yuan: record the initial mapping time
    ADGNodeAttr adgAttr;
    if(_t_adgNodeAttr[mappingTime%II].count(adgNodeId)){ //@Yuan: in IB, it may be mapped before
        adgAttr = _t_adgNodeAttr[mappingTime%II][adgNodeId];
    }
    adgAttr.dfgNode.push_back(dfgNode); //@Yuan: in some situation, an IB can map more than one input
    adgAttr.inPortUsed[srcPort] = true;                              // a valid inport-outport pair
    adgAttr.outPortUsed[dstPort] = true;
    //adgAttr.inoutPortused[dfgNode->id()]=std::make_pair(inport, outport); // mark the pair is used
    dfgAttr.srcPort.emplace(srcPort);
    dfgAttr.dstPort = dstPort;
    dfgAttr.adgIOPort= adgIOPort;               // the Input DFGNode is mapped to the corresponding ADG Input
    _dfgNodeAttr[dfgNodeId] = dfgAttr;
    _t_adgNodeAttr[mappingTime%II][adgNodeId] = adgAttr;
    //@Yuan: set the ADGNode is used and record the corresponding DFGNode
    /*std::cout << "[HETA] set DFGNode " << dfgNode->name() << " at ADGNode " << adgNode->type() << "(" << adgNode->id() << ") at time: "\
    << mappingTime << " with II = " << II << std::endl;*/
    _numNodeMapped++;
    adgNode->addUsage(dfgNodeId);
    //std::cout << "mapping 311 _numNodemapped: " << _numNodeMapped << std::endl;
    return true;
}



// unmap DFG node
void Mapping::unmapDfgNode(DFGNode* dfgNode){
    //@Yuan: check if the dfgNode is mapped
    if(!_dfgNodeAttr.count(dfgNode->id()) || _dfgNodeAttr[dfgNode->id()].adgNode == nullptr)
       return;
    // unroute the input edges of this node
    for(auto& elem : dfgNode->inputEdges()){ // input-idx, edge-id
        int inEdgeId = elem.second; //
        unrouteDfgEdge(_dfg->edge(inEdgeId), false);
    }
    // unmap the mapped ADG Node
    ADGNode* adgNode = _dfgNodeAttr[dfgNode->id()].adgNode;
    if(adgNode){
        int earliestTime = *(_dfgNodeAttr[dfgNode->id()].mappedtime.begin());
        int latestTime = *(_dfgNodeAttr[dfgNode->id()].mappedtime.rbegin());
        for(int t = earliestTime; t <=latestTime ; t++){
            unuseADGNode(dfgNode, adgNode, t);
        }
    }
    _dfgNodeAttr.erase(dfgNode->id());
    _ALAPTime[dfgNode->getALAP()].erase(dfgNode->id());// delete the mapping time of this node
    _numNodeMapped--;
    for(auto dstNodes : dfgNode->outputs()){
        for(auto dstNode : dstNodes.second){
            if(isMapped(_dfg->node(dstNode.first))){
                unmapDfgNode(_dfg->node(dstNode.first));
            }
        }
    }
}

// unmap DFG node
void Mapping::unmapFixDfgNode(DFGNode* dfgNode){
    //@Yuan: check if the dfgNode is mapped
    /*if(!_dfgNodeAttr.count(dfgNode->id()) || _dfgNodeAttr[dfgNode->id()].adgNode == nullptr)
       return;*/
    // unroute the input edges of this node
    for(auto& elem : dfgNode->inputEdges()){ // input-idx, edge-id
        int inEdgeId = elem.second; //
        unrouteDfgEdge(_dfg->edge(inEdgeId), false);
    }
    // unmap the mapped ADG Node
    ADGNode* adgNode = _dfgNodeAttr[dfgNode->id()].adgNode;
    if(adgNode){
        int earliestTime = *(_dfgNodeAttr[dfgNode->id()].mappedtime.begin());
        int latestTime = *(_dfgNodeAttr[dfgNode->id()].mappedtime.rbegin());
        for(int t = earliestTime; t <=latestTime ; t++){
            unuseADGNode(dfgNode, adgNode, t);
        }
    }
    _dfgNodeAttr.erase(dfgNode->id());
    _ALAPTime[dfgNode->getALAP()].erase(dfgNode->id());// delete the mapping time of this node
    _numNodeMapped--;
    for(auto dstNodes : dfgNode->outputs()){
        for(auto dstNode : dstNodes.second){
            if(isMapped(_dfg->node(dstNode.first))){
                unmapDfgNode(_dfg->node(dstNode.first));
            }
        }
    }
}


// unmap DFG node
//@Yuan: just delete the _dfgNodeAttr status
void Mapping::unmapDfgNode(int DFGId){
    _dfgNodeAttr.erase(DFGId);
    _numNodeMapped--;
}


//@Yuan: unuse ADGNode in given time
void Mapping::unuseADGNode(DFGNode* dfgNode, ADGNode* adgNode, int time){
    int Time = time < 0 ? time + II : time ;
    // unmap the mapped ADG Node
    if(!_t_adgNodeAttr[Time % II].count(adgNode->id())|| std::find(_t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.begin(),_t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.end(), dfgNode) == _t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.end() ){
        return;
    }
     /*std::cout << "[HETA] UNMAP DFGNode " << dfgNode->name() << " at ADGNode " << adgNode->type() << "(" << adgNode->id() << ") at time: "\
    << time << " with II = " << II << std::endl;*/
    if(dfgNode->operation() == "INPUT" || dfgNode->operation() == "OUTPUT"){
       _t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.erase(std::remove(_t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.begin(), _t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.end(), dfgNode), _t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.end());
       if(_adgNodeUsageAttr.count(adgNode->id()) && _t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.empty()){
          _adgNodeUsageAttr[adgNode->id()]._currentConfigMem = _adgNodeUsageAttr[adgNode->id()]._currentConfigMem - 1;
       }
       int srcPort = *(_dfgNodeAttr[dfgNode->id()].srcPort.begin());
       int dstPort = _dfgNodeAttr[dfgNode->id()].dstPort;
       _t_adgNodeAttr[Time % II][adgNode->id()].inPortUsed[srcPort] = false;
       _t_adgNodeAttr[Time % II][adgNode->id()].outPortUsed[dstPort]= false;
    }else{
       _t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.erase(std::remove(_t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.begin(), _t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.end(), dfgNode), _t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.end());//delete the dfgNode that mapped at current adgNode
       if( _t_adgNodeAttr[Time % II][adgNode->id()].dfgNode.empty()){//if the ADGNode is only mapped by 1 dfgNode, just delete the _t_adgNodeAttr
            _t_adgNodeAttr[Time % II].erase(adgNode->id());
           if(_adgNodeUsageAttr.count(adgNode->id())){  //the ADGNode is not used in current time, configmem usage - 1
              _adgNodeUsageAttr[adgNode->id()]._currentConfigMem = _adgNodeUsageAttr[adgNode->id()]._currentConfigMem - 1;
           }
       }else{
           //if the time is the execute time of dfgNode,
          auto exeNode = _t_adgNodeAttr[Time % II][adgNode->id()].executeNode;
          if(exeNode == dfgNode){
            _t_adgNodeAttr[Time % II][adgNode->id()].executeNode = nullptr;
            _t_adgNodeAttr[Time % II][adgNode->id()].inPortUsed.clear();
          }
       }

        //_t_adgNodeAttr[time%II].erase(adgNode->id());
    }
}

// // if the ADG link is already routed
// bool Mapping::isRouted(ADGLink* link){
//     return !_adgLinkAttr[link->id()].dfgEdges.empty();
// }

// routed DFG edge links, including inter-node and intra-node links
const std::vector<EdgeLinkAttr>& Mapping::routedEdgeLinks(DFGEdge* edge){
    assert(_dfgEdgeAttr.count(edge->id()));
    return _dfgEdgeAttr[edge->id()].edgeLinks;
}

// /* route DFG edge to ADG link
// *  one link can route multiple edges, but they should have the same srcId and srcPortIdx
// */
// bool Mapping::routeDfgEdge(DFGEdge* edge, ADGLink* link){
//     if(_adgLinkAttr.count(link->id())){
//         for(auto& linkEdge : _adgLinkAttr[link->id()].dfgEdges){
//             if(linkEdge->srcId() != edge->srcId() || linkEdge->srcPortIdx() != edge->srcPortIdx()){
//                 return false;
//             }
//         }
//     }
//     _adgLinkAttr[link->id()].dfgEdges.push_back(edge);
//     EdgeLinkAttr attr;
//     attr.isLink = true;
//     attr.edgeLink.adgLink = link;
//     _dfgEdgeAttr[edge->id()].edgeLinks.push_back(attr);
//     return true;
// }


// route DFG edge to passthrough ADG node
// one internal link can passthrough multiple edges, but they should have the same srcId and srcPortIdx
// isTry: just try to route, not change the routing status
//@Yuan: add time, ADGNode usually be GIB, find an available outport in given time
//          dstPort
//             |
//         ------------
//         |   |      |
//         |   ？     |
//srcport--|----      |
//         ------------
bool Mapping::routeDfgEdgePass(DFGEdge* edge, ADGNode* passNode, int srcPort, int dstPort, bool isTry, int time){
    int passNodeId = passNode->id();
    int routeSrcPort = srcPort;
    int routeDstPort = dstPort;
    bool hasSameSrcEdge = false;
    if(_t_adgNodeAttr[time%II].count(passNodeId)){
        auto& passNodeAttr = _t_adgNodeAttr[time % II][passNodeId];
        // check if conflict with current routed edges
        for(auto& edgeLink : passNodeAttr.dfgEdgePass){//in given time, for each DFGEdge uses this ADGNode
            auto passEdge = edgeLink.edge;
            if((edgeLink.srcPort == srcPort || edgeLink.dstPort == dstPort) &&  // occupy the same port
               (passEdge->srcId() != edge->srcId() || passEdge->srcPortIdx() != edge->srcPortIdx())){
                return false;
            } else if((edgeLink.srcPort == srcPort || edgeLink.dstPort == dstPort) &&  // try to route same-source edge to same internal link
                       passEdge->srcId() == edge->srcId() && passEdge->srcPortIdx() == edge->srcPortIdx()){
                routeSrcPort = edgeLink.srcPort; // default route link
                routeDstPort = edgeLink.dstPort;
                hasSameSrcEdge = true;
            }
        }
        if(srcPort >= 0 && dstPort >= 0){ // manually assign srcPort and dstPort
            if(!passNode->isInOutConnected(srcPort, dstPort)){
                return false;
            } else if(isAdgNodeOutPortUsed(time%II, passNodeId, dstPort) && (!hasSameSrcEdge || (routeSrcPort != srcPort) || (routeDstPort != dstPort))){
                return false; // if dstPort used, must have same-source edge with same srcPort and dstPort
            } else if(isAdgNodeInPortUsed(time%II, passNodeId, srcPort) && (!hasSameSrcEdge || (routeSrcPort != srcPort))){
                return false; // if srcPort used, must have same-source edge with same srcPort
            }
            routeSrcPort = srcPort;
            routeDstPort = dstPort;
        } else if(srcPort >= 0){ // auto-assign dstPort
            if(isAdgNodeInPortUsed(time%II, passNodeId, srcPort) && (!hasSameSrcEdge || (routeSrcPort != srcPort))){
                return false; // no same-source edge or have different srcPort
            }
            if(!isAdgNodeInPortUsed(time%II, passNodeId, srcPort)){//if the target srcPort is unused
                bool flag = false;
                for(auto port : passNode->in2outs(srcPort)){
                    if(isAdgNodeOutPortUsed(time%II, passNodeId, port)){ // already used
                        continue;
                    }
                    // find one available port
                    routeSrcPort = srcPort;
                    routeDstPort == port;
                    flag = true;
                    break;
                }
                if(!flag){ // cannot find one available port
                    return false;
                }
            }
            // if have same-source edge and same srcPort, select the same dstPort
        } else if(dstPort >= 0){ // auto-assign srcPort
            if(isAdgNodeOutPortUsed(time%II, passNodeId, dstPort) && (!hasSameSrcEdge || (routeDstPort != dstPort))){
                return false;
            }
            if(!isAdgNodeOutPortUsed(time%II, passNodeId, dstPort)){ // no same-source edge or have different dstPort
                bool flag = false;
                for(auto port : passNode->out2ins(dstPort)){
                    if(isAdgNodeInPortUsed(time%II, passNodeId, port)){ // already used
                        continue;
                    }
                    // find one available port
                    routeSrcPort == port;
                    routeDstPort = dstPort;
                    flag = true;
                    break;
                }
                if(!flag){ // cannot find one available port
                    return false;
                }
            }
            // if have same-source edge with same dstPort, select the same srcPort and dstPort
        } else { // auto-assign srcPort and dstPort
            if(!hasSameSrcEdge){
                bool outflag = false;
                for(auto& elem : passNode->outputs()){
                    int outPort = elem.first;
                    if(isAdgNodeOutPortUsed(time%II, passNodeId, outPort)){ // already used
                        continue;
                    }
                    bool inflag = false;
                    for(auto inPort : passNode->out2ins(outPort)){
                        if(isAdgNodeInPortUsed(time%II, passNodeId, inPort)){ // already used
                            continue;
                        }
                        // find one available inport
                        routeSrcPort == inPort;
                        routeDstPort = outPort;
                        inflag = true;
                        break;
                    }
                    if(inflag){ // find one available port
                        outflag = true;
                        break;
                    }
                }
                if(!outflag){ // cannot find one available port
                    return false;
                }
            }
        }
    } else { // _adgNodeAttr.count(passNodeId) = 0; this passNode has not been used
        bool outflag = false;
        //std::cout << "one ADGNode done========" << std::endl;
        for(auto& elem : passNode->outputs()){
            int outPort = elem.first;
            auto inPorts = passNode->out2ins(outPort);
            //std::cout << "ADGNode type and inports size and outport"<< passNode->type() << "," << inPorts.size() << "," << outPort << std::endl;
            if(!inPorts.empty()){ // find one available inport
                routeSrcPort == *(inPorts.begin());
                routeDstPort = outPort;
                outflag = true;
                break;
            }
        }
        if(!outflag){ // cannot find one available port
            return false;
        }
    }
    //@Yuan: it seems not execute here
    if(!isTry){
        EdgeLinkAttr edgeLinkAttr;
        edgeLinkAttr.srcPort = routeSrcPort;
        edgeLinkAttr.dstPort = routeDstPort;
        edgeLinkAttr.adgNode = passNode;
        edgeLinkAttr.mappingtime = time;
        _dfgEdgeAttr[edge->id()].edgeLinks.push_back(edgeLinkAttr);
        DfgEdgePassAttr edgePassAttr;
        edgePassAttr.edge = edge;
        edgePassAttr.srcPort = routeSrcPort;
        edgePassAttr.dstPort = routeDstPort;
        //@Yuan: should consider mapping time here
        _t_adgNodeAttr[time%II][passNodeId].inPortUsed[routeSrcPort] = true;
        _t_adgNodeAttr[time%II][passNodeId].outPortUsed[routeDstPort] = true;
        _t_adgNodeAttr[time%II][passNodeId].dfgEdgePass.push_back(edgePassAttr);

    }
    return true;
}


// route DFG edge between srcNode and dstNode/OB
// find a routable path from srcNode to dstNode/OB by BFS
// dstNode = nullptr: OB
// dstPortRange: the input port index range of the dstNode
//@Yuan: returnTime: return the dstnode's mapping time
//@Yuan: isTry, used by fixVio, just try to route, do not change the status
bool Mapping::routeDfgEdgeFromSrc(DFGEdge* edge, ADGNode* srcNode, ADGNode* dstNode, const std::set<int>& dstPortRange, int pre_time, int* returnTime, bool isTry){
    struct VisitNodeInfo{
        // int inPortIdx;  // input port index of this node
        int srcNodeId;  // src node ID
        int srcInPortIdx; // input port index of src node
        int srcOutPortIdx; // output port index of src node
        int Mappingtime;  // currrent node mapping time
    };
    // cache visited node information, <<node-id, inport-index>, VisitNodeInfo>
    std::map<std::pair<int, int>, VisitNodeInfo> visitNodes;
    // cache visited nodes, <node, inport-index>
    std::queue<std::pair<ADGNode*, int>> nodeQue;
    // Breadth first search for possible routing path
    // assign the index of the output port of the srcNode
    //@Yuan: treat IB and srcNode differently, because the number of output of IB may not be 1
    int srcNodePortIdx = edge->srcPortIdx();
    int srcNodeId = edge->srcId();
    int dstNodePortIdx;
    int mappedAdgOutPort;
    std::pair<int, int> finalDstNode; // <node-id, inport-index>
    bool success = false;
    nodeQue.push(std::make_pair(srcNode, -1));
    int curTime = 0; // current mapping time
    int finalTime = 0; //
    while(!nodeQue.empty()){
        auto queHead = nodeQue.front();
        ADGNode* adgNode = queHead.first;
        int inPortIdx = queHead.second;
        nodeQue.pop();
        VisitNodeInfo info;
        std::vector<int> outPortIdxs;
        if(inPortIdx == -1){ // srcNode
            //@Yuan: we need to get the next node mapping time here
            curTime = pre_time + getDFG()->node(srcNodeId)->opLatency();
            outPortIdxs.push_back(srcNodePortIdx);
        }else{ // intermediate node
            //@Yuan: get current GIB's mapping time
            curTime = visitNodes[std::make_pair(adgNode->id(), inPortIdx)].Mappingtime;
            for(int outPortIdx : adgNode->in2outs(inPortIdx)){// for current node, find some available outport
                if(!routeDfgEdgePass(edge, adgNode, inPortIdx, outPortIdx, true, curTime)){ // try to find an internal link to route the edge
                    continue;
                }
                outPortIdxs.push_back(outPortIdx); // collect all available outPort
            }
        }
        // search this layer of nodes
        for(int outPortIdx : outPortIdxs){
            //@Yuan: GIB -> GIB, if the src GIB's outport is reged, dst GIB mapping time should + 1
            int nextTime = 0;
            if(adgNode->type() == "GIB"){
              if(dynamic_cast<GIBNode*>(adgNode)->outReged(outPortIdx)){
                 nextTime = curTime + 1;
                }else{
                 nextTime = curTime;
                }
            }else{
                 nextTime = curTime;
            }
            for(auto& elem : adgNode->output(outPortIdx)){
                int nextNodeId = elem.first;
                int nextSrcPort = elem.second;
                auto nextId = std::make_pair(nextNodeId, nextSrcPort);
                ADGNode* nextNode = _adg->node(nextNodeId);
                auto nextNodeType = nextNode->type();
                if(dstNode != nullptr){ // route to dstNode
                    if((dstNode->id() == nextNodeId) && dstPortRange.count(nextSrcPort)){ // get to the dstNode
                        //@Yuan: check if the dstNode can be mapped in nextTime, only for OB
                        if(_dfg->node(edge->dstId())->operation() == "OUTPUT"){
                            if(!isAdgNodeInPortUsed(nextTime%II, nextNodeId, nextSrcPort)){
                                success = true;
                                finalDstNode = nextId;
                                finalTime = nextTime;
                            }else{
                                continue;
                            }
                        }else{
                            if(!isAdgNodeInPortUsed(nextTime%II, nextNodeId, edge->dstId(), nextSrcPort)){
                                success = true;
                                finalDstNode = nextId;
                                finalTime = nextTime;
                            }else{
                                continue;
                            }
                        }
                    } else if((dstNode->id() == nextNodeId) || (nextNodeType == "GPE") || (nextNodeType == "LSU") ||// not use GPE node to route
                              (nextNodeType == "OB") || // cannot route to the dstNode through IOB
                              visitNodes.count(nextId)){ // the <node-id, inport-index> already visited
                            continue;
                      }else if( isAdgNodeInPortUsed(nextTime%II, nextNodeId, nextSrcPort)){// the input port is already used
                              // if has the same srcId and srcPortIdx in given time, nextId can still be used
                              auto& nextNodeAttr = _t_adgNodeAttr[nextTime % II][nextNodeId];
                              bool conflict = false;
                              for(auto& edgeLink : nextNodeAttr.dfgEdgePass){
                                    auto passEdge = edgeLink.edge;
                                    if(edgeLink.srcPort == nextSrcPort && (passEdge->srcId() != srcNodeId || passEdge->srcPortIdx() != srcNodePortIdx)){
                                        conflict = true; // the edge with different srcId or srcPortIdx occupied nextSrcPort
                                        break;
                                    }
                               }
                              if(conflict) continue;
                              }
                    }
                nodeQue.push(std::make_pair(nextNode, nextSrcPort)); // cache in queue
                info.srcNodeId = adgNode->id(); //  current node ID
                //std::cout << "mapping512 currentnode type " << adgNode->type() << std::endl;
                info.srcInPortIdx = inPortIdx;  // input port index of src node
                info.srcOutPortIdx = outPortIdx;   //  output port index of current node
                info.Mappingtime = nextTime;        //@Yuan: next node mapping time
                visitNodes[nextId] = info;
                if(success){
                    break;
                }
            }
            if(success){
                break;
            }
        }
        if(success){
            break;
        }
    }
    if(!success){
        return false;
    }
    //@Yuan: if it's try to route, we should return here
    if(isTry){
      *returnTime = finalTime;
      return true;
    }
    // route the found path
    auto routeNode = finalDstNode;//<dstNode-id, dstNode-srcPort>
    auto& edgeLinks = _dfgEdgeAttr[edge->id()].edgeLinks;
    int dstPort = (dstNode)? -1 : dstNodePortIdx;
    int NodeTime = 0;
    while (true){
        int nodeId = routeNode.first;
        int srcPort = routeNode.second;
        // keep the DFG edge routing status
        EdgeLinkAttr edgeAttr;
        edgeAttr.srcPort = srcPort;
        edgeAttr.dstPort = dstPort;
        edgeAttr.adgNode = _adg->node(nodeId);
        //@Yuan: along the path may across multi time, we need to find each ADGNode actually mapping time
        if(srcPort == -1){
           NodeTime = pre_time;
        }else{
           NodeTime = visitNodes[routeNode].Mappingtime;
        }
        edgeAttr.mappingtime = NodeTime;
        edgeLinks.push_back(edgeAttr);
        // keep the ADG Node routing status
        ADGNodeAttr& nodeAttr = _t_adgNodeAttr[NodeTime%II][nodeId];
        if(dstNode != nullptr && routeNode == finalDstNode){ // dstNode
            nodeAttr.inPortUsed[srcPort] = true;  // only change the input port status
            DfgEdgePassAttr passAttr;
            passAttr.edge = edge;
            passAttr.srcPort = srcPort;
            passAttr.dstPort = dstPort;
            nodeAttr.dfgEdgePass.push_back(passAttr);
            _dfgNodeAttr[edge->dstId()].srcPort.emplace(srcPort);// only record the input port that current dfgNode used
        } else if(nodeId == srcNode->id()){ // srcNode
            nodeAttr.outPortUsed[dstPort] = true; // only change the output port status
            break; // get to the srcNode
        } else{ // intermediate routing nodes or OB
            nodeAttr.inPortUsed[srcPort] = true;
            nodeAttr.outPortUsed[dstPort] = true;
            //@Yuan: passAttr records current ADGNode to route this path(edge), uses which srcPort and dstPort
            DfgEdgePassAttr passAttr;
            passAttr.edge = edge;
            passAttr.srcPort = srcPort;
            passAttr.dstPort = dstPort;
            nodeAttr.dfgEdgePass.push_back(passAttr);
        }
        dstPort = visitNodes[routeNode].srcOutPortIdx;
        routeNode = std::make_pair(visitNodes[routeNode].srcNodeId, visitNodes[routeNode].srcInPortIdx);

    }
    // reverse the passthrough nodes from srcNode to dstNode
    std::reverse(edgeLinks.begin(), edgeLinks.end());
    *returnTime = finalTime;
    _dfgEdgeAttr[edge->id()].endTime = finalTime;
    return true;
}


// find a routable path from dstNode to srcNode/IB by BFS
// srcNode = nullptr: IB
//@Yuan: isTry, used by fixVio, just try to route, do not change the status
bool Mapping::routeDfgEdgeFromDst(DFGEdge* edge, ADGNode* srcNode, ADGNode* dstNode, const std::set<int>& dstPortRange, int curtime, bool isTry){
            //std::cout << "new edge~~ dstPortRange size" <<dstPortRange.size() <<std::endl;
struct VisitNodeInfo{
        int dstNodeId;     // dst node ID
        int dstInPortIdx;  // input port index of dst node
        int dstOutPortIdx; // output port index of dst node
        int Mappingtime;  //@Yuan: curreent dfgNode mapping time
    };
    //@Yuan: cache visited node information, <<node-id, outport-index, time>, VisitNodeInfo>
    std::map<std::pair<int, int>, VisitNodeInfo> visitNodes;
    // cache visited nodes, <node, outport-index>
    std::queue<std::pair<ADGNode*, int>> nodeQue;
    // Breadth first search for possible routing path
    // assign the index of the output port of the srcNode
    int srcNodeOutPortIdx = edge->srcPortIdx();
    int srcNodeId = edge->srcId();
    auto srcDFGNode = _dfg->node(srcNodeId);
    int srcNodeInPortIdx = -1;
    int mappedAdgInPort;
    std::pair<int, int> finalSrcNode; // <node-id, outport-index>
    bool success = false;
    int curTime = 0; // current mapping time
    int finalTime = 0; //
    DFGEdge* sameSrcEdge = nullptr; // the already-routed edge with the same srcId and srcPortIdx
    nodeQue.push(std::make_pair(dstNode, -1));
    while(!nodeQue.empty()){
        auto queHead = nodeQue.front();
        ADGNode* adgNode = queHead.first;
        int outPortIdx = queHead.second;
        nodeQue.pop();
        std::vector<int> inPortIdxs;
        if(outPortIdx == -1){ // dstNode
            DFGNode* dstDFGNode = _dfg->node(edge->dstId());
            for(int inPort : dstPortRange){
                if(!isAdgNodeInPortUsed(curtime%II, dstNode->id(), dstDFGNode->id(), inPort)){
                    inPortIdxs.push_back(inPort);
                }
            }
            curTime = curtime; //initial time equals to dstNode mapping time
        }else{ // intermediate node
            curTime = visitNodes[std::make_pair(adgNode->id(), outPortIdx)].Mappingtime;
            for(int inPortIdx : adgNode->out2ins(outPortIdx)){
                if(!routeDfgEdgePass(edge, adgNode, inPortIdx, outPortIdx, true, curTime)){ // try to find an internal link to route the edge
                    continue;
                }
                inPortIdxs.push_back(inPortIdx); // collect all available inPort
            }
        }

        // search this layer of nodes
        for(int inPortIdx : inPortIdxs){
            int nextTime = curTime;
            auto elem = adgNode->input(inPortIdx);
            int nextNodeId = elem.first;
            int nextDstPort = elem.second;
            auto nextId = std::make_pair(nextNodeId, nextDstPort);
            ADGNode* nextNode = _adg->node(nextNodeId);
            if(nextNode->type() == "GIB"){
                if(dynamic_cast<GIBNode*>(nextNode)->outReged(nextDstPort)){
                 nextTime = curTime - 1;
                }
            }else if(nextNode->type() == "LSU"){
                nextTime = curTime - 1;
            }
            if(nextTime < 0){
                continue;
            }
            auto nextNodeType = nextNode->type();
                if(nextNodeType == "IB" && srcDFGNode->operation() == "INPUT"){
                    if(isMapped(srcDFGNode)){                       //the INPUT node has been mapped
                        if(_dfgNodeAttr[srcNodeId].adgNode != nextNode || *(_dfgNodeAttr[srcNodeId].mappedtime.begin())!= nextTime || _dfgNodeAttr[srcNodeId].dstPort != nextDstPort){
                            continue;
                        }else{
                            success = true;
                            finalSrcNode = nextId;
                            finalTime = nextTime;
                            srcNodeInPortIdx = *(_dfgNodeAttr[srcNodeId].srcPort.begin());
                            mappedAdgInPort = nextNode->input(*(_dfgNodeAttr[srcNodeId].srcPort.begin())).second;
                           }

                    }else{                                                  //the INPUT node has not been mapped
                        if(!isAdgNodeOutPortUsed(nextTime%II, nextNodeId, nextDstPort)){    // the IB output port is already used
                           auto inports =  nextNode->out2ins(nextDstPort);
                           for(auto inport : inports){
                              if(!isAdgNodeInPortUsed(nextTime%II, nextNodeId, inport)){
                                finalSrcNode = nextId;
                                finalTime = nextTime;
                                srcNodeInPortIdx = inport;
                                mappedAdgInPort = nextNode->input(inport).second;
                                if(mapInDfgNode(srcDFGNode, nextNode, nextTime, inport, nextDstPort, mappedAdgInPort, isTry)){
                                   success = true;
                                   break;
                                }
                              }
                           }
                        }
                        if(!success){
                            continue;
                        }
                    }
                }else if(nextNodeType == "LSU" && srcDFGNode->operation() == "LOAD"){
                    if(isMapped(srcDFGNode)){                     //the LOAD node has been mapped
                        if(_dfgNodeAttr[srcNodeId].adgNode != nextNode || *(_dfgNodeAttr[srcNodeId].mappedtime.begin())!= nextTime || _dfgNodeAttr[srcNodeId].dstPort != nextDstPort){
                            continue;
                        }else{
                            success = true;
                            finalSrcNode = nextId;
                            finalTime = nextTime;
                            srcNodeInPortIdx = *(_dfgNodeAttr[srcNodeId].srcPort.begin());
                           }
                    }else{                                       //the LOAD node has not been mapped
                        if(mapDfgNode(srcDFGNode, nextNode, nextTime, true, 0, nextTime)){
                            success = true;
                            finalSrcNode = nextId;
                            finalTime = nextTime;
                            srcNodeInPortIdx = *(nextNode->out2ins(nextDstPort).begin());
                        }
                        if(!success){
                            continue;
                        }
                    }
                }else if((nextNodeType == "GPE") || (nextNodeType == "LSU") ||((nextNodeType == "IB")) ||// not use GPE/LSU node to route
                          visitNodes.count(nextId)){ // the <node-id, outport-index> already visited
                    continue;
                } else if(isAdgNodeOutPortUsed(nextTime%II, nextNodeId, nextDstPort)){ // the output port is already used
                    // if has the same srcId and srcPortIdx, nextId can still be used
                    auto& nextNodeAttr = _t_adgNodeAttr[nextTime%II][nextNodeId];
                    bool conflict = false;
                    for(auto& edgeLink : nextNodeAttr.dfgEdgePass){//edgeLink records one edge using which inport and outport
                        auto passEdge = edgeLink.edge;
                        if(edgeLink.dstPort == nextDstPort && (passEdge->srcId() != srcNodeId || passEdge->srcPortIdx() != srcNodeOutPortIdx)){
                            conflict = true; // the edge with different srcId or srcPortIdx occupied nextDstPort
                            break;
                        }/*else if(edgeLink.dstPort == nextDstPort){
                            sameSrcEdge = passEdge; // the edge with the same srcId and srcPortIdx
                            finalSrcNode = nextId;
                            finalTime = nextTime;
                            success = true; // REUSE part of the routing path of the edge
                            break;
                        }*/
                    }
                    if(conflict) continue;
                }
            nodeQue.push(std::make_pair(nextNode, nextDstPort)); // cache in queue
            VisitNodeInfo info;
            info.dstNodeId = adgNode->id(); //  current node ID
            info.dstInPortIdx = inPortIdx;  // input port index of src node
            info.dstOutPortIdx = outPortIdx;   //  output port index of current node
            info.Mappingtime = nextTime;
            visitNodes[nextId] = info;
            if(success){
                break;
            }
        }
        if(success){
            break;
        }
    }
    if(!success){
        return false;
    }
     //@Yuan: if it's try to route, we should return here
    if(isTry){
      return true;
    }
    // route the found path
    auto routeNode = finalSrcNode;
    auto& edgeLinks = _dfgEdgeAttr[edge->id()].edgeLinks;
    int srcPort = (srcNode != nullptr)? -1 : srcNodeInPortIdx;
    if(srcNode != nullptr) exit(0);
    /*if(sameSrcEdge != nullptr){
        auto& sameSrcEdgeLinks = _dfgEdgeAttr[sameSrcEdge->id()].edgeLinks;
        for(auto& link : sameSrcEdgeLinks){ // search from the source node
            // keep the DFG edge routing status
            edgeLinks.push_back(link);
            // keep the ADG Node routing status
            ADGNodeAttr& nodeAttr = _t_adgNodeAttr[finalTime%II][link.adgNode->id()];
            DfgEdgePassAttr passAttr;
            passAttr.edge = edge;
            passAttr.srcPort = link.srcPort;
            passAttr.dstPort = link.dstPort;
            nodeAttr.dfgEdgePass.push_back(passAttr);
            if(link.adgNode->id() == finalSrcNode.first && link.dstPort == finalSrcNode.second){
                break;
            }
        }
        srcPort = visitNodes[finalSrcNode].dstInPortIdx;
        routeNode = std::make_pair(visitNodes[finalSrcNode].dstNodeId, visitNodes[finalSrcNode].dstOutPortIdx);
    }*/
    int nodeTime = 0;
    while (true){
        int nodeId = routeNode.first;
        int dstPort = routeNode.second;
        // keep the DFG edge routing status
        EdgeLinkAttr edgeAttr;
        edgeAttr.srcPort = srcPort;
        edgeAttr.dstPort = dstPort;
        edgeAttr.adgNode = _adg->node(nodeId);
        if(dstPort == -1){
            nodeTime = curtime;
        }else{
            nodeTime = visitNodes[routeNode].Mappingtime;
        }
        edgeAttr.mappingtime = nodeTime;
        edgeLinks.push_back(edgeAttr);
        // keep the ADG Node routing status
        ADGNodeAttr& nodeAttr = _t_adgNodeAttr[nodeTime%II][nodeId];
        if(routeNode == finalSrcNode){ // srcNode
            nodeAttr.outPortUsed[dstPort] = true; // only change the output port status
        } else if(nodeId == dstNode->id()){ // dstNode
            DfgEdgePassAttr passAttr;
            passAttr.edge = edge;
            passAttr.srcPort = srcPort;
            passAttr.dstPort = dstPort;
            nodeAttr.dfgEdgePass.push_back(passAttr);
            nodeAttr.inPortUsed[srcPort] = true;  // only change the input port status
            _dfgNodeAttr[edge->dstId()].srcPort.emplace(srcPort);
            break; // get to the dstNode
        } else{
            nodeAttr.inPortUsed[srcPort] = true;
            nodeAttr.outPortUsed[dstPort] = true;
            DfgEdgePassAttr passAttr;
            passAttr.edge = edge;
            passAttr.srcPort = srcPort;
            passAttr.dstPort = dstPort;
            nodeAttr.dfgEdgePass.push_back(passAttr);
        }
        srcPort = visitNodes[routeNode].dstInPortIdx;
        routeNode = std::make_pair(visitNodes[routeNode].dstNodeId, visitNodes[routeNode].dstOutPortIdx);
    }
    _dfgEdgeAttr[edge->id()].endTime = curtime;
    return true;
}

// new add support for lsu
// find the available input ports in the dstNode to route edge
// @Yuan: should condier about OB
std::set<int> Mapping::availDstPorts(DFGEdge* edge, ADGNode* dstNode){
    DFGNode* dstDfgNode = _dfg->node(edge->dstId());
    int edgeDstPort = edge->dstPortIdx();
    int opereandNum = dstDfgNode->numInputs();
    if(dstNode->type() == "GPE"){
        GPENode* dstADGNode = dynamic_cast<GPENode*>(dstNode);
        std::set<int> dstPortRange; // the input port index range of the dstNode
        std::vector<int> opIdxs; // operand indexes
        std::set<int> operandUsed;
        if(dstDfgNode->commutative()){ // operands are commutative, use all the operand indexes
        // if the operands are commutative, we should find the operands have been used
            for(auto elem : dstDfgNode->inputEdges()){
                if(_dfgEdgeAttr.count(elem.second) && !_dfgEdgeAttr[elem.second].edgeLinks.empty()){
                    int inPortused = (_dfgEdgeAttr[elem.second].edgeLinks.rbegin())->srcPort;
                    operandUsed.emplace(dstADGNode->getOperandIdx(inPortused));
                }
            }
            for(int opIdx = 0; opIdx < opereandNum; opIdx++){
                if(!operandUsed.count(opIdx)){
                    opIdxs.push_back(opIdx);
                }
            }
        } else{ // operands are not commutative, use the edgeDstPort as the operand index
            opIdxs.push_back(edgeDstPort);
        }
    // select all the ports connected to available operand
        for(int opIdx : opIdxs){
            auto& inPorts = dstADGNode->operandInputs(opIdx);
            for(int inPort : inPorts){ // all the ports are available
                dstPortRange.emplace(inPort);
            }
    }
    return dstPortRange;
    }else  if(dstNode->type() == "LSU") {
        LSUNode* dstADGNode = dynamic_cast<LSUNode*>(dstNode);
        std::set<int> dstPortRange; // the input port index range of the dstNode
        std::vector<int> opIdxs; // operand indexes
        std::set<int> operandUsed;
        if(dstDfgNode->commutative()){ // operands are commutative, use all the operand indexes
        // if the operands are commutative, we should find the operands have been used
            for(auto elem : dstDfgNode->inputEdges()){
                if(_dfgEdgeAttr.count(elem.second) && !_dfgEdgeAttr[elem.second].edgeLinks.empty()){
                    int inPortused = (_dfgEdgeAttr[elem.second].edgeLinks.rbegin())->srcPort;
                    operandUsed.emplace(dstADGNode->getOperandIdx(inPortused));
                }
            }
            for(int opIdx = 0; opIdx < opereandNum; opIdx++){
                if(!operandUsed.count(opIdx)){
                    opIdxs.push_back(opIdx);
                }
            }
        } else{ // operands are not commutative, use the edgeDstPort as the operand index
            opIdxs.push_back(edgeDstPort);
        }
        // select all the ports connected to available operand
        for(int opIdx : opIdxs){
            auto& inPorts = dstADGNode->operandInputs(opIdx);
            for(int inPort : inPorts){ // all the ports are available
                dstPortRange.emplace(inPort);
            }
    }
    return dstPortRange;
    }else{
        IOBNode* dstADGNode = dynamic_cast<IOBNode*>(dstNode);
        std::set<int> dstPortRange; // the input port index range of the dstNode
        for(auto inPort : dstADGNode->inputs()){
           dstPortRange.emplace(inPort.first);
        }
        return dstPortRange;
    }
}

// route DFG edge between srcNode and dstNode
// find a routable path from srcNode to dstNode by BFS
bool Mapping::routeDfgEdge(DFGEdge* edge, ADGNode* srcNode, ADGNode* dstNode, int preTime, int* finalTime){
    std::set<int> dstPortRange = availDstPorts(edge, dstNode); // the input port index range of the dstNode
    if(dstPortRange.empty()){ // no available input port in the dstNode
        return false;
    }
    //@Yuan: route from srcNode to dstNode
    return routeDfgEdgeFromSrc(edge, srcNode, dstNode, dstPortRange, preTime, finalTime, false);
    //return routeDfgEdgeFromDst(edge, srcNode, dstNode, dstPortRange, time);
    // if(!routeDfgEdgeFromDst(edge, srcNode, dstNode, dstPortRange)){
    //     return routeDfgEdgeFromSrc(edge, srcNode, dstNode, dstPortRange);
    // }
    // return true;
}


// route DFG edge between adgNode and IOB
// is2Input: whether connected to IB or OB
/*bool Mapping::routeDfgEdge(DFGEdge* edge, ADGNode* adgNode, bool is2Input){
    std::set<int> dstPortRange; // the input port index range of the dstNode
    if(!is2Input){ // route to OB
        return routeDfgEdgeFromSrc(edge, adgNode, nullptr, dstPortRange, 0, 0);
    }
    dstPortRange = availDstPorts(edge, adgNode); // the input port index range of the dstNode
    if(dstPortRange.empty()){ // no available input port in the dstNode
        return false;
    }
    return routeDfgEdgeFromDst(edge, nullptr, adgNode, dstPortRange, 0);
}*/

//@Yuan : DFG edge between adgNode and IOB
// curTime: current adgNode mapping time, will decrease along the path
bool Mapping::routeDfgEdge(DFGEdge* edge, ADGNode* adgNode, int  curTime){
    std::set<int> dstPortRange; // the input port index range of the dstNode
    dstPortRange = availDstPorts(edge, adgNode); // the input port index range of the dstNode (for INPUT -> GPE, the availDstPorts are the range of GPE input ports)
    if(dstPortRange.empty()){ // no available input port in the dstNode
        return false;
    }
    return routeDfgEdgeFromDst(edge, nullptr, adgNode, dstPortRange, curTime, false);
}

// @Yuan: route self-cycle DFG edge
bool Mapping::routeSelfDfgEdge(DFGNode* dfgNode, DFGEdge* edge, ADGNode* adgNode, int EndTime){
    GPENode* gpeNode = dynamic_cast<GPENode*>(adgNode); // only GPE will has self-cycle
    //@Yuan: find an available ALU operand
    std::set<int> operandUsed;
    int inPort;
    if(!dfgNode->commutative()){
        inPort = *(gpeNode->operandInputs(edge->dstPortIdx()).begin());
    }else{
        for(auto edges: dfgNode->inputEdges()){
            if(edge->id() == edges.second)
                continue;
            int inPortused = (_dfgEdgeAttr[edges.second].edgeLinks.rbegin())->srcPort;
            operandUsed.emplace(gpeNode->getOperandIdx(inPortused));
        }
        for(auto inPorts : gpeNode->inputs()){
            if(operandUsed.count(gpeNode->getOperandIdx(inPorts.first))){
                continue;
            }else{
                inPort = inPorts.first;
                break;
            }
        }
    }
    int edgeId = edge->id();
    auto& edgeLinks = _dfgEdgeAttr[edgeId].edgeLinks;
    _dfgEdgeAttr[edgeId].endTime = EndTime;
    //record edge attribute
    EdgeLinkAttr edgeAttr;
    edgeAttr.srcPort = inPort;
    edgeAttr.dstPort = 0;
    edgeAttr.adgNode = adgNode;
    edgeAttr.mappingtime = EndTime;
    edgeLinks.push_back(edgeAttr);
    //record node attribute (which edge pass this node)
    DfgEdgePassAttr passAttr;
    passAttr.edge = edge;
    passAttr.srcPort = inPort;
    passAttr.dstPort = 0;
    int Time = EndTime < 0 ? EndTime + II : EndTime;
    auto nodeAttr = _t_adgNodeAttr[Time % II][adgNode->id()];
    nodeAttr.dfgEdgePass.push_back(passAttr);
    return true;
}

// unroute DFG edge
//@Yuan: isVio, unroute is due to this edge has violation
void Mapping::unrouteDfgEdge(DFGEdge* edge, bool isVio){
    int eid = edge->id();
    if(!_dfgEdgeAttr.count(eid)) return;
    auto& edgeAttr = _dfgEdgeAttr[eid];
    //std::cout << "mapping1139 =============one edge ==============" << std::endl;
    // remove this edge from all the routed ADG links
    for(auto& edgeLink : edgeAttr.edgeLinks){
        // if(edgeLink.isLink){ // remove this edge from all the routed ADG links
        //     auto link = edgeLink.edgeLink.adgLink;
        //     if(!_adgLinkAttr.count(link->id())) continue;
        //     auto& linkEdges = _adgLinkAttr[link->id()].dfgEdges;
        //     std::remove(linkEdges.begin(), linkEdges.end(), edge);
        // } else { // remove this edge from all the passed-through ADG nodes
        auto node = edgeLink.adgNode;//get the ADGNode that this edge used
        int NodeTime = edgeLink.mappingtime;//get the time of ADGNode to route the edge
        int Time = NodeTime < 0 ? NodeTime + II : NodeTime;
        if(!_t_adgNodeAttr[Time % II].count(node->id())) continue;
        auto& nodeAttr = _t_adgNodeAttr[Time % II][node->id()];
        auto& nodeEdges = nodeAttr.dfgEdgePass;
        auto iter = std::remove_if(nodeEdges.begin(), nodeEdges.end(), [&](DfgEdgePassAttr& x){ return (x.edge == edge); });
        nodeEdges.erase(iter, nodeEdges.end());
        bool setInPortUsed = (edgeLink.srcPort != -1);
        bool setOutPortUsed = (edgeLink.dstPort != -1);
        //@Yuan: one ADGNode may pass multiple DFGEdge
        for(auto& nodeEdge : nodeEdges){ // the srcPort/dstPort may be used by other edges
            if(nodeEdge.edge->id() == edge->id()){
                continue;
            }
            if(nodeEdge.srcPort == edgeLink.srcPort){
                setInPortUsed = false;
            }
            if(nodeEdge.dstPort == edgeLink.dstPort){
                setOutPortUsed = false;
            }
            if(!setInPortUsed && !setOutPortUsed){
                break;
            }
        }
        if(setInPortUsed){
            if(node->type() == "GPE" ||  node->type() == "LSU"){
                _dfgNodeAttr[edge->dstId()].srcPort.erase(edgeLink.srcPort);
            }
            nodeAttr.inPortUsed[edgeLink.srcPort] = false;
        }
        if(setOutPortUsed){
            nodeAttr.outPortUsed[edgeLink.dstPort] = false;
        }
        // if(edgeLink.srcPort != -1){
        //     nodeAttr.inPortUsed[edgeLink.srcPort] = false;
        // }
        // if(edgeLink.dstPort != -1){
        //     nodeAttr.outPortUsed[edgeLink.dstPort] = false;
        // }
        // }
    }
    _dfgEdgeAttr.erase(eid);
    if(!isVio){
    delVioEdge(eid);
    }
    // unmap DFG IO
    if(_dfg->node(edge->srcId())->operation() == "INPUT" && _dfgNodeAttr.count(edge->srcId())){ // connected to mapped DFG input port
        auto inNode = _dfg->node(edge->srcId());
        //check if the input port has other mapped output nodes, if so, it can not be unmapped
        bool canUnmap = true;
        for(auto output : inNode->outputs()){
            for(auto outNodes : output.second){
                DFGNode * outNode = _dfg->node(outNodes.first);
                if(outNode->id() != edge->dstId() && isMapped(outNode) && outNode != nullptr){
                    canUnmap = false;
                    break;
                }
            }
            if(!canUnmap)
                break;
        }
        if(canUnmap){
            // unmap the mapped ADG Node
            int time = *(_dfgNodeAttr[inNode->id()].mappedtime.begin());
            ADGNode* adgNode = _dfgNodeAttr[inNode->id()].adgNode;
            if(adgNode){
               unuseADGNode(inNode, adgNode, time);
            }
            _dfgNodeAttr.erase(inNode->id());
            _numNodeMapped--;
        }
    }
    /*std::cout << "[HETA] UNMAP DFGEdge " << edge->id() <<" at time: "\
    << edgeAttr.endTime << " with II = " << II << std::endl;*/
}


// if succeed to map all DFG nodes
bool Mapping::success(){
    return _dfg->nodes().size() == _numNodeMapped;
}


// total/max edge length (link number)
void Mapping::getEdgeLen(int& totalLen, int& maxLen){
    int total = 0;
    int maxVal = 0;
    for(auto& elem : _dfgEdgeAttr){
        int num = elem.second.edgeLinks.size();
        total += num;
        maxVal = std::max(maxVal, num);
    }
    totalLen = total;
    maxLen = maxVal;
}


// assign DFG IO to ADG IO according to mapping result
// post-processing after mapping
void Mapping::assignDfgIO(){
    // assign input ports
    for(auto& elem : _dfg->inputEdges()){
        // IB has only one input port, connected to ADG input port
        // all the edges with the same src routing to the same IB
        int eid = *(elem.second.begin());
        auto ib = _dfgEdgeAttr[eid].edgeLinks.begin()->adgNode; // ADG IB node
        int inport = ib->input(0).second;
        _dfgInputAttr[elem.first].adgIOPort = inport;
    }
    // assign output ports
    for(auto& elem : _dfg->outputEdges()){
        // OB has only one output port, connected to ADG output port
        auto ob = _dfgEdgeAttr[elem.second].edgeLinks.rbegin()->adgNode; // ADG OB node
        int outport = ob->output(0).begin()->second;
        _dfgOutputAttr[elem.first].adgIOPort = outport;
    }
}



// ==== tming schedule >>>>>>>>>>>>>
// // reset the timing bounds of each DFG node
// void Mapping::resetBound(){
//     // int INF = 0x3fffffff;
//     for(auto& elem : _dfg->nodes()){
//         auto& attr = _dfgNodeAttr[elem.second->id()];
//         attr.minLat = 0;
//         attr.maxLat = 0;
//     }
// }

// calculate the routing latency of each edge, not inlcuding the delay pipe
void Mapping::calEdgeRouteLat(){
    for(auto& elem : _dfgEdgeAttr){
        auto& attr = elem.second;
        int lat = 0;
        for(auto& linkAttr : attr.edgeLinks){
            if(linkAttr.adgNode->type() == "GIB"){ // edge only pass-through GIB nodes
                bool reged = dynamic_cast<GIBNode*>(linkAttr.adgNode)->outReged(linkAttr.dstPort); // output port reged
                lat += reged;
            }
        }
        attr.lat = lat;
        // attr.latNoDelay = lat;
    }
}


// calculate the DFG node latency bounds not considering the Delay components, including
// min latency of the output ports
// ID of the DFG node with the max latency
void Mapping::latencyBound(){
    int maxLatDfg = 0; // max latency of DFG
    int maxLatNodeId;  // ID of the DFG node with the max latency
    // calculate the LOWER bounds in topological order
    for(DFGNode* node : _dfg->topoNodes()){
        int maxLat = 0; // max latency of the input ports
        int nodeId = node->id();
        int nodeOpLat = _dfg->node(nodeId)->opLatency(); // operation latency
        for(auto& elem : node->inputEdges()){
            int eid = elem.second;
            int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
            int srcNodeId = _dfg->edge(eid)->srcId();
            int srcNodeLat = 0; // DFG input port min latency = 0
            if(srcNodeId != _dfg->id()){ // not connected to DFG input port
                srcNodeLat = _dfgNodeAttr[srcNodeId].lat;
            }
            int inPortLat = srcNodeLat + routeLat;
            maxLat = std::max(maxLat, inPortLat);
        }
        // for(auto& elem : node->inputs()){
        //     int srcNodeId = elem.second.first;
        //     int srcNodeLat = 0;
        //     if(srcNodeId != _dfg->id()){ // not connected to DFG input port
        //         srcNodeLat = _dfgNodeAttr[srcNodeId].lat;
        //     }
        //     int inPortLat = srcNodeLat; // edge no latency
        //     maxLat = std::max(maxLat, inPortLat);
        // }
        int lat = maxLat + nodeOpLat;
        _dfgNodeAttr[nodeId].lat = lat;
        if(lat >= maxLatDfg){
            maxLatDfg = lat;
            maxLatNodeId = nodeId;
        }
    }
    // _maxLat = maxLatDfg;
    _maxLatNodeId = maxLatNodeId;
}

//new
// schedule the latency of each DFG node based on the mapping status
// DFG node latency: output port latency
// DFG edge latency: latency from the output port of src node to the ALU Input port of dst Node, including DelayPipe
void Mapping::latencySchedule(){
    std::set<int> scheduledNodeIds;
    std::vector<DFGNode*> unscheduledNodes = _dfg->topoNodes();
    std::reverse(unscheduledNodes.begin(), unscheduledNodes.end()); // in reversed topological order
    // calculate the routing latency of each edge, not inlcuding the delay pipe
    calEdgeRouteLat();
    // calculate the DFG node latency bounds, finding the max-latency path
    latencyBound();
    // schedule the DFG nodes in the max-latency path
    DFGNode* dfgNode = _dfg->node(_maxLatNodeId);
    scheduledNodeIds.emplace(dfgNode->id());
    auto iterEnd = std::remove(unscheduledNodes.begin(), unscheduledNodes.end(), dfgNode);
    while(dfgNode){ // until getting to the input port
        int nodeId = dfgNode->id();
        // std::cout << "id: " << nodeId << std::endl;
        // use the latency lower bound as the target latency
        int inPortLat = _dfgNodeAttr[nodeId].lat - _dfg->node(nodeId)->opLatency(); // input port latency
        int ADGmaxdelay = 0;
        if(dfgNode->operation() == "STORE" ||dfgNode->operation() == "LOAD"){
            LSUNode* lsuNode = dynamic_cast<LSUNode*>(_dfgNodeAttr[nodeId].adgNode); // mapped LSU node
            ADGmaxdelay = lsuNode->maxDelay();
        }else{
            GPENode* gpeNode = dynamic_cast<GPENode*>(_dfgNodeAttr[nodeId].adgNode); // mapped GPE node
            ADGmaxdelay = gpeNode->maxDelay();
        }
        _dfgNodeAttr[nodeId].maxLat = inPortLat; // input port max latency
        _dfgNodeAttr[nodeId].minLat = std::max(inPortLat - ADGmaxdelay, 0); // input port min latency
        DFGNode* srcNode = nullptr;
        for(auto& elem : dfgNode->inputEdges()){
            int eid = elem.second;
            int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
            int srcNodeId = _dfg->edge(eid)->srcId();
            if(srcNodeId == _dfg->id()){ // connected to DFG input port
                continue;
            }
            int srcNodeLat = _dfgNodeAttr[srcNodeId].lat;
            if(inPortLat == srcNodeLat + routeLat){
                scheduledNodeIds.emplace(srcNodeId); // latency fixed
                srcNode = _dfg->node(srcNodeId);
                iterEnd = std::remove(unscheduledNodes.begin(), iterEnd, srcNode);
                break; // only find one path
            }
        }
        dfgNode = srcNode;
    }
    unscheduledNodes.erase(iterEnd, unscheduledNodes.end());
    // schedule the DFG nodes not in the max-latency path
    while(!unscheduledNodes.empty()){
        for(auto iter = unscheduledNodes.begin(); iter != unscheduledNodes.end();){
            dfgNode = *iter;
            int nodeId = dfgNode->id();
            // std::cout << "id: " << nodeId << std::endl;
            int maxLat = 0x3fffffff;
            int minLat = 0;
            bool fail = false; // fail to schedule this DFG node
            bool updated = false; // maxLat/minLat UPDATED
            for(auto& outsPerPort : dfgNode->outputEdges()){
                for(auto& eid : outsPerPort.second){
                    int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
                    int dstNodeId = _dfg->edge(eid)->dstId();
                    if(dstNodeId == _dfg->id()){ // connected to DFG output port
                        continue;
                    }
                    if(scheduledNodeIds.count(dstNodeId)){ // already scheduled
                        maxLat = std::min(maxLat, _dfgNodeAttr[dstNodeId].maxLat - routeLat);
                        minLat = std::max(minLat, _dfgNodeAttr[dstNodeId].minLat - routeLat);
                        updated = true;
                    }else{
                        fail = true;
                        break;
                    }
                }
                if(fail) break;
            }
            if(fail){
                iter++;
                continue;
            }
            // if all its output ports are connected to DFG output ports, keep original latency
            // otherwise, update the latency
            int targetLat = _dfgNodeAttr[nodeId].lat;
            if(updated){
                targetLat = std::max(targetLat, std::min(maxLat, minLat));
                _dfgNodeAttr[nodeId].lat = targetLat;
            }
            int ADGmaxdelay = 0;
            if(dfgNode->operation() == "STORE" ||dfgNode->operation() == "LOAD"){
               LSUNode* lsuNode = dynamic_cast<LSUNode*>(_dfgNodeAttr[nodeId].adgNode); // mapped LSU node
               ADGmaxdelay = lsuNode->maxDelay();
            }else{
               GPENode* gpeNode = dynamic_cast<GPENode*>(_dfgNodeAttr[nodeId].adgNode); // mapped GPE node
               ADGmaxdelay = gpeNode->maxDelay();
            }
            _dfgNodeAttr[nodeId].maxLat = targetLat - _dfg->node(nodeId)->opLatency(); // input port max latency
            _dfgNodeAttr[nodeId].minLat = std::max(_dfgNodeAttr[nodeId].maxLat - ADGmaxdelay, 0); // input port min latency
            scheduledNodeIds.emplace(nodeId); // latency fixed
            iter = unscheduledNodes.erase(iter);
        }
    }
    // calculate the latency of DFG IO
    calIOLat();
    // calculate the latency violation of each edge
    calEdgeLatVio();
}

//new
//compare the DFGNode latency with the LSU maximun en delay
bool Mapping::calLSUenlatency(){
    bool flag = true;
    for(auto& dfgNode : _dfg->topoNodes()){
      if(dfgNode->operation() != "STORE" && dfgNode->operation() != "LOAD"){
          continue;
      }
      int nodeId = dfgNode->id();
      int nodelat =  _dfgNodeAttr[nodeId].lat -  _dfg->node(nodeId)->opLatency();
      LSUNode* lsuNode = dynamic_cast<LSUNode*>(_dfgNodeAttr[nodeId].adgNode); // mapped LSU node
      int lsumaxenlat = lsuNode->maxEnDelay();
      //lsuNode->setEnDelay(nodelat);//set LSU delay_en
      if(lsumaxenlat < nodelat){
          return false;
      }
    }
    return flag;
}
//new
// set the LSU Node en delay
void Mapping::setLSUenlatency(){
    for(auto& dfgNode : _dfg->topoNodes()){
      if(dfgNode->operation() != "STORE" && dfgNode->operation() != "LOAD"){
          continue;
      }
      int nodeId = dfgNode->id();
      int nodelat =  _dfgNodeAttr[nodeId].lat -  _dfg->node(nodeId)->opLatency();
      LSUNode* lsuNode = dynamic_cast<LSUNode*>(_dfgNodeAttr[nodeId].adgNode); // mapped LSU node
      lsuNode->setEnDelay(nodelat);//set LSU delay_en
    }
}
//new compare OB latency with store, to determine the total latency
// calculate the latency of DFG IO
void Mapping::calIOLat(){
    // DFG input port latency
    for(auto& insPerPort : _dfg->inputEdges()){
        int minLat = 0;
        int maxLat = 0x3fffffff;
        for(auto& eid : insPerPort.second){
            int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
            int dstNodeId = _dfg->edge(eid)->dstId();
            if(dstNodeId == _dfg->id()){ // connected to DFG output port
                continue;
            }
            minLat = std::max(minLat, _dfgNodeAttr[dstNodeId].minLat - routeLat);
            maxLat = std::min(maxLat, _dfgNodeAttr[dstNodeId].maxLat - routeLat);
        }
        int targetLat = std::min(maxLat, minLat);
        _dfgInputAttr[insPerPort.first].lat = targetLat;
    }
    int maxLat = 0;
    // DFG output port latency
    for(auto& elem : _dfg->outputEdges()){
        int eid = elem.second;
        int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
        int srcNodeId = _dfg->edge(eid)->srcId();
        int srcNodeLat;
        if(srcNodeId == _dfg->id()){ // connected to DFG input port
            srcNodeLat = _dfgInputAttr[_dfg->edge(eid)->srcPortIdx()].lat;
        } else{
            srcNodeLat = _dfgNodeAttr[srcNodeId].lat;
        }
        int targetLat = srcNodeLat + routeLat;
        _dfgOutputAttr[elem.first].lat = targetLat;
        maxLat = std::max(maxLat, targetLat);
    }
    //new
    //store node latency
    for(auto& iter : _dfg->stores()){
       int storelat;
       storelat = _dfgNodeAttr[iter.first].lat;
       maxLat = std::max(maxLat,storelat);
    }
    _maxLat = maxLat;
}


// calculate the latency violation of each edge
void Mapping::calEdgeLatVio(){
    int dfgSumVio = 0; // total edge latency violation
    int dfgMaxVio = 0; // max edge latency violation
    _vioDfgEdges.clear(); // DFG edges with latency violation
    for(DFGNode* node : _dfg->topoNodes()){
        int nodeId = node->id();
        int minLat = _dfgNodeAttr[nodeId].minLat; // min latency of the input ports
        int maxLat = _dfgNodeAttr[nodeId].maxLat; // max latency of the input ports, =  latency - operation_latency
        for(auto& elem : node->inputEdges()){
            int eid = elem.second;
            int routeLat = _dfgEdgeAttr[eid].lat; // latNoDelay;
            int srcNodeId = _dfg->edge(eid)->srcId();
            int srcNodeLat;
            if(srcNodeId != _dfg->id()){ // not connected to DFG input port
                srcNodeLat = _dfgNodeAttr[srcNodeId].lat;
            } else{ // connected to DFG input port
                srcNodeLat = _dfgInputAttr[_dfg->edge(eid)->srcPortIdx()].lat;
            }
            _dfgEdgeAttr[eid].lat = maxLat - srcNodeLat; // including delay pipe latency
            _dfgEdgeAttr[eid].delay = maxLat - srcNodeLat - routeLat; // delay pipe latency
            int inPortLat = srcNodeLat + routeLat;
            if(inPortLat < minLat){ // need to add pass node to compensate the latency gap
                int vio = minLat - inPortLat;
                _dfgEdgeAttr[eid].vio = vio;
                //_vioDfgEdges.emplace(eid);
                dfgSumVio += vio;
                dfgMaxVio = std::max(dfgMaxVio, vio);
            } else {
                _dfgEdgeAttr[eid].vio = 0;
            }
        }
    }
    _totalViolation = dfgSumVio;
    _maxViolation = dfgMaxVio;
}


// insert pass-through DFG nodes into a copy of current DFG
void Mapping::insertPassDfgNodes(DFG* newDfg){
    *newDfg = *_dfg;
    int maxNodeId = newDfg->nodes().rbegin()->first; // std::map auto sort the key
    int maxEdgeId = newDfg->edges().rbegin()->first;
    int maxDelay = 1;
    // for(auto& elem : _dfgNodeAttr){
    //     auto adgNode = elem.second.adgNode;
    //     if(!adgNode){
    //         std::cout << "error!: " << elem.first << std::endl;
    //         exit(0);
    //     }
    //     if(adgNode->type() == "GPE" ){
    //         maxDelay = std::max(dynamic_cast<GPENode*>(adgNode)->maxDelay(), maxDelay);
    //     }else if(adgNode->type() == "LSU"){
    //         maxDelay = std::max(dynamic_cast<LSUNode*>(adgNode)->maxDelay(), maxDelay);
    //     }
    //     /*if(adgNode->type() == "GPE"){//@Yuan: because the CGRA is homogeneous now, LSU's maxdelay = GPE's
    //         maxDelay = dynamic_cast<GPENode*>(adgNode)->maxDelay() + 1;
    //         break;
    //     }*/
    // }
    for(auto elem : _adg->nodes()){
        if(elem.second->type() == "GPE" ){
            maxDelay = std::max(dynamic_cast<GPENode*>(elem.second)->maxDelay(), maxDelay);
        }else if(elem.second->type() == "LSU"){
            maxDelay = std::max(dynamic_cast<LSUNode*>(elem.second)->maxDelay(), maxDelay);
        }
    }
    maxDelay = maxDelay + 1;
    int maxInsertNodesPerEdge = 1;
    for(auto vioedge : _vioDfgEdges){ // DFG edges with latency violation
        int vio = vioedge.second; // maybe add multiple nodes according to vio
        int eid = vioedge.first;
        if(vio == 0)
            continue;
        int num = std::min(maxInsertNodesPerEdge, std::max(1, vio/maxDelay));
        DFGEdge* e = newDfg->edge(eid);
        int srcId = e->srcId();
        int dstId = e->dstId();
        int srcPortIdx = e->srcPortIdx();
        int dstPortIdx = e->dstPortIdx();
        newDfg->delEdge(eid);
        int lastId = srcId;
        int lastPort = srcPortIdx;
        for(int i = 0; i < num; i++){
            DFGNode* newNode = new DFGNode();
            newNode->setId(++maxNodeId);
            newNode->setName("pass"+std::to_string(maxNodeId));
            newNode->setOperation("PASS");
            newDfg->addNode(newNode);
            DFGEdge* e1 = new DFGEdge(lastId, maxNodeId);
            e1->setId(++maxEdgeId);
            e1->setSrcPortIdx(lastPort);
            e1->setDstPortIdx(0);
            newDfg->addEdge(e1);
            lastId = maxNodeId;
            lastPort = 0;
        }
        DFGEdge* e2 = new DFGEdge(maxNodeId, dstId);
        e2->setId(++maxEdgeId);
        e2->setSrcPortIdx(0);
        e2->setDstPortIdx(dstPortIdx);
        newDfg->addEdge(e2);
        std::cout << "insert pass between: " << newDfg->node(srcId)->name() << " and " << newDfg->node(dstId)->name() << " srcPortIdx: " << srcPortIdx << " dstPortIdx: " << dstPortIdx << " num: " << num << std::endl;
    }
}


// check the ADG nodes can be used in given time
/*bool Mapping::canMap(int time, ADGNode* candidate){
  int curCinfigMem = candidate->CurconfigMem();
   // Check whether the limit of config mem is reached.
  if(curCinfigMem + 1 > candidate->MaxConfigMem()){
      return false;
  }
  for(int t = time; t < timeBoundary; t = t + II){
       if(candidate->isOccupied(t)){
           return false;
       }
  }
  for(int t = time; t >= 0; t = t - II){
       if(candidate->isOccupied(t)){
           return false;
       }
  }
  return true;
}*/

//@Yuan: set the ADG nodes is occupied
void Mapping::setOccupied(int time, ADGNode* adgNode, DFGNode* dfgNode){
    //@Yuan: set the ADGNode is used and record the corresponding DFGNode
    for(int t = time; t < timeBoundary ; t = t+II){
    adgNode->setOccupied(t, true);
    adgNode->setMappedDFG(t, dfgNode);
    }
    for(int t = time; t >= 0 ; t = t-II){
    adgNode->setOccupied(t, true);
    adgNode->setMappedDFG(t, dfgNode);
    }
}

//@Yuan: cancel the ADG nodes is occupied
void Mapping::cancelOccupied(int time, ADGNode* adgNode, DFGNode* dfgNode){
    //@Yuan: set the ADGNode is used and record the corresponding DFGNode
    for(int t = time; t < timeBoundary ; t = t+II){
    adgNode->setOccupied(t, false);
    adgNode->delMappedDFG(t, dfgNode);
    }
    for(int t = time; t >= 0 ; t = t-II){
    adgNode->setOccupied(t, false);
    adgNode->delMappedDFG(t, dfgNode);
    }
}

//@Yuan: set edge latency violation
void Mapping::setVio(int edgeId, int Vio){
    if(II > 5){
        std::cout << "edge 8 vio: " << edgeId <<"," <<Vio<<std::endl;
    }
     if(_dfgEdgeAttr.count(edgeId)){
         _dfgEdgeAttr[edgeId].vio = Vio;
     }

}

//@Yuan: record the latency violation edge
void Mapping::VioEdge(int edgeId, int vio){
 _vioDfgEdges[edgeId] = vio;
}

//@Yuan: delete the latency violation edge
void Mapping::delVioEdge(int edgeId){
 /*if(std::find(_vioDfgEdges.begin(), _vioDfgEdges.end(), edgeId) !=_vioDfgEdges.end() ){
    _vioDfgEdges.erase(std::remove(_vioDfgEdges.begin(), _vioDfgEdges.end(), edgeId), _vioDfgEdges.end());
 }*/
 if(_vioDfgEdges.count(edgeId))
    _vioDfgEdges.erase(edgeId);
}

//@Yuan: set edge delay, which is used for delay unit
void Mapping::setDly(int edgeId, int delay){
     if(_dfgEdgeAttr.count(edgeId)){
         _dfgEdgeAttr[edgeId].delay = delay;
     }

}

//@Yuan: set total violation value
void Mapping::setTotalVio(int edgeId){
    if(_dfgEdgeAttr.count(edgeId)){
         _totalViolation += _dfgEdgeAttr[edgeId].vio;
     }
}

//@Yuan: substract one edge vio in total violation value
void Mapping::subTotalVio(int edgeId){
    if(_dfgEdgeAttr.count(edgeId)){
         _totalViolation -= _dfgEdgeAttr[edgeId].vio;
     }
}

//@Yuan:set mapping time of one node (sort by ALAP)
void Mapping::setALAPMapTime(int ALAP, int nodeid, int time){
    _ALAPTime[ALAP][nodeid] = time;
}


//@Yuan: Get the average mapping time of the node corresponding to an ALAP value
int Mapping::getALAPMapTime(int ALAP){
    int sumTime = 0;
    int maxTime = 0;
    if(!_ALAPTime[ALAP].empty()){
     for(auto nodesTime : _ALAPTime[ALAP]){
        sumTime += nodesTime.second;
        //maxTime = std::max(nodesTime.second, maxTime);
     }
     return int (std::floor(sumTime / _ALAPTime[ALAP].size()));
     //return maxTime;
   }else{
    return ALAP;
   }


}


//@Yuan: go back to adjust the delay unit of the GPE/LSU, to avoid timing violation
// latestTime: the dfgNode latest mapping time, which will affect the delay usage
int Mapping::tryFixVio(DFGNode* dfgNode, ADGNode* candidate, DFGEdge* edge, int latestTime){
    int maxDelay = 0;
    int curVio = 0;
    bool succeed;
    int returnTime = 0;
    int delayUsage = 0;
    DFGNode * curNode = dfgNode;
    DFGEdge * curEdge = edge;
    //@Yuan: calculate the maximum numbers of go back nodes
    if(candidate->type() == "GPE"){
        GPENode * gpe = dynamic_cast<GPENode*>(candidate);
        maxDelay = gpe->maxDelay();
    }else{
        LSUNode * lsu = dynamic_cast<LSUNode*>(candidate);
        maxDelay = lsu->maxDelay();
    }
    int Max_Vio =  latestTime  - _dfgEdgeAttr[edge->id()].endTime;
    curVio = Max_Vio;  //use Max_Vio, aims to use as less as possible delay units at the violated dfgNode
    std::stack<DFGNode *> backNodes;
    //first record the dfgNode which has violation
    std::map<int, DFGEdge*> gobackPath;
    while(curVio > 0){
        bool searchEnd = false;
        DFGNode* preNode = _dfg->node(curEdge->srcId());
        if(preNode->operation() == "INPUT" || preNode == dfgNode){
            break;
        }else{//@Yuan: if the node has other mapped down-stream nodes, it can not be scheduled
            for(auto out : preNode->outputs()){
                for(auto outnodes : out.second){
                    if(isMapped(_dfg->node(outnodes.first)) && outnodes.first !=curNode->id()){
                        searchEnd = true;
                        break;
                    }
                }
                if(searchEnd)
                    break;
            }
            if(searchEnd)
                break;
        }
        gobackPath[curNode->id()] = curEdge; // save current dfgNode and its input edge
        backNodes.push(curNode);
        curNode = preNode;
        int curEarliestTime = *(_dfgNodeAttr[curNode->id()].mappedtime.begin());
        int curUsage = _dfgNodeAttr[curNode->id()].DelayUsage;
        curVio = curVio - (maxDelay - curUsage);
        for(auto inEdges : curNode->inputEdges()){   //shedule the earliest edge
            DFGEdge* inEdge = _dfg->edge(inEdges.second);
            if(_dfgEdgeAttr[inEdge->id()].endTime == curEarliestTime){
                searchEnd = true;
                curEdge = inEdge;
            }
        }
        if(!searchEnd)
            break;
    }
    if(backNodes.empty()){// the up-stream nodes of candidate node can not be mapped
        return -1;
    }
   // backNodes.push(curNode); // save the begining node
    DFGNode* beginNode = curNode;
    ADGNode* curMappedNode = _dfgNodeAttr[beginNode->id()].adgNode;
    int beginADGNodeId = curMappedNode->id();
    int beginNode_earliestTime = *(_dfgNodeAttr[beginNode->id()].mappedtime.begin());
    int beginNode_latesttTime = *(_dfgNodeAttr[beginNode->id()].mappedtime.rbegin());
    int beginNodeUsage = _dfgNodeAttr[curNode->id()].DelayUsage;  //begin node current delay usage
    int curfreeDelay = maxDelay - beginNodeUsage;//how many delay units can be used in src node
    int cursrcExtrDelay = std::min(curVio + curfreeDelay, curfreeDelay);  //the small one is enough
    int srcNewlatestTime = beginNode_latesttTime + cursrcExtrDelay;
    int srcTargetdelayUsage = beginNodeUsage + cursrcExtrDelay;
    int actualUsage = srcTargetdelayUsage;
    //we need to check the begin node is avaulable in extra time
    for(int t = beginNode_earliestTime ; t <= srcNewlatestTime; t++){//to find the delay usage of the dfgNode that mapped at extra time
        int Time = t < 0 ? t + II : t;
        bool searchEnd = false;
        if(_t_adgNodeAttr[Time % II].count(beginADGNodeId)){    //if candidate ADGNode is mapped by other dfgNodes, get the delay usage of this dfgNode
            for(auto DFGNode : _t_adgNodeAttr[Time % II][beginADGNodeId].dfgNode){
                if(DFGNode != nullptr && DFGNode !=beginNode){
                    actualUsage =  _dfgNodeAttr[DFGNode->id()].DelayUsage;
                    searchEnd = true;
                    break;
                }
            }
            if(searchEnd){
                srcNewlatestTime = t - 1;
                break;
            }
        }

    }
    bool noExtraTime = srcNewlatestTime <= beginNode_latesttTime? true : false;//if noExtraTime = true, it means beginnode share ADGNode with other dfgNodes in its mapped time,so we can not change its mapping status
    if(noExtraTime){
        srcNewlatestTime = beginNode_latesttTime;
    }else{
        if(actualUsage > srcTargetdelayUsage ){//if the delay usage of the dfgNode (mapped at extra time) bigger than the usage of begin node, we can not overwrite the time
            int beginNode_actualUsasge = srcNewlatestTime - beginNode_earliestTime;
            for(int t = beginNode_latesttTime + 1; t <= srcNewlatestTime; t++){ //just use all the free time
                bool isExecute = t == srcNewlatestTime? true : false;
                if(!mapDfgNode(beginNode, curMappedNode, t, isExecute, 0, beginNode_earliestTime)){
                    srcNewlatestTime = t - 1;
                    _t_adgNodeAttr[(t-1)%II][beginADGNodeId].executeNode = beginNode;
                    break;
                }
            }
            beginNode_actualUsasge = srcNewlatestTime - beginNode_earliestTime;
            setDelayuse(beginNode, beginNode_actualUsasge);
        }else{
        //if the begin node's delay usage is large than the dfgNode mapped at extra time, the begin node only use the small numbers of delay unit
            bool Continue = true;
            int minFreeTime = srcNewlatestTime;
            //first we need to occupy the free time
            for(int t = beginNode_latesttTime + 1; t <= minFreeTime; t++){//just use all the free time
                if(!mapDfgNode(beginNode, curMappedNode, t, false, 0, beginNode_earliestTime)){
                    _t_adgNodeAttr[(t-1)%II][beginADGNodeId].executeNode = beginNode;
                    srcNewlatestTime = t - 1;
                    Continue = false;
                    break;
                }
            }
            setDelayuse(beginNode, srcNewlatestTime - beginNode_earliestTime);
            if(srcNewlatestTime > beginNode_latesttTime){// use free time, execute time is delayed
                _t_adgNodeAttr[srcNewlatestTime%II][beginADGNodeId].executeNode = beginNode;
            }
            if((srcNewlatestTime - beginNode_earliestTime) >= actualUsage)
                Continue = false;
            if(Continue){
                cursrcExtrDelay = actualUsage - (srcNewlatestTime - beginNode_earliestTime);
                for(int t = srcNewlatestTime + 1; t <= srcNewlatestTime + cursrcExtrDelay; t++){
                    bool isExecute = t == srcNewlatestTime + cursrcExtrDelay? true : false;
                    if(!mapDfgNode(beginNode, curMappedNode, t, isExecute, actualUsage, beginNode_earliestTime)){
                        for(int unmapt = srcNewlatestTime + 1; unmapt < t; unmapt++){ // can not map all the candidate time, fail to map ,we should delete all the mapping status that overwrite the occupied time
                            unuseADGNode(beginNode, curMappedNode, unmapt);
                        }
                        Continue = false;
                        break;
                    }
                }
            }
            if(Continue){//current dfgNode share the time at cndidate ADGNode with other dfgNodes
                _t_adgNodeAttr[srcNewlatestTime%II][beginADGNodeId].executeNode = nullptr;
                srcNewlatestTime = srcNewlatestTime + cursrcExtrDelay;
                _t_adgNodeAttr[srcNewlatestTime%II][beginADGNodeId].executeNode = beginNode;
                setDelayuse(beginNode, srcNewlatestTime - beginNode_earliestTime);
            }
        }
    }
    //set DFGNode extrdelay
    //_dfgNodeAttr[beginNode->id()].extrDelay = cursrcExtrDelay;
    /*std::cout << "[HETA] try to go back to fix violation for " << dfgNode->name() << " at ADGNode " << candidate->type() << "(" << candidate->id() << ") "\
    << " with II = " << II << std::endl;*/
    while(!backNodes.empty()){// try to reroute the path
          DFGNode * nextNode = backNodes.top();
          int nextNodeId =  nextNode->id();
          backNodes.pop();
          DFGEdge * rerouteEdge = gobackPath[nextNodeId];
          ADGNode * srcMappedNode = curMappedNode;
          if(nextNode == dfgNode){//remap the dfgNode that has violation
            unrouteDfgEdge(rerouteEdge, false);
            succeed = routeDfgEdge(rerouteEdge, srcMappedNode, candidate, srcNewlatestTime, &returnTime);
            if(!succeed){
              return -1;
            }
          }else{//in this stage, we need to use the defNode's delay unit as more as possible
            std::set<int> newMappingTime;
            curMappedNode = _dfgNodeAttr[nextNodeId].adgNode;
            //unmap next node in all its mapping time, and unmap the edge that is being scheduled
            for(int t = *(_dfgNodeAttr[nextNodeId].mappedtime.begin()); t <= *(_dfgNodeAttr[nextNodeId].mappedtime.rbegin()); t++){
              unuseADGNode(nextNode, curMappedNode, t);
            }
            unmapDfgNode(nextNodeId);
            unrouteDfgEdge(rerouteEdge, false); //unroute the edge that is being scheduled
            succeed = routeDfgEdge(rerouteEdge, srcMappedNode, curMappedNode, srcNewlatestTime, &returnTime);
            if(succeed){
                for(auto inputedge : nextNode->inputEdges()){//current node has new mapping time
                    if(_dfgEdgeAttr.count(inputedge.second))
                        newMappingTime.emplace(_dfgEdgeAttr[inputedge.second].endTime);
                }
                int curEarliestTime = *(newMappingTime.begin());
                int curEarlatestTime = *(newMappingTime.rbegin());
                srcNewlatestTime = curEarlatestTime;
                int minFreeTime = 0;
                delayUsage = curEarlatestTime - curEarliestTime;  //minimum demand of delay usage (must be satisfied)
                curfreeDelay = maxDelay - delayUsage; //we  need to make full use of current node's delay unit, so get the free usage
                for(int t = curEarliestTime; t <= curEarlatestTime + curfreeDelay; t++){//to find the delay usage of the dfgNode that mapped at a certain time (nextnode's demand)
                    int Time = t < 0 ? t + II : t;
                    bool searchEnd = false;
                    if(_t_adgNodeAttr[Time % II].count(curMappedNode->id())){    //if candidate ADGNode is mapped by other dfgNodes, get the delay usage of this dfgNode
                        for(auto DFGNode : _t_adgNodeAttr[Time % II][curMappedNode->id()].dfgNode){
                            if(DFGNode != nullptr && DFGNode != nextNode){
                                actualUsage =  _dfgNodeAttr[DFGNode->id()].DelayUsage;
                                searchEnd = true;
                                break;
                            }
                        }
                        if(searchEnd){
                            minFreeTime = t - 1;//minimun free time, before the ADGNode is mapped by other dfgNodes
                            break;
                        }
                    }

                }
                //if the free time can meet the delay usage demand, just use it
                if(minFreeTime >= curEarlatestTime){
                    //first, fill the free time to meet the minimum demand
                    for(int t = curEarliestTime; t<= curEarlatestTime; t++){//meet the minimum demand, if can not meet, quit
                        bool isExecute = t == curEarliestTime? true : false;
                        if(!mapDfgNode(nextNode, curMappedNode, t, isExecute, 0,  curEarliestTime)){
                            unmapFixDfgNode(nextNode);
                            succeed = false;
                            break;
                        }
                    }
                    if(!succeed)
                        break;
                   //use the extra free time
                    srcNewlatestTime = minFreeTime;
                    for(int t = curEarlatestTime + 1; t<= minFreeTime; t++){
                        if(!mapDfgNode(nextNode, curMappedNode, t, false, 0,  curEarliestTime)){
                            srcNewlatestTime = t-1;
                            break;
                        }
                    }
                    //try to share ADGNode to futher increase delay usage
                    if(srcNewlatestTime < minFreeTime){ // if we can not make full use of the free time, it also can not share ADGNodes
                        delayUsage = srcNewlatestTime - curEarliestTime;
                        setDelayuse(nextNode, delayUsage);//set delay usage
                        _t_adgNodeAttr[srcNewlatestTime%II][curMappedNode->id()].executeNode = nextNode;
                    }else{// begin try to share the ADGNode with other dfgNodes
                        delayUsage = srcNewlatestTime - curEarliestTime;
                        if(delayUsage >= actualUsage){ //can not share, because dfgNode has different delay usage
                            setDelayuse(nextNode, delayUsage);//set delay usage
                            _t_adgNodeAttr[srcNewlatestTime%II][curMappedNode->id()].executeNode = nextNode;
                        }else{
                            int extraTime =  actualUsage - delayUsage;
                            bool finish = true;
                            for(int t = srcNewlatestTime + 1; t <= srcNewlatestTime + extraTime; t++){
                                bool isExecute = t ==  srcNewlatestTime + extraTime? true : false;
                                if(!mapDfgNode(nextNode, curMappedNode, t, isExecute, actualUsage, curEarliestTime)){
                                    for(int unmapt = srcNewlatestTime + 1; unmapt < t; unmapt++){ // can not map all the candidate time, fail to map ,we should delete all the mapping status that overwrite the occupied time
                                        unuseADGNode(nextNode, curMappedNode, unmapt);
                                    }
                                    finish = false;
                                    break;
                                }
                            }
                            if(finish){// success share the ADGNode
                                setDelayuse(nextNode, actualUsage);//set delay usage
                                srcNewlatestTime += extraTime;
                                _t_adgNodeAttr[srcNewlatestTime%II][curMappedNode->id()].executeNode = nextNode;
                            }else{
                                setDelayuse(nextNode, delayUsage);//set delay usage
                                _t_adgNodeAttr[srcNewlatestTime%II][curMappedNode->id()].executeNode = nextNode;
                            }
                        }

                    }
                }else{//there is no enough free time to meet the delay usage demand, it should share the ADGNode with other dfgNodes
                    if(delayUsage > actualUsage){// can not share
                        unmapFixDfgNode(nextNode);
                        succeed = false;
                        break;
                    }else{
                        int extraTime =  actualUsage - delayUsage;
                        bool finish = true;
                        for(int t = curEarliestTime; t <= curEarlatestTime ; t++){// map the dfgNode, which sharing the ADGNode with other dfgNodes
                            bool isExecute = false;
                            if(t == curEarliestTime || t == curEarlatestTime)
                                isExecute = true;
                            if(!mapDfgNode(nextNode, curMappedNode, t, isExecute, actualUsage, curEarliestTime)){//if fail, just unmap everything
                                unmapFixDfgNode(nextNode);
                                finish = false;
                                break;
                            }
                        }
                        if(!finish){
                            succeed = false;
                            break;
                        }
                        for(int t = curEarlatestTime + 1; t <= curEarlatestTime + extraTime; t++){// map the dfgNode, which sharing the ADGNode with ither dfgNodes
                            bool isExecute = false;
                            if(t == curEarlatestTime + extraTime)
                                isExecute = true;
                            if(!mapDfgNode(nextNode, curMappedNode, t, isExecute, actualUsage, curEarliestTime)){//if fail, just unmap everything
                                unmapFixDfgNode(nextNode);
                                finish = false;
                                break;
                            }
                        }
                        if(finish){
                            srcNewlatestTime = curEarlatestTime + extraTime;
                            setDelayuse(nextNode, actualUsage);//set delay usage
                        }else{
                            succeed = false;
                            break;
                        }
                    }
                }
                if(!succeed)
                break;
            }else{
                unmapFixDfgNode(nextNode);
                break;
            }
          }
    }
    if(!succeed){
        return -1;
    }else{
        return returnTime;
    }

}


//return ADGNode usage in time
int Mapping::numUsedTime(int ADGnodeId){
    int usage = 0;
    for(int t = 0; t < II; t++){
        if(_t_adgNodeAttr[t].count(ADGnodeId)){
            for(auto DFGNode : _t_adgNodeAttr[t][ADGnodeId].dfgNode){
                if(DFGNode){
                    usage += 1;
                    break;
                }
            }
        }

    }
    return usage;
}
//set the usage of delay unit of dfgNode
void Mapping::setDelayuse(DFGNode * dfgNode, int usage){
     int dfgId =  dfgNode->id();
     if(_dfgNodeAttr.count(dfgId)){
       _dfgNodeAttr[dfgId].DelayUsage = usage;
     }
}


//get the usage of delay unit of dfgNode
int Mapping::getDelayuse(DFGNode * dfgNode){
     int dfgId =  dfgNode->id();
     if(_dfgNodeAttr.count(dfgId)){
       return _dfgNodeAttr[dfgId].DelayUsage;
     }else{
        return 0;
     }
}

//get DFGNode latest mapping time
int Mapping::getLatest(int DFGNodeId){
    if(_dfgNodeAttr.count(DFGNodeId)){
        return *(_dfgNodeAttr[DFGNodeId].mappedtime.rbegin());
    }else{
        return 0;
    }
}

// @Yuan: update the edge endtime (just for cycle, which edge's endtime needs to subtract II)
void Mapping::updateEndTime(DFGEdge* edge, int newEndTime){
    int edgeId = edge->id();
    if(_dfgEdgeAttr.count(edgeId)){
        _dfgEdgeAttr[edgeId].endTime = newEndTime;
    }
}


/*const ADGNodeAttr& Mapping::t_adgNodeAttr(int id, int time){
    if(_t_adgNodeAttr[time%II].count(id)){
        return _t_adgNodeAttr[time%II][id];
    }else{
        return null;
    }

}*/


////just for test, print the available Nodes
void Mapping::printAva(){
    for(auto elem : _adg->nodes()){
        if(elem.second->type() == "GIB")
            continue;
        std::set<int> avaTime;
        for(int t = 0; t<II; t++){
            bool occupy =false;
            for(auto dfgNode : _t_adgNodeAttr[t][elem.second->id()].dfgNode){
                if(dfgNode != nullptr)
                   occupy = true;
            }
            if(!occupy)
              avaTime.emplace(t);
        }
        std::cout << elem.second->type() << "(" << elem.second->id() << ") available time: ";
        for(auto t: avaTime){
            std::cout  << t ;
        }
        std::cout  <<std::endl;
    }
    /*for(auto edge : _vioDfgEdges){
        int vio =edge.second; // maybe add multiple nodes according to vio
        int eid = edge.first;
        std::cout << "vioedge: " << eid << " , " << vio << std::endl;
        if(II>5 && vio > 0)
            exit(0);
    }*/
}


void Mapping::clearADGNodeUsage(){
    auto adg = getADG();
    for(auto& elem : adg->nodes()){
        auto node = elem.second;
        node->clearUsage();
    }
}


int Mapping::getADGNodeHisUsage(int ADGNodeId, int DFGNodeId){
    auto adg = getADG();
    auto adgNode = adg->node(ADGNodeId);
    return adgNode->getUsage(DFGNodeId);
}
