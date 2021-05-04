#include <cstring>
#include "SWCP.hpp"
#include <iostream>

int main()
{
	bool pass = true;

	int vcount[] = {5, 32434, 3130, 2285, 10024};

    SWCP::Parser parser;
    SWCP::Graph graph;
    std::stringstream path;
    path << "./test.swc";
    bool result = parser.ReadSWCFromFile(path.str().c_str(), graph);
    pass &= result;
    pass &= vcount[1] == static_cast<int>(graph.vertices.size());

    if (!result)
    {
        printf("Failed on %d\n", 1);
    }
    else
    {
        printf("Passed: %s\n", path.str().c_str());
    }

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
	pass &= result;
	std::cout << generated << std::endl;
	result = strcmp(generated.c_str(), content) == 0;
	pass &= result;
	
	if (!result)
	{
		printf("Failed on comparing read and generated file\n");
	}
	else
	{
		printf("Passed on comparing read and generated file\n");
	}
		
	if (!pass)
	{
		return -1;
	}
	printf("Passed all tests\n");
	return 0;
}