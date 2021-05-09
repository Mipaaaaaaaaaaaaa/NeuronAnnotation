#include <AnnotationDS.hpp>
#include <SWCP.hpp>
#include <iostream>
#include <Poco/Mutex.h>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Array.h>
#include <Poco/Dynamic/Var.h>

using namespace std;
using Poco::Dynamic::Var;
using Poco::JSON::Object;

string NeuronPool::getLinestoJson(){
    return graph->getLinestoJson(this);
}

string NeuronGraph::getLinestoJson(NeuronPool * np){
    Poco::JSON::Object package;
    package.set("type","structure");

    Poco::JSON::Array graphs;
    int count = 0;
    for( auto it = lines.begin() ; it != lines.end() ; it ++ ){
        Poco::JSON::Object line;
        line.set("name",it->second.name);
        line.set("color",it->second.color);
        line.set("index",it->first);
        line.set("key",count++);
        line.set("status",np->getLineVisible(it->first));
        Poco::JSON::Array Vertexes;
        int v_count = 0;
        for( auto v = it->second.hash_vertexes.begin() ; v != it->second.hash_vertexes.end() ; v ++ ){
            Poco::JSON::Object V;
            V.set("index",v->first);
            V.set("key",v_count++);
            V.set("lastEditTime",v->second.timestamp);
            Poco::JSON::Array arc;
            for( auto seg = v->second.hash_linked_seg_ids.begin() ; seg != v->second.hash_linked_seg_ids.end() ; seg ++ ){
                Poco::JSON::Object a;
                a.set("headVex",segments[seg->first].start_id);
                a.set("tailVex",segments[seg->first].end_id);
                a.set("distance",getDistance(seg->first));
                arc.add(a);
            }
            V.set("arc",arc);
            Vertexes.add(V);
        }
        line.set("sub",Vertexes);
        graphs.add(line);
    }
    package.set("graphs",graphs);

    Poco::Dynamic::Var json(package);
    return json.toString();
}

double NeuronGraph::getDistance(int seg_id){
    double distance = 0;
    NeuronSWC *_last = NULL;
    NeuronSWC *_now = NULL;
    for( auto v = segments[seg_id].segment_vertex_ids.begin() ; v != segments[seg_id].segment_vertex_ids.end() ; v++ ){
        _now = &list_swc[hash_swc_ids[v->second]];
        if( _last == NULL ){
            _last = _now;
            continue;
        }else{
            distance += getDistance(_last->x, _last->y, _last->z, _now->x, _now->y, _now->z );
            _last = _now;
        }
    }
    return distance;
}

double NeuronGraph::getDistance( float x1, float y1, float z1, float x2, float y2, float z2 ){
    double square1=(x1-x2)*(x1-x2);
    double square2=(y1-y2)*(y1-y2);
    double square3=(z1-z2)*(z1-z2);
    double sum=square1+square2+square3;
    double result=sqrt(sum);
    return result;
}


bool NeuronPool::getLineVisible(int id){
    if( line_id_visible.find(id) != line_id_visible.end() ){
        return line_id_visible[id];
    }
    return true;
}

bool NeuronPool::addVertex(Vertex *v){
   if( m_selected_line_index != -1 ){
       std::cout << "用户未选择路径" << std::endl;
       return false;
   }
   v->id = graph->getNewVertexId();
   v->name = graph->lines[m_selected_line_index].name;
   v->line_id = m_selected_line_index;
   if( m_selected_vertex_index == -1 ){ // this vertex is the first picked vertex in this line
        if( graph->addVertex(v) ){
            m_selected_vertex_index = v->id;
            return true;
        }
   }else{ // draw a line
        v->linked_vertex_ids[m_selected_line_index] = true;
        if( graph->addSegment(m_selected_line_index,v) ){
            m_selected_vertex_index = v->id;
            return true;
        }
   }
   return false;
}

void NeuronPool::selectVertex( int id ){
    m_selected_vertex_index = id;
}

void NeuronPool::selectLine( int id ){
    m_selected_line_index = id;
}

bool NeuronPool::addVertex(float x, float y, float z){
    Vertex *v = new Vertex();
    if( v ){
        v->x = x;
        v->y = y;
        v->z = z;
        return addVertex(v);
    }
    return false;
}


NeuronGraph::NeuronGraph(const char * filePath){
    SWCP::Parser parser;
    this->cur_max_vertex_id = -1;
    this->cur_max_seg_id = -1;
    this->cur_max_line_id = -1;
    bool result = parser.ReadSWCFromFile(filePath, *this);
    if( result )std::cout << " Build Graph From File Successfully!" << std::endl;
    else std::cout << " Build Graph From File Error!" << std::endl;
}

long int NeuronGraph::getNewVertexId(){
    long int getId;
    max_vertex_id_mutex.lock();
    getId = ++cur_max_vertex_id;
    max_vertex_id_mutex.unlock();
    return getId;
}

long int NeuronGraph::getNewSegmentId(){
    long int getId;
    max_seg_id_mutex.lock();
    getId = ++cur_max_seg_id;
    max_seg_id_mutex.unlock();
    return getId;
}

long int NeuronGraph::getNewLineId(){
    long int getId;
    max_line_id_mutex.lock();
    getId = ++cur_max_line_id;
    max_line_id_mutex.unlock();
    return getId;
}

bool NeuronGraph::addVertex(Vertex *v){
    NeuronSWC swc;
    swc.id = v->id;
    swc.name = v->name = lines[v->line_id].name;
    swc.pn = -1;
    swc.x = v->x;
    swc.y = v->y;
    swc.z = v->z;
    swc.r = 1;
    swc.type = Type(1);
    swc.color = v->color = lines[v->line_id].color;
    list_and_hash_mutex.lock();
    {
        list_swc.push_back(swc);
        hash_swc_ids[v->id] = list_swc.size() - 1;
    }
    list_and_hash_mutex.unlock();
    lines[v->line_id].hash_vertexes[v->id] = *v;
    return true;
}

bool NeuronGraph::addSegment(int id, Vertex *v){
    NeuronSWC vStartswc = list_swc[hash_swc_ids[id]];
    NeuronSWC vEndswc;
    //生成新的segments
    long int segId = getNewSegmentId();
    vEndswc.id = v->id;
    vEndswc.name = v->name;
    vEndswc.color = v->color;
    vEndswc.x = v->x;
    vEndswc.y = v->y;
    vEndswc.z = v->z;
    vEndswc.r = 1;
    vEndswc.type = v->type;
    //路径生成算法之后修改该部分
    vEndswc.pn = id;
    vEndswc.block_id = lines[v->line_id].block_id;
    vEndswc.line_id = v->line_id;
    vEndswc.seg_id = segId;
    vEndswc.seg_in_id = 1;
    vEndswc.seg_size = 2;

    //连接两个点
    v->linked_vertex_ids[id] = true;
    lines[v->line_id].hash_vertexes[id].linked_vertex_ids[v->id] = true;
    v->hash_linked_seg_ids[segId] = true;
    lines[v->line_id].hash_vertexes[id].hash_linked_seg_ids[segId] = true;

    Segment sg;
    sg.id = segId;
    sg.line_id = v->line_id;
    sg.start_id = id;
    sg.end_id = v->id;
    sg.size = 2;
    sg.segment_vertex_ids[0] = id;
    sg.segment_vertex_ids[1] = v->id;
    segments[segId] = sg;

    list_and_hash_mutex.lock();
    {
        list_swc.push_back(vEndswc);
        hash_swc_ids[v->id] = list_swc.size() - 1;
    }
    list_and_hash_mutex.unlock();

    lines[v->line_id].hash_vertexes[v->id] = *v;
    return true;
}

long int NeuronGraph::getCurMaxVertexId(){
    return cur_max_vertex_id;
}

long int NeuronGraph::getCurMaxLineId(){
    return cur_max_line_id;
}

long int NeuronGraph::getCurMaxSegmentId(){
    return cur_max_seg_id;
}

void NeuronGraph::setMaxVertexId(long int id){
    cur_max_vertex_id = id;
}

void NeuronGraph::setMaxLineId(long int id){
    cur_max_line_id = id;
}

void NeuronGraph::setMaxSegmentId(long int id){
    cur_max_seg_id = id;
}