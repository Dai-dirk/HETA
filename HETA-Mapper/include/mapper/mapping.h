#ifndef __MAPPING_H__
#define __MAPPING_H__

#include <iostream>
#include <fstream>
#include <algorithm>
#include <queue>
#include <stack>
#include "adg/adg.h"
#include "dfg/dfg.h"

// DFG node attributes used for mapping
struct DFGNodeAttr
{
    int minLat = 0; // min latency of the input port
    int maxLat = 0; // max latency of the input port
    int lat = 0;    // latency of the output port
    int extrDelay = 0; // extra delay, used to elimate timing violation
    int DelayUsage = 0;
    // int vio = 0;
    std::set<int> mappedtime ;
    //@Yuan: merge from DFGOPAttr
    int adgIOPort = 0; // mapped ADG Input/Outut port
    std::set<int> srcPort ; // source port of the passed node
    int dstPort = 0; // destination port of the passed node
    ADGNode* adgNode = nullptr;
};

// Edge link attributes : pass-through ADG node
// including inter-node and intra-node links
struct EdgeLinkAttr // internal link
{
    int srcPort; // source port of the passed node
    int dstPort; // destination port of the passed node
    int mappingtime; //@Yuan
    ADGNode* adgNode; // pass-through node or src/dst node
};

// DFG edge attributes used for mapping
struct DFGEdgeAttr
{
    int lat = 0;
    // int latNoDelay; // not including the delay pipe latency
    int delay = 0; // delay pipe latency
    int vio = 0;
    int endTime;
    std::vector<EdgeLinkAttr> edgeLinks;
};

// pass-through DFG edge attributes, used for ADGNodeAttr
struct DfgEdgePassAttr
{
    int srcPort; // source port of the passed node
    int dstPort; // destination port of the passed node
    DFGEdge* edge;
};


struct DFGIOAttr
{
    int adgIOPort; // mapped ADG Input/Outut port
    int lat; // DFG I/O Port latency
    std::set<int> routedEdgeIds; // IDs of the routed DFG edges
};

// ADG node attributes used for mapping
struct ADGNodeAttr
{
    std::vector<DFGNode*> dfgNode; // dfgNode and dfgEdgePass are mutually exclusive
    std::vector<DfgEdgePassAttr> dfgEdgePass;
    std::map<int, bool> inPortUsed;
    std::map<int, bool> outPortUsed;
    //bool isExcute = false;   //if the ADGNode is mapped at DFGNode excute time
    DFGNode * executeNode = nullptr;
    std::map<int, std::pair<int, int>> inoutPortused; //@Yuan: DFGNode id, (inport-idex, outport-index)
};

// @Yuan: ADG node usage attributes
struct ADGNodeUsageAttr
{
    int _currentConfigMem = 0;
    std::map<int, bool> _isOccuppied;      // <mapped time, flag>
    std::map<int, std::vector<DFGNode*> > _mapped_dfg;  //  <mapped time, DFGNode>
};


// // ADG link attributes used for mapping
// // one link can route multiple edges, but they should have the same srcId and srcPortIdx
// struct ADGLinkAttr
// {
//     std::vector<DFGEdge*> dfgEdges;
// };


// Mapping App. DFG to CGRA ADG
// 1. provide basic mapping kit,
// 2. cache mapping results
class Mapping
{
private:
    ADG* _adg; // from outside, not delete here
    DFG* _dfg; // from outside, not delete here
    int _totalViolation = 0; // total edge latency violation
    int _maxViolation; // max edge latency violation
    int _maxLat;    // max latency of DFG
    int _maxLatNodeId; // DFG node with max latency
    // int _maxLatMis; // max timing mismatch
    int _numNodeMapped = 0; // number of mapped DFG nodes
    // the ADG information of each mapped DFG node
    std::map<int, DFGNodeAttr> _dfgNodeAttr;
    // the ADG information of each mapped DFG edge
    std::map<int, DFGEdgeAttr> _dfgEdgeAttr;
    // the ADG information of each mapped DFG input ports
    std::map<int, DFGIOAttr> _dfgInputAttr;
    // the ADG information of each mapped DFG output ports
    std::map<int, DFGIOAttr> _dfgOutputAttr;
    // the DFG information of each occupied ADG node
    std::map<int, ADGNodeUsageAttr> _adgNodeUsageAttr;
    // the DFG information of each occupied ADG node
    std::map<int, std::map<int, ADGNodeAttr>> _t_adgNodeAttr;
    // // the DFG information of each occupied ADG link
    // std::map<int, ADGLinkAttr> _adgLinkAttr;

    // DFG edges with latency violation
    std::map<int, int> _vioDfgEdges; // < edge-id, violation value>
    //@Yuan: mapping time save by node's ALAP <ALAP, <nodeid, average time>>
    std::map<int, std::map<int, int>> _ALAPTime;
    // route DFG edge to passthrough ADG node
    // if srcPort/dstPort == -1, auto-assign port; else assign provided port
    // one internal link can route multiple edges, but they should have the same srcId and srcPortIdx
    // isTry: just try to route, not change the routing status
    //@Yuan: add time, ADGNode usually be GIB, find an available outport in given time
    bool routeDfgEdgePass(DFGEdge* edge, ADGNode* passNode, int srcPort, int dstPort, bool isTry, int time);
    // route DFG edge between srcNode and dstNode
    // dstPortRange: the input port index range of the dstNode
    // find a routable path from srcNode to dstNode/OB by BFS
    // dstNode = nullptr: OB
    bool routeDfgEdgeFromSrc(DFGEdge* edge, ADGNode* srcNode, ADGNode* dstNode, const std::set<int>& dstPortRange, int pre_time, int* returTime, bool isTry);
    // find a routable path from dstNode to srcNode/IB by BFS
    // srcNode = nullptr: IB
    bool routeDfgEdgeFromDst(DFGEdge* edge, ADGNode* srcNode, ADGNode* dstNode, const std::set<int>& dstPortRange, int pre_time, bool isTry);
    // find the available input ports in the dstNode to route edge
    std::set<int> availDstPorts(DFGEdge* edge, ADGNode* dstNode);

protected:
    int II = 1;
    int timeBoundary = 1;

public:
    Mapping(ADG* adg, DFG* dfg): _adg(adg), _dfg(dfg) {}
    ~Mapping(){}
    // void setDFG(DFG* dfg){ _dfg = dfg; }
    DFG* getDFG(){ return _dfg; }
    // void setADG(ADG* adg){ _adg = adg; }
    ADG* getADG(){ return _adg; }
    const DFGNodeAttr& dfgNodeAttr(int id){ return _dfgNodeAttr[id]; }
    const DFGEdgeAttr& dfgEdgeAttr(int id){ return _dfgEdgeAttr[id]; }
    const std::map<int, std::map<int, ADGNodeAttr>> t_adgNodeAttr(){ return _t_adgNodeAttr; }
    const DFGIOAttr& dfgInputAttr(int idx){ return _dfgInputAttr[idx]; };
    const DFGIOAttr& dfgOutputAttr(int idx){ return _dfgOutputAttr[idx]; };
    // reset mapping intermediate result and status
    void reset();
    // if this input port of this ADG node is used
    bool isAdgNodeInPortUsed(int nodeId, int portIdx);
    // if this output port of this ADG node is used
    bool isAdgNodeOutPortUsed(int nodeId, int portIdx);
    //@Yuan: new
    // if this output port of this ADG node is used
    bool isAdgNodeOutPortUsed(int time, int nodeId, int portIdx);
    // if this input port of this ADG node is used
    bool isAdgNodeInPortUsed(int time, int nodeId, int portIdx);
    //@Yuan: new for GPE/LSU
    bool isAdgNodeInPortUsed(int time, int nodeId, int dstNodeId, int portIdx);
    // if the DFG node is already mapped
    bool isMapped(DFGNode* dfgNode);
    // if the ADG node is already mapped
    bool isMapped(ADGNode* adgNode);
    // if the IB node is available
    bool isIBAvail(ADGNode* adgNode);
    // if the OB node is available
    bool isOBAvail(ADGNode* adgNode);
    // if the DFG input port is mapped
    bool isDfgInputMapped(int idx);
    // if the DFG output port is mapped
    bool isDfgOutputMapped(int idx);
    // occupied ADG node of this DFG node
    ADGNode* mappedNode(DFGNode* dfgNode);
    // mapped DFG node of this ADG node
    DFGNode* mappedNode(ADGNode* adgNode);
    //@Yuan: pipeline map DFG node to ADG node, not assign ports, but check if there is constant operand
    bool mapDfgNode(DFGNode* dfgNode, ADGNode* adgNode,  int mappingTime, bool isExecute, int delayUsage, int startTime);
    //map DFG node to ADG node, not assign ports, but check if there is constant operand
    bool mapDfgNode(DFGNode* dfgNode, ADGNode* adgNode, int mappingTime, bool isExecute);
    //@Yuan: map IO DFG node to ADG node
    bool mapOutDfgNode(DFGNode* dfgNode, ADGNode* adgNode,  int mappingTime);
    //@Yuan: map INPUT node to ADG node, with certain src/dst ports
    bool mapInDfgNode(DFGNode* dfgNode, ADGNode* adgNode, int mappingTime, int srcPort, int dstPort,  int adgIOPort, bool isTry);
    // unmap DFG node
    void unmapDfgNode(DFGNode* dfgNode);
    // unmap recall DFG node
    void unmapFixDfgNode(DFGNode* dfgNode);
    //@Yuan: delete the _dfgNodeAttr status
    void unmapDfgNode(int DFGId);
    // unuse ADG node in given time
    void unuseADGNode(DFGNode* dfgNode, ADGNode* adgNode, int time);
    // // if the ADG link is already routed
    // bool isRouted(ADGLink* link);
    // routed DFG edge links, including inter-node and intra-node links
    const std::vector<EdgeLinkAttr>& routedEdgeLinks(DFGEdge* edge);
    // // route DFG edge to ADG link
    // // one link can route multiple edges, but they should have the same srcId and srcPortIdx
    // bool routeDfgEdge(DFGEdge* edge, ADGLink* link);
    // route DFG edge between srcNode and dstNode
    //@Yuan: should know the srcnode mapping time, return the dstnode mapping time
    // find a routable path from srcNode to dstNode by BFS
    bool routeDfgEdge(DFGEdge* edge, ADGNode* srcNode, ADGNode* dstNode, int preTime, int* finalTime);
    // route DFG edge between adgNode and IOB
    // is2Input: whether connected to IB or OB
    //bool routeDfgEdge(DFGEdge* edge, ADGNode* adgNode, bool is2Input);
    //@Yuan : DFG edge between adgNode and IOB
    // curTime: current adgNode mapping time, will decrease along the path
    bool routeDfgEdge(DFGEdge* edge, ADGNode* adgNode, int curTime);
    // unroute DFG edge
    void unrouteDfgEdge(DFGEdge* edge, bool isVio);
    // if succeed to map all DFG nodes
    bool success();
    // total/max edge length (link number)
    void getEdgeLen(int& totalLen, int& maxLen);
    // assign DFG IO to ADG IO according to mapping result
    // post-processing after mapping
    void assignDfgIO();


    // ==== Data synchronization: schedule latencies of DFG node inputs and outputs >>>>>>>>>>>>>
    int totalViolation(){ return _totalViolation; }
    int maxViolation(){ return _maxViolation; }
    int maxLat(){ return _maxLat; }
    void setMaxLat(int maxLat){  _maxLat = maxLat; }
    // int maxLatMis(){ return _maxLatMis; }
    // // reset the latency bounds of each DFG node
    // void resetBound();
    // calculate the routing latency of each edge, not inlcuding the delay pipe
    void calEdgeRouteLat();
    // calculate the DFG node latency bounds not considering the Delay components
    // including min latency of the output ports
    void latencyBound();
    // schedule the latency of each DFG node based on current mapping status
    void latencySchedule();
    // compare the load/store node latency, ensure delay_en latency demand needs
    bool calLSUenlatency();
    // set the LSU Node en delay
    void setLSUenlatency();
    // calculate the latency of DFG IO
    void calIOLat();
    // calculate the latency violation of each edge
    void calEdgeLatVio();
    // insert pass-through DFG nodes into a copy of current DFG
    void insertPassDfgNodes(DFG* newDfg);

    //@Yuan: temporal mapping
    // check the ADG nodes can be used in given time
    bool canMap(int time, ADGNode* candidate);
    // check the IOB ports can be used in given time
    bool checkIOB(int time, ADGNode* candidate);
    //set II
    void setII(int _II) {II = _II;};
    //get II
    int getII(){return II;}
    //set timeBoundary
    void setTimeBoundary(int _timeBoundary) {timeBoundary = _timeBoundary;};
    //set the ADG nodes is occupied
    void setOccupied(int time, ADGNode* adgNode, DFGNode* dfgNode);
    //cancel the ADG nodes is occupied
    void cancelOccupied(int time, ADGNode* adgNode, DFGNode* dfgNode);
    //set edge latency violation
    void setVio(int edgeId, int Vio);
    //set edge deleay, which is uesd for set delay unit
    void setDly(int edgeId, int delay);
    //record the latency violation edge
    void VioEdge(int edgeId, int vio);
    //delete the latency violation edge
    void delVioEdge(int edgeId);
    //get number of latency violation edge
    int numVioEdge(){return _vioDfgEdges.size();};
    //set total violation
    void setTotalVio(int edgeId);
    //substract edge violation in total violation
    void subTotalVio(int edgeId);
    //set mapping time of one node (sort by ALAP)
    void setALAPMapTime(int ALAP, int nodeid, int time);
    //Get the average mapping time of the node corresponding to an ALAP value
    int getALAPMapTime(int ALAP);
    //go back to adjust the delay unit of the GPE/LSU, to avoid timing violation
    int tryFixVio(DFGNode* dfgNode, ADGNode* candidate, DFGEdge* edge, int latestTime);
    //return _numNodeMapped
    int numNodeMapped(){ return _numNodeMapped;};
    //return ADGNode usage in time
    int numUsedTime(int ADGnodeId);
    //set the usage of delay unit of dfgNode
    void setDelayuse(DFGNode * dfgNode, int usage);
    //set the usage of delay unit of dfgNode
    int getDelayuse(DFGNode * dfgNode);
    //get DFGNode latest mapping time
    int getLatest(int DFGNodeId);
    // @Yuan: route self-cycle DFG edge
    bool routeSelfDfgEdge(DFGNode* dfgNode, DFGEdge* edge, ADGNode* adgNode, int EndTime);
    // @Yuan: update the edge endtime (just for cycle, which edge's endtime needs to subtract II)
    void updateEndTime(DFGEdge* edge, int newEndTime);
    //just for test, print the available Nodes
    void printAva();

    // HETA: clear all the historical usage of the ADGNodes
    void clearADGNodeUsage();
    int getADGNodeHisUsage(int ADGNodeId, int DFGNodeId);
};







#endif
