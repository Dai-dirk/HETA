#ifndef __ADG_NODE_H__
#define __ADG_NODE_H__

#include <iostream>
#include <map>
#include <vector>
#include <set>
#include <assert.h>

#define ADDSUB 1
#define MUL 2
#define CMP 3
#define LOGIC 4
#define SHIF 5

// configuration data location
struct CfgDataLoc{
    int low;  // lowest bit index 
    int high; // highest bit index
};


class ADG;
class DFGNode;
class DFGEdge;
class ADGNode
{
private:
    int _id;
    std::string _type;
    int _bitWidth;
    int _numInputs;
    int _numOutputs;
    int _cfgBlkIdx;
    int _x;
    int _y;
    //new configmem constraint
    int _maxConfigMem;
    int _currentConfigMem;
    //@Yuan: extra delay unit usage, which is used to avoid the violation of sub-path
    int etrDelay = 0;
    //ADGNode is uesed flag
    std::map<int, bool> _isOccuppied;      // <mapped time, flag>
    std::map<int, std::vector<DFGNode*> > _mapped_dfg;  //  <mapped time, DFGNode>
    std::map<int, DFGEdge* > _mapped_edge;  //  <mapped time, DFGEdge>
    std::map<int, std::set<std::pair<int, int>>> _mapperInOut; // <time, set<input-index, output-index>>
    // int _cfgBitLen;
    std::vector<uint64_t> _cfgBits;
    std::map<int, std::pair<int, int>> _inputs; // <input-index, <node-id, node-port-idx>>
    std::map<int, std::set<std::pair<int, int>>> _outputs; // <output-index, set<node-id, node-port-idx>>
    // inputs connected to each output, <output-index, set<input-index>>
    std::map<int, std::set<int>> _out2ins;
    // outputs connected to each input, <input-index, set<output-index>>
    std::map<int, std::set<int>> _in2outs; 
    // configuration info, <ID, CfgDataFormat>
    std::map<int, CfgDataLoc> _configInfo;
    // sub-ADG consisting of sub-modules
    ADG* _subADG; 
    // @yuan: test for historic usage <node id, usagge>
    std::map<int, int> _historyUsage; 
protected:
    void printData();
public:
    ADGNode(){}
    ADGNode(int id){ _id = id; }
    ~ADGNode(){} // cannot delete _subADG here because of pre-declare class; 
    int id(){ return _id; }
    std::string type(){ return _type; }
    int bitWidth(){ return _bitWidth; }
    int numInputs(){ return _numInputs; }
    int numOutputs(){ return _numOutputs; }
    int cfgBlkIdx(){ return _cfgBlkIdx; }
    int x(){ return _x; }
    int y(){ return _y; }
    // int cfgBitLen(){ return _cfgBitLen; }
    void setId(int id){ _id = id; }
    void setType(std::string type){ _type = type; }
    void setBitWidth(int bitWidth){ _bitWidth = bitWidth; }
    void setNumInputs(int numInputs){ _numInputs = numInputs; }
    void setNumOutputs(int numOutputs){ _numOutputs = numOutputs; }
    void setCfgBlkIdx(int cfgBlkIdx){ _cfgBlkIdx = cfgBlkIdx; }
    void setX(int x){ _x = x; }
    void setY(int y){ _y = y; }
    // void setCfgBitLen(int cfgBitLen){ _cfgBitLen = cfgBitLen; }
    ADG* subADG(){ return _subADG; }
    void setSubADG(ADG* subADG){ _subADG = subADG; }
    const std::map<int, std::pair<int, int>>& inputs(){ return _inputs; }
    const std::map<int, std::set<std::pair<int, int>>>& outputs(){ return _outputs; }
    std::pair<int, int> input(int index); // return <node-id, node-port-idx>
    std::set<std::pair<int, int>> output(int index); // return set<node-id, node-port-idx>
    void addInput(int index, std::pair<int, int> node); // add input, _input_used
    void addOutput(int index, std::pair<int, int> node); // add output, _output_used
    // inputs connected to each output
    std::set<int> out2ins(int outPort);
    // outputs connected to each input
    std::set<int> in2outs(int inPort);
    // add input connected to the output
    void addOut2ins(int outPort, int inPort);
    // add output connected to the input
    void addIn2outs(int inPort, int outPort);
    // check if the input and output port are connected
    bool isInOutConnected(int inPort, int outPort); 
    // configuration info, <ID, CfgDataFormat>
    const std::map<int, CfgDataLoc>& configInfo(){ return _configInfo;}
    // get config info for sub-module
    const CfgDataLoc& configInfo(int id);
    // add config info for sub-module
    void addConfigInfo(int id, CfgDataLoc subModuleCfg);
    // virtual void verify();
    // // analyze the connections among the internal sub-modules 
    // virtual void analyzeIntraConnect(){}
    virtual void print();
    //new
    // set/get maximum configmem
    //void setMaxConfigMem(int maxConfigMem){_maxConfigMem = maxConfigMem;};
    //int MaxConfigMem(){ return _maxConfigMem;};
    // set/get current configmem
    void setCurConfigMem(int curConfigMem){_currentConfigMem = curConfigMem;};
    int CurconfigMem(){ return _currentConfigMem;};
    //whether the ADGNode is used in given time
    void setOccupied(int time, bool used);
    bool isOccupied(int time);
    //set/get the mapped DFGNode in given time
    void setMappedDFG(int time, DFGNode * DFGNode);
    std::vector<DFGNode*> getMappedDFG(int time);
    //delete the mapped DFGNode
    void delMappedDFG(int time, DFGNode * DFGNode);
    //set/get <inport, outport> in given time
    void addMappedInOut(int time, std::pair<int, int> node); // add (mapped time, <inPort, OutPort>)
    std::set<std::pair<int, int>> MappedInOut(int time); // return set(mapped time, <inPort, OutPort>)
    //@Yuan: reset all the PnR status
    void reset();
    //[HETA]: return available connections in a GIB
    int avaConnect();

    //[HETA]: historical usage
    void addUsage(int dfgNodeId);
    int  getUsage(int dfgNodeId);
    void clearUsage();

};


class GPENode : public ADGNode
{
private:
    int _numRfReg; // number of registers in RegFile 
    int _maxDelay; // max delay cycles of the internal DelayPipe 
    int _numOperands; // number of operands
    std::set<std::string> _operations; // supported operations
    std::vector<std::set<int>> _operandInputs; // indexes of input ports connected to each operand
    std::map<int, int> _CurOpUsage; // the usage of the opset for current test case <opset-id, usage>
    std::map<int, int> _OpUsage; // the usage of the opset for all test cases <opset-id, usage>
    int _maxdelayUsage = 0; // the usage of the delayunit for all test cases
    int _curmaxdelayUsage = 0; // the usage of the delayunit for curre test cases
public:
    using ADGNode::ADGNode; // C++11, inherit parent constructors
    int numRfReg(){ return _numRfReg; } 
    int maxDelay(){ return _maxDelay; }
    int numOperands(){ return _numOperands; }
    void setNumRfReg(int numRfReg){ _numRfReg = numRfReg; }
    void setMaxDelay(int maxDelay){ _maxDelay = maxDelay; }
    void setNumOperands(int numOperands); // set numOperands and resize _operandInputs 
    const std::set<std::string>& operations(){ return _operations; }
    void addOperation(std::string op); // add supported operation
    void delOperation(std::string op); // delete supported operation
    bool opCapable(std::string op); // check if the operation is supported
    const std::set<int>& operandInputs(int opeIdx); // get input ports connected to this operand
    void addOperandInputs(int opeIdx, int inputIdx); // add input port connected to this operand
    void delOperandInputs(int opeIdx, int inputIdx);
    void addOperandInputs(int opeIdx, std::set<int> inputIdxs); // add input ports connected to this operand
    int getOperandIdx(int inputIdx); // get which operand this input is connected
    virtual void print();
    void setCurOpsetUsage(std::string op); // set the opset used of GPE
    void updateOpUsage();
    void initOpUsage();
    const std::map<int, int>& opUsage(){ return _OpUsage; }
    void setCurMaxDlyUsage(int maxDly); //set current test case max delay-unit usage
    void updateMaxDly(); // add-up the max delay-unit usage for all test cases
    int getMaxDly(){ return _maxdelayUsage; }
};


class GIBNode : public ADGNode
{
private:
    bool _trackReged; // if there are registers in the track output ports
    std::map<int, bool> _outReged; // if there are registers in the output port 
    double _utilization = 0; // the utilization of the GIB (used connect / available connect)
    int _usage; // the usage of the GIB connections 
public:
    using ADGNode::ADGNode; // C++11, inherit parent constructors
    bool trackReged(){ return _trackReged; }
    void setTrackReged(bool trackReged){ _trackReged = trackReged; }
    bool outReged(int idx);
    void setOutReged(int idx, bool reged);
    virtual void print();
    void addUti(int connectionUsg);
    // void setUti(int connectionUsg);
    void updateUti(int II);
    double getUti() {return _utilization;}
};


class IOBNode : public ADGNode
{
private:
    std::set<std::string> _operations; // supported operations
public:
    using ADGNode::ADGNode; // C++11, inherit parent constructors
    void addOperation(std::string op); // add supported operation
    void delOperation(std::string op); // delete supported operation
    bool opCapable(std::string op); // check if the operation is supported
    const std::set<std::string>& operations(){ return _operations; }

};

class LSUNode : public ADGNode//  new
{
private:
   int _maxEnDelay; // max delay cycles of the internal Delay_en 
   int _numOperands;
   int _maxDelay; // max delay cycles of the internal DelayPipe 
   int _endelay;
   std::vector<std::set<int>> _operandInputs; // indexes of input ports connected to each SRAM operand
   std::set<std::string> _operations; // supported operations
   int _maxdelayUsage = 0; // the usage of the delayunit for all test cases
   int _curmaxdelayUsage = 0; // the usage of the delayunit for curre test cases
public:
    using ADGNode::ADGNode; // C++11, inherit parent constructors
    int maxEnDelay(){ return _maxEnDelay; }
    int maxDelay(){ return _maxDelay; }
    void setMaxEnDelay(int maxDelay){ _maxEnDelay = maxDelay; }
    void setEnDelay(int enDelay){ _endelay = enDelay; }
    int EnDelay(){ return _endelay; }
    void setMaxDelay(int maxDelay){ _maxDelay = maxDelay; }
    int numOperands(){ return _numOperands; }
    void setNumOperands(int numOperands); // set numOperands and resize _operandInputs 
    void addOperandInputs(int opeIdx, int inputIdx); // add input port connected to this operand
    const std::set<int>& operandInputs(int sramIdx); // get input ports connected to this operand
    void addOperation(std::string op); // add supported operation
    void delOperation(std::string op); // delete supported operation
    bool opCapable(std::string op); // check if the operation is supported
    const std::set<std::string>& operations(){ return _operations; }
    int getOperandIdx(int inputIdx); // get which operand this input is connected
    virtual void print();
    void setCurMaxDlyUsage(int maxDly); //set current test case max delay-unit usage
    void updateMaxDly(); // add-up the max delay-unit usage for all test cases
    int getMaxDly(){ return _maxdelayUsage; }
};




#endif