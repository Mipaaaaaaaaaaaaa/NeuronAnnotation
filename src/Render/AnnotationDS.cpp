#include "Data/AnnotationDS.hpp"
#include "Data/SWCP.hpp"
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
    //生成新的segments
    long int segId = getNewSegmentId();
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
    vEndswc.block_id = lines[v.line_id].block_id;
    vEndswc.line_id = v.line_id;
    vEndswc.seg_id = segId;
    vEndswc.seg_in_id = 1;
    vEndswc.seg_size = 2;

    //连接两个点
    v.linked_vertex_ids[id] = true;
    lines[v.line_id].hash_vertexes[id].linked_vertex_ids[v.id] = true;
    v.hash_linked_seg_ids[segId] = true;
    lines[v.line_id].hash_vertexes[id].hash_linked_seg_ids[segId] = true;

    Segment sg;
    sg.id = segId;
    sg.line_id = v.line_id;
    sg.start_id = id;
    sg.end_id = v.id;
    sg.size = 2;
    sg.segment_vertex_ids[0] = id;
    sg.segment_vertex_ids[1] = v.id;
    segments[segId] = sg;

    list_and_hash_mutex.lock();
    {
        list_swc.push_back(vEndswc);
        hash_swc_ids[v.id] = list_swc.size() - 1;
    }
    list_and_hash_mutex.unlock();

    lines[v.line_id].hash_vertexes[v.id] = v;
    return true;
}