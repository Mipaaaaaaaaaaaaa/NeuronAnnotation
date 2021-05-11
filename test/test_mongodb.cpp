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
        DataBase::insertSWC(graph.list_swc[i],"test1");
		s.push_back(make_shared<NeuronSWC>(graph.list_swc[i]));
    }
	DataBase::insertSWCs(s,"test2");
	
	return 0;
}