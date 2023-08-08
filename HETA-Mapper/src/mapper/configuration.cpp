
#include "mapper/configuration.h"
#include <cmath>

// get config data for LSU, return<LSB-location, CfgData>
std::map<int, CfgData> Configuration::getLsuCfgData(LSUNode* node, int time, int II){
    int adgNodeId = node->id();
    ADG* subAdg = node->subADG();
    auto t_adgNodeAttr = _mapping->t_adgNodeAttr();
    ADGNodeAttr adgNodeAttr;
    if(t_adgNodeAttr[time%II][adgNodeId].dfgNode.empty()){
        return {};
    }else{
        adgNodeAttr = t_adgNodeAttr[time%II][adgNodeId];
    }
    DFGNode* dfgNode;
    if(adgNodeAttr.executeNode == nullptr){// if the ADGNode does not have execute node, just use one dfgNode
     for(auto dfgnode : adgNodeAttr.dfgNode){
        if(dfgnode != nullptr){
            dfgNode = dfgnode;
            break;
        }
     }
    }else{
        dfgNode = adgNodeAttr.executeNode;
    }
    if(dfgNode == nullptr){
        return {};
    }
    //std::cout << " Excually node of LSU : " << dfgNode->name() << std::endl;
    std::map<int, CfgData> cfg;
    // operation
    int opc ;
    if(dfgNode->operation() == "LOAD"){
        opc = 1;
    }else{
        opc = 2;
    }
    int sramId = -1;
    // bool flag = false;
    std::set<int> usedOperands;
    int dfgNodeId = dfgNode->id();
    int maxDly = 0;
    for(auto& elem : dfgNode->inputEdges()){
        int eid = elem.second;
        auto& edgeAttr = _mapping->dfgEdgeAttr(eid);
        //@Yuan: delay cycles = dfgNode latest mappingTime - edge arrived time
        int edgeArrivedTime =edgeAttr.endTime;
        int latestTime = *(_mapping->dfgNodeAttr(dfgNodeId).mappedtime.rbegin());
        int delay = latestTime - edgeArrivedTime; // delay cycles
        maxDly = std::max(maxDly, delay);
        int inputIdx = edgeAttr.edgeLinks.rbegin()->srcPort; // last edgeLInk, dst port
        auto muxPair = subAdg->input(inputIdx).begin(); // one input only connected to one Mux
        int muxId = muxPair->first;
        int muxCfgData = muxPair->second;
        auto mux = subAdg->node(muxId);
        int dlypipeId = mux->output(0).begin()->first;
        auto dlypipe = subAdg->node(dlypipeId);
        auto SRAMPair = dlypipe->output(0).begin();
        sramId = SRAMPair->first;
        usedOperands.emplace(SRAMPair->second); // operand index
        CfgDataLoc muxCfgLoc = node->configInfo(muxId);
        CfgData muxCfg((muxCfgLoc.high - muxCfgLoc.low + 1), (uint32_t)muxCfgData);
        cfg[muxCfgLoc.low] = muxCfg;
        CfgDataLoc dlyCfgLoc = node->configInfo(dlypipeId);
        CfgData dlyCfg(dlyCfgLoc.high - dlyCfgLoc.low + 1, (uint32_t)delay);
        cfg[dlyCfgLoc.low] = dlyCfg;
    }
    if(sramId == -1){ // in case that some node has no input (not completed, should be avoided)
        auto muxPair = subAdg->input(0).begin(); // one input only connected to one Mux
        int muxId = muxPair->first;
        auto mux = subAdg->node(muxId);
        int dlypipeId = mux->output(0).begin()->first;
        auto dlypipe = subAdg->node(dlypipeId);
        auto sramPair = dlypipe->output(0).begin();
        sramId = sramPair->first;
    }
    CfgDataLoc sramCfgLoc = node->configInfo(sramId);
    CfgData sramCfg(sramCfgLoc.high - sramCfgLoc.low + 1, (uint32_t)opc);
    cfg[sramCfgLoc.low] = sramCfg;
    // Constant
    if(dfgNode->hasImm()){
        // find unused operand
        int i = 0;
        for(; i < dfgNode->numInputs(); i++){
            if(!usedOperands.count(i)){
                break;
            }
        }
        auto sram = subAdg->node(sramId);
        auto dlypipe = subAdg->node(sram->input(i).first); // used default delay
        int muxId = dlypipe->input(0).first;
        for(auto& elem : subAdg->node(muxId)->inputs()){
            int id = elem.second.first;
            if(id == subAdg->id()) continue;
            if(subAdg->node(id)->type() == "Const"){
                CfgDataLoc muxCfgLoc = node->configInfo(muxId);
                CfgData muxCfg(muxCfgLoc.high - muxCfgLoc.low + 1, (uint32_t)elem.first);
                cfg[muxCfgLoc.low] = muxCfg;
                CfgDataLoc constCfgLoc = node->configInfo(id);
                int len = constCfgLoc.high - constCfgLoc.low + 1;
                CfgData constCfg(len);
                uint64_t imm = dfgNode->imm();
                while(len > 0){
                    constCfg.data.push_back(uint32_t(imm&0xffffffff));
                    len -= 32;
                    imm >> 32;
                }
                cfg[constCfgLoc.low] = constCfg;
                break;
            }
        }
    }
    //[HETA]: update the maxdelay usage of GPE
    node->setCurMaxDlyUsage(maxDly);
    return cfg;
}

// get config data for GPE, return<LSB-location, CfgData>
std::map<int, CfgData> Configuration::getGpeCfgData(GPENode* node, int time, int II){
    int adgNodeId = node->id();
    ADG* subAdg = node->subADG();
    auto t_adgNodeAttr = _mapping->t_adgNodeAttr();
    ADGNodeAttr adgNodeAttr;
    if(t_adgNodeAttr[time%II][adgNodeId].dfgNode.empty()){
        return {};
    }else{
        adgNodeAttr = t_adgNodeAttr[time%II][adgNodeId];
    }
    //find the execute node of current ADGNode at current time
    DFGNode* dfgNode;
    if(adgNodeAttr.executeNode == nullptr){// if the ADGNode does not have execute node, just use one dfgNode
     for(auto dfgnode : adgNodeAttr.dfgNode){
        if(dfgnode != nullptr){
            dfgNode = dfgnode;
            break;
        }
     }
    }else{
        dfgNode = adgNodeAttr.executeNode;
    }
    if(dfgNode == nullptr){
        return {};
    }
    std::map<int, CfgData> cfg;
    // operation
    int opc = Operations::OPC(dfgNode->operation());
    int dfgNodeId = dfgNode->id();
    int aluId = -1;
    // bool flag = false;
    node->setCurOpsetUsage(dfgNode->operation());
    std::set<int> usedOperands;
    int maxDly = 0;
    for(auto& elem : dfgNode->inputEdges()){
        int eid = elem.second;
        // if the executeNode has self-cycle, we should handle it specially
        auto edge = _mapping->getDFG()->edge(eid);
        if(edge->isCycle() && dfgNode->selfloop() != -1){
            auto& edgeAttr = _mapping->dfgEdgeAttr(eid);
            //@Yuan: delay cycles = dfgNode latest mappingTime - edge arrived time
            int edgeArrivedTime =edgeAttr.endTime;
            int latestTime = *(_mapping->dfgNodeAttr(dfgNodeId).mappedtime.rbegin());
            int delay = latestTime - edgeArrivedTime; // delay cycles
            maxDly = std::max(maxDly, delay);
            int inPort = (edgeAttr.edgeLinks.rbegin())->srcPort;
            auto muxPair = subAdg->input(inPort).begin(); // one input only connected to one Mux
            int muxId = muxPair->first;
            auto mux = subAdg->node(muxId);
            int muxInputIdx = mux->inputs().size();
            int muxCfgData = muxInputIdx - 1; // RF connect to the last input of MUX
            int dlypipeId = mux->output(0).begin()->first;
            auto dlypipe = subAdg->node(dlypipeId);
            auto aluPair = dlypipe->output(0).begin();
            aluId = aluPair->first;
            usedOperands.emplace(aluPair->second); // operand index
            CfgDataLoc muxCfgLoc = node->configInfo(muxId);
            CfgData muxCfg((muxCfgLoc.high - muxCfgLoc.low + 1), (uint32_t)muxCfgData);
            cfg[muxCfgLoc.low] = muxCfg;
            CfgDataLoc dlyCfgLoc = node->configInfo(dlypipeId);
            CfgData dlyCfg(dlyCfgLoc.high - dlyCfgLoc.low + 1, (uint32_t)delay);
            cfg[dlyCfgLoc.low] = dlyCfg;
            continue;
        }
        auto& edgeAttr = _mapping->dfgEdgeAttr(eid);
        //@Yuan: delay cycles = dfgNode latest mappingTime - edge arrived time
        int edgeArrivedTime =edgeAttr.endTime;
        int latestTime = *(_mapping->dfgNodeAttr(dfgNodeId).mappedtime.rbegin());
        int delay = latestTime - edgeArrivedTime; // delay cycles
        maxDly = std::max(maxDly, delay);
        int inputIdx = edgeAttr.edgeLinks.rbegin()->srcPort; // last edgeLInk, dst port
        auto muxPair = subAdg->input(inputIdx).begin(); // one input only connected to one Mux
        int muxId = muxPair->first;
        int muxCfgData = muxPair->second;
        auto mux = subAdg->node(muxId);
        int dlypipeId = mux->output(0).begin()->first;
        auto dlypipe = subAdg->node(dlypipeId);
        auto aluPair = dlypipe->output(0).begin();
        aluId = aluPair->first;
        usedOperands.emplace(aluPair->second); // operand index
        CfgDataLoc muxCfgLoc = node->configInfo(muxId);
        CfgData muxCfg((muxCfgLoc.high - muxCfgLoc.low + 1), (uint32_t)muxCfgData);
        cfg[muxCfgLoc.low] = muxCfg;
        CfgDataLoc dlyCfgLoc = node->configInfo(dlypipeId);
        CfgData dlyCfg(dlyCfgLoc.high - dlyCfgLoc.low + 1, (uint32_t)delay);
        cfg[dlyCfgLoc.low] = dlyCfg;
    }
    if(aluId == -1){ // in case that some node has no input (not completed, should be avoided)
        auto muxPair = subAdg->input(0).begin(); // one input only connected to one Mux
        int muxId = muxPair->first;
        auto mux = subAdg->node(muxId);
        int dlypipeId = mux->output(0).begin()->first;
        auto dlypipe = subAdg->node(dlypipeId);
        auto aluPair = dlypipe->output(0).begin();
        aluId = aluPair->first;
    }
    CfgDataLoc aluCfgLoc = node->configInfo(aluId);
    CfgData aluCfg(aluCfgLoc.high - aluCfgLoc.low + 1, (uint32_t)opc);
    cfg[aluCfgLoc.low] = aluCfg;
    // Constant
    if(dfgNode->hasImm()){
        // find unused operand
        int i = 0;
        for(; i < dfgNode->numInputs(); i++){
            if(!usedOperands.count(i)){
                break;
            }
        }
        auto alu = subAdg->node(aluId);
        auto dlypipe = subAdg->node(alu->input(i).first); // used default delay
        int muxId = dlypipe->input(0).first;
        for(auto& elem : subAdg->node(muxId)->inputs()){
            int id = elem.second.first;
            if(id == subAdg->id()) continue;
            if(subAdg->node(id)->type() == "Const"){
                CfgDataLoc muxCfgLoc = node->configInfo(muxId);
                CfgData muxCfg(muxCfgLoc.high - muxCfgLoc.low + 1, (uint32_t)elem.first);
                cfg[muxCfgLoc.low] = muxCfg;
                CfgDataLoc constCfgLoc = node->configInfo(id);
                int len = constCfgLoc.high - constCfgLoc.low + 1;
                CfgData constCfg(len);
                uint64_t imm = dfgNode->imm();
                while(len > 0){
                    constCfg.data.push_back(uint32_t(imm&0xffffffff));
                    len -= 32;
                    imm >> 32;
                }
                cfg[constCfgLoc.low] = constCfg;
                if(dfgNode->selfloop() != -1){
                    int latestTime = *(_mapping->dfgNodeAttr(dfgNodeId).mappedtime.rbegin());
                    if(II == 1 && latestTime == 0){
                        int delay = 1;
                        maxDly = std::max(maxDly, delay);
                        auto mux = subAdg->node(muxId);
                        int dlypipeId = mux->output(0).begin()->first;
                        auto dlypipe = subAdg->node(dlypipeId);
                        CfgDataLoc dlyCfgLoc = node->configInfo(dlypipeId);
                        CfgData dlyCfg(dlyCfgLoc.high - dlyCfgLoc.low + 1, (uint32_t)delay);
                        cfg[dlyCfgLoc.low] = dlyCfg;
                    }
                }
                break;
            }
        }
    }
    //[HETA]: update the maxdelay usage of GPE
    node->setCurMaxDlyUsage(maxDly);
    return cfg;
}


// get config data for GIB, return<LSB-location, CfgData>
std::map<int, CfgData> Configuration::getGibCfgData(ADGNode* node, int time, int II){
    int adgNodeId = node->id();
    ADG* subAdg = node->subADG();
    auto t_adgNodeAttr = _mapping->t_adgNodeAttr();
    ADGNodeAttr adgNodeAttr;
    std::map<int, ADGNodeAttr> map_adgNodeAttr;
    if(t_adgNodeAttr.count(time%II)){
        map_adgNodeAttr = t_adgNodeAttr[time%II];
        if(map_adgNodeAttr.count(adgNodeId)){
            adgNodeAttr = map_adgNodeAttr[adgNodeId];
        }else{
            return {};
        }
    }else{
        return {};
    }
    auto& passEdges = adgNodeAttr.dfgEdgePass;
    //update the utilization of the GIB
    GIBNode* gib = (dynamic_cast<GIBNode*>(node));
    gib->addUti(passEdges.size());

    if(passEdges.empty()){
        return {};
    }
    std::map<int, CfgData> cfg;
    for(auto& elem : passEdges){
        int muxId = subAdg->output(elem.dstPort).first; // one output connected to one mux
        if(muxId == subAdg->id()){ // actually connected to input port
            continue;
        }
        auto mux = subAdg->node(muxId);
        // find srcPort
        for(auto in : mux->inputs()){
            if(in.second.second == elem.srcPort){
                CfgDataLoc muxCfgLoc = node->configInfo(muxId);
                CfgData muxCfg(muxCfgLoc.high - muxCfgLoc.low + 1, (uint32_t)in.first);
                cfg[muxCfgLoc.low] = muxCfg;
                break;
            }
        }
    }
    return cfg;
}


//@Yuan: get config data for IOB, return<LSB-location, CfgData>
std::map<int, CfgData> Configuration::getIobCfgData(ADGNode* node, int time, int II){
    int adgNodeId = node->id();
    ADG* subAdg = node->subADG();
    auto t_adgNodeAttr = _mapping->t_adgNodeAttr();
    ADGNodeAttr adgNodeAttr;
    if(t_adgNodeAttr[time%II][adgNodeId].dfgNode.empty()){
        return {};
    }else{
        adgNodeAttr = t_adgNodeAttr[time%II][adgNodeId];
    }
    auto& mappedNodes = adgNodeAttr.dfgNode;
    if(mappedNodes.empty()){
        return {};
    }
    std::map<int, CfgData> cfg;
    for(auto& elem : mappedNodes){
        auto& mappedNodeAttr = _mapping->dfgNodeAttr(elem->id());
        int muxId = subAdg->output(mappedNodeAttr.dstPort).first; // one output connected to one mux
        if(muxId == subAdg->id()){ // actually connected to input port
            continue;
        }
        auto mux = subAdg->node(muxId);
        // find srcPort
        for(auto in : mux->inputs()){
            if(in.second.second == *(mappedNodeAttr.srcPort.begin())){
                CfgDataLoc muxCfgLoc = node->configInfo(muxId);
                CfgData muxCfg(muxCfgLoc.high - muxCfgLoc.low + 1, (uint32_t)in.first);
                cfg[muxCfgLoc.low] = muxCfg;
                break;
            }
        }
    }
    return cfg;
}

// get config data for ADG node
void Configuration::getNodeCfgData(ADGNode* node, std::vector<CfgDataPacket>& cfg, int time, int II){
    std::map<int, CfgData> cfgMap;

    
    if(node->type() == "GPE"){
        cfgMap = getGpeCfgData(dynamic_cast<GPENode*>(node), time, II);
    }else if(node->type() == "GIB"){
        cfgMap = getGibCfgData(node, time, II);
    }else if((node->type() == "IB") || (node->type() == "OB")){
        cfgMap = getIobCfgData(node, time, II);
    }else if(node->type() == "LSU"){
         cfgMap = getLsuCfgData(dynamic_cast<LSUNode*>(node), time, II);
    }
    if(cfgMap.empty()){
        return;
    }
    //std::cout << "config255 a ha~~~! " << node->type() << ":" <<time << ":" << II<<std::endl;
        //std::cout << "config265 a ha~~~! " << node->type() <<","<<cfgMap.size()<< std::endl;
    ADG* adg = _mapping->getADG();
    int cfgDataWidth = adg->cfgDataWidth();
    int totalLen = cfgMap.rbegin()->first + cfgMap.rbegin()->second.len;//last one sub-unit LSB and config length
    int num = (totalLen+31)/32;// divide 32 (round up)
    std::vector<uint32_t> cfgDataVec(num, 0);
    std::set<uint32_t> addrs;
    for(auto& elem : cfgMap){ // std::map auto-sort keys
        int lsb = elem.first;
        int len = elem.second.len;
        auto& data = elem.second.data;
        // cache valid address
        uint32_t targetAddr = lsb/cfgDataWidth;
        int addrNum = (len + (lsb%cfgDataWidth) + cfgDataWidth - 1)/cfgDataWidth;
        for(int i = 0; i < addrNum; i++){
            addrs.emplace(targetAddr+i);
        }
        // cache data from 0 to MSB
        int targetIdx = lsb/32;
        int offset = lsb%32;
        uint64_t tmpData = data[0];
        int dataIdx = 0;
        int dataLenLeft = 32;
        while(len > 0){
            if(len <= 32 - offset){
                len = 0;
                cfgDataVec[targetIdx] |= (tmpData << offset);
            }else{
                dataLenLeft -= 32 - offset;
                cfgDataVec[targetIdx] |= (tmpData << offset);
                targetIdx++;
                dataIdx++;
                tmpData >>= 32 - offset;
                if(dataIdx < data.size()){
                    tmpData |= data[dataIdx] << dataLenLeft;
                    dataLenLeft += 32;
                }
                len -= 32 - offset;
                offset = 0;
            }
        }
    }
    // construct CfgDataPacket
    int cfgBlkOffset = adg->cfgBlkOffset();
    int cfgBlkIdx = node->cfgBlkIdx();
    int maxII = adg->maxII();
    int IIOffset= 0;
    if(maxII == 1){
        IIOffset = 1;
    }else{
        IIOffset = log(maxII) / log(2); //IIOffset = log2(maxII)
    }
    // int x = node->x();
    uint32_t highAddr = uint32_t(cfgBlkIdx << (cfgBlkOffset + 2*IIOffset));
    int n;
    int mask;
    if(cfgDataWidth >= 32){
        assert(cfgDataWidth%32 == 0);
        n = cfgDataWidth/32;
    }else{
        assert(32%cfgDataWidth == 0);
        n = 32/cfgDataWidth;
        mask = (1 << cfgDataWidth) - 1;
    }
    for(auto addr : addrs){
        CfgDataPacket cdp(highAddr | (addr << 2*IIOffset) | (time << IIOffset) | (II - 1));
        if(cfgDataWidth >= 32){
            int size = cfgDataVec.size();
            for(int i = 0; i < n; i++){
                int idx = addr*n+i;
                uint32_t data = (idx < size)? cfgDataVec[idx] : 0;
                cdp.data.push_back(data);
            }
        }else{
            uint32_t data = (cfgDataVec[addr/n] >> ((addr%n)*cfgDataWidth)) & mask;
            cdp.data.push_back(data);
        }
        cfg.push_back(cdp);
    }
}

//@Yuan: generate each cycle's config data
// get config data for ADG
void Configuration::getCfgData(std::vector<CfgDataPacket>& cfg, int time, int II){
    cfg.clear();
    for(auto& elem : _mapping->getADG()->nodes()){
        getNodeCfgData(elem.second, cfg, time, II);
    }
}


// dump config data
void Configuration::dumpCfgData(std::ostream& os){
    ADG* adg = _mapping->getADG();
    int cfgAddrWidth = adg->cfgAddrWidth();
    int cfgDataWidth = adg->cfgDataWidth();
    int addrWidthHex = (cfgAddrWidth+3)/4;
    int dataWidthHex = std::min(cfgDataWidth/4, 8);
    int II = _mapping->getII();
    std::vector<CfgDataPacket> cfg;
    std::map<int, std::vector<CfgDataPacket>> t_cfg;  // <cycle, cfgdata>
    //generate config data
    for(int time = 0; time < II; time++){
        //std::cout << " generate bitstream of t: " << time << std::endl; 
        getCfgData(t_cfg[time], time, II);
        os << std::hex;
        for(auto& cdp : t_cfg[time]){
            os << std::setw(addrWidthHex) << std::setfill('0') << (cdp.addr) << " ";
            for(int i = cdp.data.size() - 1; i >= 0; i--){
                os << std::setw(dataWidthHex) << std::setfill('0') << cdp.data[i];
            }
            os << std::endl;
        }
    }
    os << std::dec;
}
