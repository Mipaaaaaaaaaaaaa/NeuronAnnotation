#include "AnnotationDS.hpp"
#include "SWCP.hpp"
#include <iostream>
#include <Poco/Mutex.h>

bool NeuronPool::addVertex(Vertex v){
   if( m_selected_line_index != -1 ){
       std::cout << "用户未选择路径" << std::endl;
       return false;
   }
   v.id = graph->getNewVertexId();
   v.name = graph->lines[m_selected_line_index].name;
   v.line_id = m_selected_line_index;
   if( m_selected_vertex_index == -1 ){ // this vertex is the first picked vertex in this line
        graph->addVertex(v);
   }else{ // draw a line
        v.linked_vertex_ids[m_selected_line_index] = true;
        graph->addSegment(m_selected_line_index,v);
   }
   m_selected_vertex_index = v.id;
}

NeuronGraph::NeuronGraph(const char * filePath){
    SWCP::Parser parser;
    bool result = parser.ReadSWCFromFile(filePath, this);
    if( result )std::cout << " Build Graph From File Successfully!" << std:endl;
    else std::cout << " Build Graph From File Error!" << std:endl;
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

bool NeuronGraph::addVertex(Vertex v){
    NeuronSWC swc;
    swc.id = v.id;
    swc.name = v.name = graph->lines[m_selected_line_index].name;
    swc.pn = v.pn = -1;
    swc.x = v.x;
    swc.y = v.y;
    swc.z = v.z;
    swc.r = v.r;
    swc.type = 1;
    swc.color = v.color = graph->lines[m_selected_line_index].color;
}

bool NeuronGraph::addSegment(int id, Vertex v){
    NeuronSWC vStartswc = list_swc[hash_swc_ids[id]];
    NeuronSWC vEndswc;

    vEndswc.id = v.id;
    vEndswc.name = v.name;
    vEndswc.color = v.color;
    vEndswc.x = v.x;
    vEndswc.y = v.y;
    vEndswc.z = v.z;
    vEndswc.r = v.r;
    vEndswc.type = v.type;
    //路径生成算法之后修改该部分
    vEndswc.pn = id;

    v.linked_vertex_ids[id] = true;
    lines[v.line_id].hash_vertexes[id].linked_vertex_ids[v.id] = true; //连接两个点
    lines[v.line_id].hash_vertexes[v.id] = v;
    list_and_hash_mutex.lock();
    {
        list_swc.push_back(vEndswc);
        hash_swc_ids[v.id] = list_swc.size() - 1;
    }
    list_and_hash_mutex.unlock();

}