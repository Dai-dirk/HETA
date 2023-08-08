#ifndef __CONFIGURATION_H__
#define __CONFIGURATION_H__

#include "mapper/mapping.h"



// configuration data
struct CfgData{
    int len; // config data length
    std::vector<uint32_t> data; // config data
    CfgData(){}
    CfgData(int len_) : len(len_){}
    CfgData(int len_, uint32_t data_){
        len = len_;
        data.push_back(data_);
    }
    CfgData& operator=(const CfgData& that){
        if(this == &that) return *this;
        len = that.len;
        data = that.data;
        return *this;
    }
};


// configuration data packet
struct CfgDataPacket{
    unsigned addr; // config address
    std::vector<uint32_t> data; // config data
    CfgDataPacket(unsigned addr_) : addr(addr_){}
    CfgDataPacket(unsigned addr_, uint32_t data_){
        addr = addr_;
        data.push_back(data_);
    }
};


// CGRA Configuration
class Configuration
{
private:
    Mapping* _mapping;
public:
    Configuration(Mapping* mapping) : _mapping(mapping) {}
    ~Configuration(){}
    // get config data for LSU, return<LSB-location, CfgData> new
    std::map<int, CfgData> getLsuCfgData(LSUNode* node, int time, int II);
    // get config data for GPE, return<LSB-location, CfgData>
    std::map<int, CfgData> getGpeCfgData(GPENode* node, int time, int II);
    // get config data for GIB, return<LSB-location, CfgData>
    std::map<int, CfgData> getGibCfgData(ADGNode* node, int time, int II);
    // get config data for IOB, return<LSB-location, CfgData>
    std::map<int, CfgData> getIobCfgData(ADGNode* node, int time, int II);
    // @Yuan； generating according to cycles with II
    // get config data for ADG node
    void getNodeCfgData(ADGNode* node, std::vector<CfgDataPacket>& cfg, int time, int II);
    // @Yuan； generating according to cycles with II
    // get config data for ADG
    void getCfgData(std::vector<CfgDataPacket>& cfg, int time, int II);
    // dump config data
    void dumpCfgData(std::ostream& os);
};






#endif