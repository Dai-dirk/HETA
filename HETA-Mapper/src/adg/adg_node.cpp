
#include "adg/adg_node.h"
#include<algorithm>

// ===================================================
//   ADGNode functions
// ===================================================

// return <node-id, node-port-idx>
std::pair<int, int> ADGNode::input(int index){
    if(_inputs.count(index)){
        return _inputs[index];
    }else{
        return {}; // std::make_pair(-1, -1);
    }
}

// return set<node-id, node-port-idx>
std::set<std::pair<int, int>> ADGNode::output(int index){
    if(_outputs.count(index)){
        return _outputs[index];
    }else{
        return {}; // return empty set
    }
}

// add input, _input_used
void ADGNode::addInput(int index, std::pair<int, int> node){
    _inputs[index] = node;
    // _input_used[index] = false; // initialize not used 
}

// add output, _output_used
void ADGNode::addOutput(int index, std::pair<int, int> node){
    _outputs[index].emplace(node);
    // _output_used[index] = false; // initialize not used 
}


// inputs connected to each output
std::set<int> ADGNode::out2ins(int outPort){
    if(_out2ins.count(outPort)){
        return _out2ins[outPort];
    } else {
        return {};
    }
}


// outputs connected to each input
std::set<int> ADGNode::in2outs(int inPort){
    if(_in2outs.count(inPort)){
        return _in2outs[inPort];
    } else {
        return {};
    }
}

// add input connected to the output
void ADGNode::addOut2ins(int outPort, int inPort){
    if(_inputs.count(inPort) && _outputs.count(outPort)){
        _out2ins[outPort].emplace(inPort);
    }
}

// add output connected to the input
void ADGNode::addIn2outs(int inPort, int outPort){
    if(_inputs.count(inPort) && _outputs.count(outPort)){
        _in2outs[inPort].emplace(outPort);
    }
}


// check if the input and output port are connected
bool ADGNode::isInOutConnected(int inPort, int outPort){
    if(!_out2ins.count(outPort)){
        return false;
    } else{
        for(auto in : _out2ins[outPort]){
            if(in == inPort){
                return true;
            }
        }
        return false;
    }
}


// get config info for sub-module
const CfgDataLoc& ADGNode::configInfo(int id){
    assert(_configInfo.count(id));
    return _configInfo[id];
}


// add config info for sub-module
void ADGNode::addConfigInfo(int id, CfgDataLoc subModuleCfg){
    _configInfo[id] = subModuleCfg;
}


void ADGNode::printData(){
    std::cout << "ADGNode(id): " << _id << std::endl;
    std::cout << "bitWidth: " << _bitWidth << std::endl;
    std::cout << "type: " << _type << std::endl;
    std::cout << "numInputs: " << _numInputs << std::endl;
    std::cout << "numOutputs: " << _numOutputs << std::endl;
    std::cout << "cfgBlkIdx: " << _cfgBlkIdx << std::endl;
    // std::cout << "cfgBitLen: " << _cfgBitLen << std::endl;
    std::cout << "inputs: " << std::endl;
    for(auto& elem : _inputs){
        std::cout << elem.first << ", " << elem.second.first << ", " << elem.second.second << std::endl;
    }
    std::cout << "outputs: " << std::endl;
    for(auto& elem : _outputs){
        int idx = elem.first;
        auto& s = elem.second;
        for(auto it = s.begin(); it != s.end(); it++)
            std::cout << idx << ", " << it->first << ", " << it->second << std::endl;
    }
    std::cout << "out2ins: " << std::endl;
    for(auto& elem : _out2ins){
        for(auto sec : elem.second)
            std::cout << elem.first << ", " << sec << std::endl;
    }
    std::cout << "in2outs: " << std::endl;
    for(auto& elem : _in2outs){
        for(auto sec : elem.second)
            std::cout << elem.first << ", " << sec << std::endl;
    }
}


void ADGNode::print(){
    printData();
}
//new
//whether the ADGNode is used in given time
void ADGNode::setOccupied(int time, bool used){
   _isOccuppied[time] = used;
}


bool ADGNode::isOccupied(int time){
    if(_isOccuppied.count(time)){
        return _isOccuppied[time];
    } else {
        return false;
    }
}

//new
//set/get the mapped DFGNode in given time
void ADGNode::setMappedDFG(int time, DFGNode * DFGNode){
    _mapped_dfg[time].push_back(DFGNode);
}

//new
//delete the mapped DFGNode in given time
void ADGNode::delMappedDFG(int time, DFGNode * dfgNode){
    _mapped_dfg[time].erase(std::remove(_mapped_dfg[time].begin(), _mapped_dfg[time].end(), dfgNode), _mapped_dfg[time].end());
}

std::vector<DFGNode*> ADGNode::getMappedDFG(int time){
    if(_mapped_dfg.count(time)){
        return _mapped_dfg[time];
    } else {
        return {};
    }
}

//new
//set/get <inport, outport> in given time
void ADGNode::addMappedInOut(int time, std::pair<int, int> node){// add (mapped time, <inPort, OutPort>)
    _mapperInOut[time].emplace(node);
} 


std::set<std::pair<int, int>> ADGNode::MappedInOut(int time){ // return set(mapped time, <inPort, OutPort>)
    if(_mapperInOut.count(time)){
        return _outputs[time];
    }else{
        return {}; // return empty set
    }
}


//@Yuan: reset all the PnR status
void ADGNode::reset(){
     _isOccuppied.clear();
     _mapped_dfg.clear();
     _currentConfigMem = 0;
}

//[HETA]: return available connections in a GIB
int ADGNode::avaConnect(){
    int num_connect = 0;
    for(auto& elem: _out2ins){
        num_connect += elem.second.size();
    }
    return num_connect;
}


//[HETA]: add historical usage 
void ADGNode::addUsage(int dfgNodeId){
    if(_historyUsage.count(dfgNodeId)){
        _historyUsage[dfgNodeId] += 1;
    }else{
        _historyUsage[dfgNodeId] = 1;
    }
}

//[HETA]: get the ADGNode historical usage of specific dfgNode
int ADGNode::getUsage(int dfgNodeId){
    if(_historyUsage.count(dfgNodeId)){
        return _historyUsage[dfgNodeId];
    }else{
        return 0;
    }
}


//[HETA]: get the ADGNode historical usage of specific dfgNode
void ADGNode::clearUsage(){
    _historyUsage.clear();
}

// ===================================================
//   GPENode functions
// ===================================================

// set numOperands and resize _operandInputs 
void GPENode::setNumOperands(int numOperands){ 
    _numOperands = numOperands; 
    _operandInputs.resize(numOperands);
}

// add supported operation
void GPENode::addOperation(std::string op){
    _operations.emplace(op);
}

// delete supported operation
void GPENode::delOperation(std::string op){
    _operations.erase(op);
}

// check if the operation is supported
bool GPENode::opCapable(std::string op){
    return _operations.count(op);
}

// get input ports connected to this operand
const std::set<int>& GPENode::operandInputs(int opeIdx){
    assert(_operandInputs.size() > opeIdx);
    return _operandInputs[opeIdx]; 
}

// add input port connected to this operand
void GPENode::addOperandInputs(int opeIdx, int inputIdx){
    assert(_operandInputs.size() > opeIdx);
    if(inputs().count(inputIdx)){
        _operandInputs[opeIdx].emplace(inputIdx);
    }
}


// delete input port connected to this operand
void GPENode::delOperandInputs(int opeIdx, int inputIdx){
    assert(_operandInputs.size() > opeIdx);
    if(inputs().count(inputIdx)){
        _operandInputs[opeIdx].erase(inputIdx);
    }
}


// add input ports connected to this operand
void GPENode::addOperandInputs(int opeIdx, std::set<int> inputIdxs){
    assert(_operandInputs.size() > opeIdx);
    _operandInputs[opeIdx] = inputIdxs;
}


// get which operand this input is connected
int GPENode::getOperandIdx(int inputIdx){
    for(int i = 0; i < _numOperands; i++){
        if(_operandInputs[i].count(inputIdx)){
            return i;
        }
    }
    return -1;
}


void GPENode::print(){
    printData();
    std::cout << "numOperands: " << _numOperands << std::endl;
    std::cout << "operandInputs: " << std::endl;
    int i = 0;
    for(auto& elem : _operandInputs){
        std::cout << i++ << ": ";
        for(auto in : elem){
            std::cout << in << " ";
        }
        std::cout << std::endl;
    }
}

void GPENode::initOpUsage(){
    _OpUsage[ADDSUB] = 0;
    _OpUsage[MUL] = 0;
    _OpUsage[CMP] = 0;
    _OpUsage[LOGIC] = 0;
    _OpUsage[SHIF] = 0;
}

void GPENode::setCurOpsetUsage(std::string op){
    int opset = 0;
    std::set<std::string> _ADDSUB = {"ADD", "SUB"};
    std::set<std::string> _MUL = {"MUL"};
    std::set<std::string> _CMP = {"EQ", "LT", "LE" };
    std::set<std::string> _LOGIC = {"AND", "OR", "XOR" };
    std::set<std::string> _SHIFT = {"SHL", "LSHR", "ASHR" };
    if(_ADDSUB.find(op) != _ADDSUB.end()){
        opset = ADDSUB;
    }else if(_MUL.find(op) != _MUL.end()){
        opset = MUL;
    }else if(_CMP.find(op) != _CMP.end()){
        opset = CMP;
    }else if(_LOGIC.find(op) != _LOGIC.end()){
        opset = LOGIC;
    }else if(_SHIFT.find(op) != _SHIFT.end()){
        opset = SHIF;
    }else if(op != "PASS"){
        std::cout << "[HETA]: Error, the operation: " << op << " is not supported!" << std::endl;
        exit(1);
    }
    _CurOpUsage[opset] = 1;
    // if(_opUsage.count(opset)){
    //     _opUsage[opset] += 1;
    // }else{
    //     _opUsage[opset] = 1;
    // }
}
// update the opset usage for all test cases
void GPENode::updateOpUsage(){
    if(_CurOpUsage.count(ADDSUB)){
        _OpUsage[ADDSUB] += _CurOpUsage[ADDSUB];
        _CurOpUsage[ADDSUB] = 0;
    }
    if(_CurOpUsage.count(MUL)){
        _OpUsage[MUL] += _CurOpUsage[MUL];
        _CurOpUsage[MUL] = 0;
    }
    if(_CurOpUsage.count(CMP)){
        _OpUsage[CMP] += _CurOpUsage[CMP];
        _CurOpUsage[CMP] = 0;
    }
    if(_CurOpUsage.count(LOGIC)){
        _OpUsage[LOGIC] += _CurOpUsage[LOGIC];
        _CurOpUsage[LOGIC] = 0;
    }
    if(_CurOpUsage.count(SHIF)){
        _OpUsage[SHIF] += _CurOpUsage[SHIF];
        _CurOpUsage[SHIF] = 0;
    }
}

// set the maximum delay usage
void GPENode::setCurMaxDlyUsage(int maxDly){
    _curmaxdelayUsage = std::max(_curmaxdelayUsage, maxDly);
}
// update the maximu delay usage for all test cases
void GPENode::updateMaxDly(){
    _maxdelayUsage += _curmaxdelayUsage;
    // _maxdelayUsage = std::max(_maxdelayUsage, _curmaxdelayUsage);
    _curmaxdelayUsage = 0;
}
// ===================================================
//   GIBNode functions
// ===================================================

// if there are registers in the output port 
bool GIBNode::outReged(int idx){
    return _outReged[idx];
}

// if there are registers in the output port 
void GIBNode::setOutReged(int idx, bool reged){
    _outReged[idx] = reged;
}


void GIBNode::print(){
    printData();
    std::cout << "trackReged: " << _trackReged << std::endl;
    std::cout << "outReged: " << std::endl;
    for(auto& elem : _outReged){
        std::cout << elem.first << ": " << elem.second << std::endl;
    }
}

// add the usage of GIB
void GIBNode::addUti(int connectionUsg){
    _usage += connectionUsg;
}

// set the usage of GIB
// void GIBNode::setUti(int connectionUsg){
//     _usage = connectionUsg;
// }
// update the utilization of GIB
void GIBNode::updateUti(int II){
    double utilization = 0;
    utilization = ((double)_usage / (avaConnect())) * 100;
    // std::cout << "II: " << II << " USAGE: " << _usage <<" avaconnect: " <<avaConnect()<<" avaconnect*II: " << (avaConnect() * II) <<" _utilization: "<<_utilization<< std::endl;
    _utilization += utilization;
    _usage = 0;
}

//new
// ===================================================
//   LSUNode functions
// ===================================================

// set numSramInput and resize _sramInputs 
void LSUNode::setNumOperands(int numOperands){ 
    _numOperands = numOperands; 
    _operandInputs.resize(numOperands);
}

// add input port connected to this sram input
void LSUNode::addOperandInputs(int opeIdx, int inputIdx){
    assert(_operandInputs.size() > opeIdx);
    if(inputs().count(inputIdx)){
        _operandInputs[opeIdx].emplace(inputIdx);
    }
}

// get input ports connected to this Sram Input
const std::set<int>& LSUNode::operandInputs(int sramIdx){
    assert(_operandInputs.size() > sramIdx);
    return _operandInputs[sramIdx]; 
}
// add supported operation
void LSUNode::addOperation(std::string op){
    _operations.emplace(op);
}
// delete supported operation
void LSUNode::delOperation(std::string op){
    _operations.erase(op);
}
// check if the operation is supported
bool LSUNode::opCapable(std::string op){
    return _operations.count(op);
}
void LSUNode::print(){
    printData();
    std::cout << "MaxEnDelay: " << _maxEnDelay << std::endl;
    std::cout << "SRAMInputs: " << std::endl;
    int i = 0;
    for(auto& elem : _operandInputs){
        std::cout << i++ << ": ";
        for(auto in : elem){
            std::cout << in << " ";
        }
        std::cout << std::endl;
    }
}

// get which operand this input is connected
int LSUNode::getOperandIdx(int inputIdx){
    for(int i = 0; i < _numOperands; i++){
        if(_operandInputs[i].count(inputIdx)){
            return i;
        }
    }
    return -1;
}
// set the maximum delay usage
void LSUNode::setCurMaxDlyUsage(int maxDly){
    _curmaxdelayUsage = std::max(_curmaxdelayUsage, maxDly);
}
// update the maximu delay usage for all test cases
void LSUNode::updateMaxDly(){
    _maxdelayUsage += _curmaxdelayUsage;
    // _maxdelayUsage = std::max(_maxdelayUsage, _curmaxdelayUsage);
    _curmaxdelayUsage = 0;
}

//new
// ===================================================
//   LSUNode functions
// ===================================================
// add supported operation
void IOBNode::addOperation(std::string op){
    _operations.emplace(op);
}
// delete supported operation
void IOBNode::delOperation(std::string op){
    _operations.erase(op);
}
// check if the operation is supported
bool IOBNode::opCapable(std::string op){
    return _operations.count(op);
}