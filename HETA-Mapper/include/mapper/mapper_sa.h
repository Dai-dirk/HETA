#ifndef __MAPPER_SA_H__
#define __MAPPER_SA_H__

#include "mapper/mapper.h"
#include <cmath>


// mapper using simulated annealing algorithm
class MapperSA : public Mapper
{
private:
    // max iteration number of mapping 
    int _maxIters;
    // if optimize mapping objective
    bool _objOpt;
    const float MAX_TEMP = 100; // max temperature
public:
    MapperSA(ADG* adg, int timeout_ms = 600000, int maxIter = 10000, bool objOpt = true);
    // MapperSA(ADG* adg, DFG* dfg);
    MapperSA(ADG* adg, DFG* dfg, int timeout_ms = 600000, int maxIter = 10000, bool objOpt = true);
    ~MapperSA();
    float randfloat(){ return rand() / (float)(RAND_MAX);}
    void setMaxIters(int num){ _maxIters = num; }
    void setObjOpt(bool objOpt){ _objOpt = objOpt; }
    // PnR and Data Synchronization
    bool pnrSync(int maxIters, float temp, bool isOpt = false, bool spatial = false, bool isHeteMapping = false);
    // PnR, Data Synchronization, and objective optimization
    bool pnrSyncOpt();
    // map the DFG to the ADG, mapper API
    virtual bool mapper(bool spatial, bool isHeteMapping);
    
protected:
    // PnR with SA temperature(max = 100)
    int pnr(Mapping* mapping, float temp);
    // unmap some DFG nodes
    void unmapSome(Mapping* mapping, float temp);
    // incremental PnR, try to map all the left DFG nodes based on current mapping status
    int incrPnR(Mapping* mapping);
    // try to map one DFG node to one of its candidates
    // return selected candidate index
    int tryCandidates(Mapping* mapping, DFGNode* dfgNode, const std::vector<ADGNode*>& candidates);
    // try to map one DFG node to one candidates
    bool tryCandidate(Mapping* mapping, DFGNode* dfgNode, ADGNode* candidate);
    // find candidates for one DFG node based on current mapping status
    std::vector<ADGNode*> findCandidates(Mapping* mapping, DFGNode* dfgNode, int maxCandidates);
    // get the shortest distance between ADG node and the available ADG input
    int getAdgNode2InputDist(Mapping* mapping, int id);
    // get the shortest distance between ADG node and the available ADG output
    int getAdgNode2OutputDist(Mapping* mapping, int id);
    // sort candidates according to their distances with the mapped src and dst ADG nodes of this DFG node 
    // return sorted index of candidates
    std::vector<int> sortCandidates(Mapping* mapping, DFGNode* dfgNode, const std::vector<ADGNode*>& candidates);
    
    // objective funtion
    int objFunc(Mapping* mapping);
    // SA: the probablity of accepting new solution
    bool metropolis(double diff, double temp);
    // annealing funtion
    float annealFunc(float temp);
    // get the shortest distance between ADG node and the available IOB
    int getAdgNode2IODist(int id);
    // get the shortest distance between ADG node and the available LSU
    int getAdgNode2LSUDist(int id);
};




#endif