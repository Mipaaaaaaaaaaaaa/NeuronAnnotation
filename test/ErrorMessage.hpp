#ifndef NEURONANNOTATION_ERRORMESSAGE_HPP
#define NEURONANNOTATION_ERRORMESSAGE_HPP

#include<iostream>
#include <Poco/Dynamic/Var.h>
#include <Poco/JSON/Object.h>
using namespace std;
using Poco::Dynamic::Var;
class ErrorMessage{
    public:
    string type;
    string message;
    ErrorMessage(string m,string t="error"){
        type = t;
        message = m;
    }
    string ToJson(){
        Poco::JSON::Object v;
        v.set("type",type);
        v.set("message",message);
        Poco::Dynamic::Var jsnString = JSON(v);
        return jsnString.toString();
    }
}