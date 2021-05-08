#include <cstring>
#include <SWCP.hpp>
#include <iostream>

using namespace std;
int main()
{
	bool pass = true;

	int vcount[] = {5, 32434, 3130, 2285, 10024};

    SWCP::Parser parser;
    SWCP::Graph graph;
    std::stringstream path;
    path << "./test.swc";
    bool result = parser.ReadSWCFromFile(path.str().c_str(), graph);



	SWCP::Generator generator;
	result = parser.ReadSWCFromFile("./test.swc", graph);

	std::ifstream file("./test.swc", std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	char* content = new char[static_cast<size_t>(size) + 1];
	file.read(content, size);

	content[size] = '\0';


	result = parser.ReadSWC(content, graph);
	pass &= result;

	std::string generated;
	result = generator.Write(generated, graph);

	std::cout << generated << std::endl;

	return 0;
}