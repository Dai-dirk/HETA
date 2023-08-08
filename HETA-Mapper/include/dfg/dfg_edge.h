#ifndef __DFG_EDGE_H__
#define __DFG_EDGE_H__

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <assert.h>


class DFGEdge
{
private:
    int _id;
    int _srcPortIdx; // source node I/O port index
    int _dstPortIdx; // destination node I/O port index
    int _srcId;   // source node ID
    int _dstId;   // destination node ID
    bool _isCycle = false;  // @Yuan: if the edge is belongs to cycle
public:
    DFGEdge(){}
    DFGEdge(int edgeId){ _id = edgeId; }
    DFGEdge(int srcId, int dstId) : _srcId(srcId), _dstId(dstId) {}
    ~DFGEdge(){}
    int id(){ return _id; }
    void setId(int id){ _id = id; }
    int srcPortIdx(){ return _srcPortIdx; }
    void setSrcPortIdx(int srcPortIdx){ _srcPortIdx = srcPortIdx; }
    int dstPortIdx(){ return _dstPortIdx; }
    void setDstPortIdx(int dstPortIdx){ _dstPortIdx = dstPortIdx; }
    int srcId(){ return _srcId; }
    void setSrcId(int srcId){ _srcId = srcId; }
    int dstId(){ return _dstId; }
    void setDstId(int dstId){ _dstId = dstId; }
    void setEdge(int srcId, int dstId){
        _srcId = srcId;
        _dstId = dstId;
    }
    void setEdge(int srcId, int srcPort, int dstId, int dstPort){
        _srcId = srcId;
        _srcPortIdx = srcPort;
        _dstId = dstId;
        _dstPortIdx = dstPort;
    }
    //@Yuan: edge belongs to cycle flag
    void setIsCycle(bool isCycle){ _isCycle = isCycle; }
    bool isCycle(){ return _isCycle; };

};





#endif