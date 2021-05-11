#include <cstring>
#include <SWCP.hpp>
#include <iostream>
#include "DataBase.hpp"
#include <AnnotationDS.hpp>

using namespace std;
int main()
{
	bool pass = true;
    SWCP::Parser parser;
    NeuronGraph graph;
    std::stringstream path;
    path << "./test.swc";
    bool result = parser.ReadSWCFromFile(path.str().c_str(), graph);
    DataBase::connect();
	std::vector<std::shared_ptr<NeuronSWC> > s;
    for( int i = 0 ; i < graph.list_swc.size() ; i ++ ){
		s.push_back(make_shared<NeuronSWC>(graph.list_swc[i]));
    }
    // DataBase::insertSWCs(s,"test2");
    std::cout << DataBase::getSWCFileStringFromTable("test2") << std::endl;
	DataBase::modifySWCs(s,"test2");
    DataBase::modifySWC(graph.list_swc[2],"test2");
    std::cout << DataBase::findTable("test2") << std::endl;
    std::cout << DataBase::findTable("test1") << std::endl;
	std::cout << DataBase::getSWCFileStringFromTable("test2") << std::endl;


	return 0;
}