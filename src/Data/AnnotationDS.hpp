//
// Created by wyz on 2021/5/3.
//

#ifndef NEURONANNOTATION_ANNOTATIONDS_HPP
#define NEURONANNOTATION_ANNOTATIONDS_HPP

#include<vector>
#include<list>
#include<memory>
#include<map>
#include<glm/glm.hpp>
#include<Poco/Mutex.h>
#include<Camera.hpp>

using namespace std;
class NeuronGraph;

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

// struct Vertex{
//     glm::vec3 pos;
//     int vertex_index;
//     NeuronGraph* graph;
// };
// struct Edge{
//     Vertex *v0,*v1;
//     NeuronGraph* graph;
//     int edge_index;
//     float length;
// };
// struct Line{
//     std::list<Edge*> edges;
//     int line_index;
//     NeuronGraph* graph;
// };

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
    double x,y,z;
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

    Vertex( Type type, double x, double y, double z, float radius) : type(type), radius(radius), x(x), y(y), z(z)
    {
        line_id = -1;
        hash_linked_seg_ids.clear();
        linked_vertex_ids.clear();
    };
    
    Vertex() {};

    double x,y,z;
    float radius;
    Type type;
    int64_t line_id;
    map<int, bool> hash_linked_seg_ids; //相关的线id
    map<int, bool> linked_vertex_ids; //相连的点id
} Vertex;

typedef struct Segment //路径中的单个线段
{
    int size;
    int line_id;
    int start_id;
    int end_id;
    map<int,int> segment_vertex_ids; //在线中的顶点在SWC中的索引id
    Segment(){
        size = 0;
        line_id = -1;
        start_id = -1;
        end_id = -1;	
    }
    Segment(int s, int l){
        size = s;
        line_id = l;
        start_id = -1;
        end_id = -1;
    };

} Segment;
		
struct Line : public BasicObj //Line是有关关键Vertex的集合
{
    map< int, Vertex > hash_vertexes;
    int block_id;
    Line(){
        id=0;
        color="#000000";
        selected=false;
        visible=true;
        name="";
        block_id = -1;
    }
};
	
class NeuronGraph : public BasicObj{
public:
    NeuronGraph(const char* filePath);
    NeuronGraph(){};
    // explicit NeuronGraph(int idx):graph_index(idx){}
    bool selectVertices(std::vector<int> idxes);
    bool selectEdges(std::vector<int> idxes);
    bool selectLines(std::vector<int> idxes);
    bool deleteCurSelectVertices();
    bool deleteCurSelectEdges();
    bool deleteCurSelectLines();
    bool addVertex(Vertex* v);
    bool addSegment(int id, Vertex* v);
    long int getNewVertexId();
    long int getNewSegmentId();
    long int getNewLineId();

    string file; //文件源
    vector<NeuronSWC> list_swc; //list_swc中间删除时，需要对hash_swc_id重新计算
    map<int,int> hash_swc_ids; //方便查询，点与相关联SWC文件的映射索引
    map<int,Line> lines; //路径合集（点合辑）
    map<int,Segment > segments; //关键点及非关键点的线段合集
    vector<string> meta; //memo

private:
    long int cur_max_line_id;
    long int cur_max_seg_id;
    long int cur_max_vertex_id;

    Poco::Mutex max_line_id_mutex;
    Poco::Mutex max_seg_id_mutex;
    Poco::Mutex max_vertex_id_mutex;
    Poco::Mutex list_and_hash_mutex;
    // int graph_index;
    // std::map<int,Segment*> segments;
    // std::map<int,Vertex*> vertices;
    // std::map<int,Line*> lines;
    // std::vector<Vertex*> cur_select_vertices;//last add or current pick
    // std::vector<Segment*> cur_select_segments;//last add edge or current select edge
    // std::vector<Line*> cur_select_lines;
    // int select_obj;//0 for nothing, 1 for point, 2 for edge, 3 for points, 4 for edges,5 for line,6 for lines
};

// class NeuronGraphDB{
// public:
//     NeuronGraphDB()=default;
// private:
//     bool isValid(int user_id) const{
//         return user_id==owner_id;
//     }
// private:
//     std::vector<std::unique_ptr<NeuronGraph>> graphs;
//     int owner_id;
// };

class NeuronPool{
public:
    bool addVertex(Vertex v);
    bool addVertex(float x, float y, float z);
    bool addLine();
    bool deleteLine();
    bool jumpToVertex(int id);
    void setGraph( NeuronGraph * pN){
        this->graph = pN;
    };
    void setUserId( int id ){
        this->user_id = id;
    }
private:
    Camera m_camera; //视角信息
    int m_selected_vertex_index; //当前编辑顶点
    int m_selected_line_index; //当前选择路径
    map<int,bool> line_id_visible; //路径可视映射

private:
    int user_id;
    NeuronGraph* graph;
    //std::shared_ptr<NeuronGraph> graph;
};


#endif //NEURONANNOTATION_ANNOTATIONDS_HPP
