
#include "op/operations.h"


std::map<std::string, OpInfo> Operations::_OpInfoMap;

Operations::Operations(std::string& filename)
{
    std::ifstream ifs(filename);
    if(!ifs){
        std::cout << "cannot open OpInfo file: " << filename << std::endl;
        exit(1);
    } 
    json ops_info;
    ifs >> ops_info;
    for(auto& elem : ops_info["Operations"]){
        std::string name = elem["name"].get<std::string>();
        OpInfo op_info;
        op_info.OPC = elem["OPC"].get<int>();
        if(elem.contains("numOperands")){
            op_info.numOperands = elem["numOperands"].get<int>();
        } else{
            op_info.numOperands = -1;
        }
        if(elem.contains("numRes")){
            op_info.numRes = elem["numRes"].get<int>();
        } else{
            op_info.numRes = -1;
        }
        if(elem.contains("latency")){
            op_info.latency = elem["latency"].get<int>();
        } else{
            op_info.latency = -1;
        }
        if(elem.contains("commutative")){
            op_info.commutative = elem["commutative"].get<int>() == 1;
        } else{
            op_info.commutative = false;
        }
        if(elem.contains("bitWidth")){
            op_info.bitWidth = elem["bitWidth"].get<int>();
        } else{
            op_info.bitWidth = -1;
        }
        if(elem.contains("area")){
            op_info.area = elem["area"].get<float>();
        } else{
            op_info.area = 0.0;
        }
        if(elem.contains("power")){
            op_info.power = elem["power"].get<float>();
        } else{
            op_info.power = 0.0;
        }
        _OpInfoMap[name] = op_info;
    }
}

Operations::~Operations()
{
    _OpInfoMap.clear();
}


void Operations::print(){
    int i = 0;
    for(auto& elem : _OpInfoMap){
        std::cout << i++ << ". operation name: " << elem.first << std::endl;
        auto& info = elem.second;
        std::cout << "\tOPC: " << info.OPC << std::endl;
        std::cout << "\tnumOperands: " << info.numOperands << std::endl;
        std::cout << "\tnumRes: " << info.numRes << std::endl;
        std::cout << "\tlatency: " << info.latency << std::endl;
        std::cout << "\tisCommutative: " << info.commutative << std::endl;
        std::cout << "\tbitWidth: " << info.bitWidth << std::endl;
        std::cout << "\tarea: " << info.area << std::endl;
        std::cout << "\tpower: " << info.power << std::endl;
    }
}

int Operations::numOps(){
    return _OpInfoMap.size();
}


int Operations::OPC(const std::string& op){
    if(_OpInfoMap.count(op)){
        auto op_info = _OpInfoMap[op];
        return op_info.OPC;
    }else{
        return -1;
    }
}


int Operations::numOperands(const std::string& op){
    if(_OpInfoMap.count(op)){
        auto op_info = _OpInfoMap[op];
        return op_info.numOperands;
    }else{
        return -1;
    }
}


int Operations::numRes(const std::string& op){
    if(_OpInfoMap.count(op)){
        auto op_info = _OpInfoMap[op];
        return op_info.numRes;
    }else{
        return -1;
    }
}

//new, set delay of load or store operation equals 1
int Operations::latency(const std::string& op){
    if(_OpInfoMap.count(op)){
        auto op_info = _OpInfoMap[op];
        return op_info.latency;
    }else if(op == "LOAD" || op == "STORE"){
        return 1;  
    }else{
        return -1;
    }
}

int Operations::bitWidth(const std::string& op){
    if(_OpInfoMap.count(op)){
        auto op_info = _OpInfoMap[op];
        return op_info.bitWidth;
    }else{
        return -1;
    }
}


float Operations::area(const std::string& op){
    if(_OpInfoMap.count(op)){
        auto op_info = _OpInfoMap[op];
        return op_info.area;
    }else{
        return 0.0;
    }
}


float Operations::power(const std::string& op){
    if(_OpInfoMap.count(op)){
        auto op_info = _OpInfoMap[op];
        return op_info.power;
    }else{
        return 0.0;
    }
}

//new add support for load/store operations
bool Operations::isCommutative(const std::string& op){
    if(_OpInfoMap.count(op)){
        auto op_info = _OpInfoMap[op];
        return op_info.commutative;
    //}else if(op == "LOAD" || op == "STORE"){
    }else if(op == "STORE"){
        return true;  
    }else{
        return false;
    }
}


// if the operation is supported
bool Operations::opCapable(const std::string& op){
    return _OpInfoMap.count(op);
}