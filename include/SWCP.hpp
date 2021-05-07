#ifndef SWCP_HPP
#define SWCP_HPP

#include <stdlib.h>
#include <cstdarg>
#include <vector>
#include <fstream>
#include <sstream>
#include <map>
#include <list>

using namespace std;

namespace SWCP 
{
	enum Type
	{
		Undefined = 0,
		Soma, //胞体 1
		Axon, //轴突 2
		Dendrite, //树突 3
		ApicalDendrite, //顶树突 4
		ForkPoint, //分叉点 5
		EndPoint, //端点 6
		Custom,
	};

	#if __cplusplus >= 201103L
		#define SWCP_CPP11_COMPATIBLE
	#endif
	#if defined _MSC_VER && _MSC_VER >= 1800
		#define SWCP_CPP11_COMPATIBLE
	#endif

	// If not c++11 and if it is not version of MSVC that is c++11 compatible, 
	// then define own int64_t and uint64_t types in SWCP namespace.
	#if !defined SWCP_CPP11_COMPATIBLE
		typedef long long int int64_t;
		typedef unsigned long long int uint64_t;
	#endif

	// strtoll and strtoull are not part of standard prior c++11, 
	// but are widely supported, except for MSVC that has own names for them
	#if defined _MSC_VER && (_MSC_VER < 1800)
		inline int64_t strtoll(char const* str, char** endPtr, int radix)
		{
			return _strtoi64(str, endPtr, radix);
		}

		inline uint64_t strtoull(char const* str, char** endPtr, int radix)
		{
			return _strtoui64(str, endPtr, radix);
		}
	#endif

	// Safe version of sprintf
	#if !defined SWCP_CPP11_COMPATIBLE && defined _MSC_VER
		template<class T, size_t N>
		inline int64_t sprintf(T(&dstBuf)[N], const char * format, ...)
		{
			va_list args;
			va_start(args, format);
			int result = vsprintf_s(dstBuf, format, args);
			va_end(args);
			return result;
		}
	#elif defined SWCP_CPP11_COMPATIBLE
		template<class T, size_t N>
		inline int sprintf(T(&dstBuf)[N], const char * format, ...)
		{
			va_list args;
			va_start(args, format);
			int result = vsnprintf(dstBuf, N, format, args);
			va_end(args);
			return result;
		}
	#endif

	int GetCurrentTimestamp(){
		time_t timer;
		return timer;
	}

	typedef struct BasicObj
	{
		int64_t id; //index
		std::string color; //color
		bool visible; //是否可见
		bool selected; //是否被选择
		std::string name;
		BasicObj(){
			id=0;
			color="#000000";
			selected=false;
			visible=true;
			name="";
		}
	}BasicObj;

	typedef struct NeuronSWC : public BasicObj
	{
		//继承id
		Type type;
		float x,y,z;
		union{
			float r;
			float radius;
		};
		union
		{
			int64_t pn;
			int64_t parent;
		};
		int64_t line_id; //属于的路径id
		int64_t seg_size; //线段的swc大小
		int64_t seg_id; //线段的id
		int64_t seg_in_id; //该点在线段内的id
		int64_t block_id; //点所在脑数据中的block
		int64_t timestamp; //时间戳
		NeuronSWC(){
			id=0;
			type=Undefined;
			x=y=z=0;
			r=1;
			pn=-1;
			line_id=-1;
			seg_id=1;
			seg_size=-1;
			seg_in_id=-1;
			block_id=1;
			timestamp=-1;
		}
	} NeuronSWC;

	typedef struct Vertex : public BasicObj //记录关键节点和它们之下的节点的SWC索引
	{

		Vertex(int64_t id, Type type, double x, double y, double z, float radius) : id(id), type(type), radius(radius), x(x), y(y), z(z)
		{};
		
		Vertex() {};

		double x,y,z;
		float radius;
		Type type;
		int64_t line_id;
		map<int, bool> hash_linked_seg_ids; //相关的线id
		list<int> linked_vertex_ids; //相连的点id
	} Vertex;

	typedef struct Segment //路径中的单个线段
	{
		int size;
		int line_id;
		list<int> segment_vertex_ids; //在线中的顶点在SWC中的索引id
		Segment(int s, int l){
			size = s;
			line_id = l;
		};

		void InsertId(int _id){
			segment_vertex_ids.push_back(_id);
		}

		bool VextexIdxIsIn(int _id){ //检测SWC中的Index点是否在该线段上
			for( auto i : segment_vertex_ids ){
				if(i == _id){
					return true;
				}
			}
			return false;
		}
	} Segment;
		
	struct Line : public BasicObj //Line是有关关键Vertex的集合
	{
		map< int, Vertex > hash_vertexes;
		Line(){
			id=0;
			color="#000000";
			selected=false;
			visible=true;
			name="";
		}
	};
	
	struct Graph
	{
		string file; //文件源
		list<NeuronSWC> list_swc; //list_swc中间删除时，需要对hash_swc_id重新计算
		map<int,int> hash_swc_ids; //方便查询，点与相关联SWC文件的映射索引
		map<int,list<Line> > lines; //路径合集（点合辑）
		map<int,list<Segment> > segments; //关键点及非关键点的线段合集
		list<string> meta; //memo
	};

	class Parser
	{
	public:
		// Reads SWC from filespecified by *filename*. 
		// Output is written to *graph*, old content is errased. 
		// If no error have happened returns true
		bool ReadSWCFromFile(const char *filename, Graph& graph);

		// Reads SWC from filestream. 
		// Output is written to graph, old content is errased. 
		// If no error have happened returns true
		bool ReadSWC(std::istream &inStream, Graph& graph);

		// Reads SWC from string. 
		// Output is written to graph, old content is errased. 
		// If no error have happened returns true
		bool ReadSWC(const char *string, Graph& graph);
		
		// Returns error message for the last parsing if error have happened.
		std::string GetErrorMessage();
	private:
		void NextSymbol();

		bool Accept(char symbol);
		bool AcceptWhightSpace();
		bool AcceptLine(Graph& graph);
		bool AcceptEndOfLine();
		bool AcceptInteger(int64_t& integer);
		bool AcceptInteger(uint64_t& integer);
		bool AcceptDouble(double& integer);
		bool AcceptStringWithoutSpace(char &);
		
		const char* m_iterator;
		int m_line;
		std::stringstream m_errorMessage;
	};

	class Generator
	{
	public:
		bool WriteToFile(const char *filename, const Graph& graph);

		bool Write(std::ostream &outStream, const Graph& graph);

		bool Write(std::string& outString, const Graph& graph);

		std::string GetErrorMessage();
	private:
		enum 
		{
			MaxLineSize = 4096
		};
		std::stringstream m_errorMessage;
	};

	inline bool SWCP::Parser::ReadSWCFromFile(const char *filename, Graph& graph)
	{
		std::ifstream file(filename, std::ios::binary | std::ios::ate);
		std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		if (!file.is_open())
		{
			m_errorMessage.clear();
			m_errorMessage << "Error: Can not open file: " << filename << '\n';
			return false;
		}

		char* content = new char[static_cast<size_t>(size) + 1];
		file.read(content, size);

		content[size] = '\0';
		
		bool result = ReadSWC(content, graph);

		delete[] content;

		return result;
	}

	inline bool Parser::ReadSWC(std::istream &inStream, Graph& graph)
	{
		std::string str;
		char buffer[4096];
		while (inStream.read(buffer, sizeof(buffer)))
		{
			str.append(buffer, sizeof(buffer));
		}
		str.append(buffer, static_cast<unsigned int>(inStream.gcount()));
		m_errorMessage.clear();

		m_iterator = str.c_str();

		int countOfLines = 1;
		while (*m_iterator != '\0')
		{
			while (!AcceptEndOfLine() && *m_iterator != '\0')
			{
				NextSymbol();
			}
			countOfLines++;
		}

		graph.list_swc.clear();
		graph.hash_swc_ids.clear();
		graph.lines.clear();
		graph.segments.clear();
		graph.meta.clear();
		while 
	}


	inline std::string Parser::GetErrorMessage()
	{
		return m_errorMessage.str();
	}

	inline bool Generator::Write(std::ostream &outStream, const Graph& graph)
	{
		for (std::list<std::string>::const_iterator it = graph.meta.begin(); it != graph.meta.end(); ++it)
		{
			outStream << "#" << (*it) << '\n';
		}

		for (std::list<NeuronSWC>::const_iterator it = graph.list_swc.begin(); it != graph.list_swc.end(); ++it)
		{
			char buff[MaxLineSize];
			sprintf(buff, " %lld %d %.15g %.15g %.15g %.7g %lld", it->id, it->type, it->x, it->y, it->z, it->radius, it->parent);
			sprintf(buff, " #name:%s color:%s line_id:%d seg_id:%d seg_size:%d seg_in_id:%d  block_id:%d timestamp:%lld\n",
					it->name,
					it->color,
					it->line_id,
					it->seg_id,
					it->seg_size,
					it->seg_in_id,
					it->block_id,
					it->timestamp);
			outStream << buff;
		}

		return true;
	}

	inline bool Generator::Write(std::string& outString, const Graph& graph)
	{
		std::stringstream sstream;
		bool result = Write(sstream, graph);
		outString = sstream.str();
		return result;
	}

	inline bool Generator::WriteToFile(const char *filename, const Graph& graph)
	{
		std::ofstream file(filename, std::ios::binary);

		if (!file.is_open())
		{
			m_errorMessage.clear();
			m_errorMessage << "Error: Can not open file: " << filename << '\n';
			return false;
		}

		std::stringstream sstream;
		bool result = Write(sstream, graph);
		file << sstream.rdbuf();
		return result;
	}
}

#endif