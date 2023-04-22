#pragma once
#include "DOM/DOMNodeBase.hpp"
#include <map>

class SBcsvIO;

class SDOMNodeSerializable : public SDOMNodeBase {
public:
    typedef SDOMNodeBase Super; 

    SDOMNodeSerializable(std::string name);

    virtual bool IsNodeType(EDOMNodeType type) const override {
        if(type == EDOMNodeType::Serializable){
            return true;
        }

        return Super::IsNodeType(type);
    }

    virtual void Deserialize(SBcsvIO* bcsv, int entry) = 0;
    virtual void Serialize(SBcsvIO* bcsv, int entry) = 0;

};