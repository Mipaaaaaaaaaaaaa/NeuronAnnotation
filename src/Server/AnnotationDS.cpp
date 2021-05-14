#include <AnnotationDS.hpp>
#include <SWCP.hpp>
#include <iostream>
#include <Poco/Mutex.h>
#include <Poco/JSON/JSON.h>
#include <Poco/JSON/Object.h>
#include <Poco/JSON/Parser.h>
#include <Poco/JSON/Array.h>
#include <Poco/Dynamic/Var.h>
#include <DataBase.hpp>

using namespace std;
using Poco::Dynamic::Var;
using Poco::JSON::Object;

string NeuronPool::getLinestoJson(){
    return graph->getLinestoJson(this);
}

// need to initialize the listswc and hashswc first
// this function will format graph from basic swc file strings
bool NeuronGraph::formatGraphFromSWCList(){
    if( list_swc.size() == 0 ) return true;
    std::map<int,vector<int>> vertexLinkedCount;
    for( int i = 0 ; i < list_swc.size() ; i ++ ){// new line
        time_t timestamp;
        list_swc[i].timestamp = time(&timestamp);
        if( list_swc[i].pn == -1 ){
            auto line_id = getNewLineId();
            Vertex v;
            list_swc[i].line_id = lines[line_id].id = v.line_id = line_id;
            list_swc[i].color = lines[line_id].color = v.color = "#aa0000";
            list_swc[i].name = lines[line_id].name = v.name = "default" + std::to_string(line_id);
            list_swc[i].user_id = lines[line_id].user_id = -1;
            v.id = list_swc[i].id;
            v.radius = list_swc[i].radius;
            v.x = list_swc[i].x;
            v.y = list_swc[i].y;
            v.z = list_swc[i].z;
            v.type = list_swc[i].type;
            lines[line_id].hash_vertexes[list_swc[i].id] = v;
        }else{
            vertexLinkedCount[list_swc[i].pn].push_back(list_swc[i].id);
            vertexLinkedCount[list_swc[i].id].push_back(list_swc[i].pn);
        }
    }
    for( int index = 0 ; index < list_swc.size() ; index ++ ){
        if( list_swc[index].pn == -1 ){ //起点
            int seg_id = getNewSegmentId();
            list_swc[index].seg_id = seg_id;
            list_swc[index].seg_in_id = 0;
            lines[list_swc[index].line_id].hash_vertexes[list_swc[index].id].hash_linked_seg_ids[seg_id] = seg_id;
            segments[seg_id].segment_vertex_ids[0] = list_swc[index].id;
            segments[seg_id].start_id = list_swc[index].id;
            segments[seg_id].id = seg_id;
            segments[seg_id].line_id = list_swc[index].line_id;
            segments[seg_id].name = list_swc[index].name;
            segments[seg_id].color = list_swc[index].color;
            segments[seg_id].size = -1; //最后计算
            return seg_id;
        }
        if( vertexLinkedCount[list_swc[index].id].size() == 1 ){ //顶点
            //终点，关键节点
            int seg_id;
            if( list_swc[hash_swc_ids[list_swc[index].pn]].seg_size != -1 && list_swc[hash_swc_ids[list_swc[index].pn]].seg_in_id == list_swc[hash_swc_ids[list_swc[index].pn]].seg_size - 2){
                //上一段已经结束，该段为新段 针对只有两个点的节点
                seg_id = getNewSegmentId();
                list_swc[index].seg_id = seg_id;
                list_swc[index].color = list_swc[index].color;
                list_swc[index].name = list_swc[hash_swc_ids[list_swc[index].pn]].name;
                list_swc[index].user_id = list_swc[index].user_id;
                list_swc[index].line_id = list_swc[hash_swc_ids[list_swc[index].pn]].line_id;
                list_swc[index].seg_in_id = 1;
                list_swc[index].seg_size = 2;
                lines[list_swc[index].line_id].hash_vertexes[list_swc[index].pn].hash_linked_seg_ids[seg_id] = true;
                segments[seg_id].start_id = list_swc[index].pn;
                segments[seg_id].end_id = list_swc[index].id;
                segments[seg_id].segment_vertex_ids[0] = list_swc[index].pn;
                segments[seg_id].segment_vertex_ids[1] = list_swc[index].id;
                segments[seg_id].id = seg_id;
                segments[seg_id].line_id = list_swc[index].line_id;
                segments[seg_id].name = list_swc[index].name;
                segments[seg_id].color = list_swc[index].color;
                segments[seg_id].size = 2;
                
            }else{
                seg_id = list_swc[hash_swc_ids[list_swc[index].pn]].seg_id;
                list_swc[index].color = list_swc[hash_swc_ids[list_swc[index].pn]].color;
                list_swc[index].name = list_swc[hash_swc_ids[list_swc[index].pn]].name;
                list_swc[index].user_id = list_swc[hash_swc_ids[list_swc[index].pn]].user_id;
                list_swc[index].line_id = segments[seg_id].line_id;
                list_swc[index].seg_id = seg_id;
                list_swc[index].seg_in_id = list_swc[hash_swc_ids[list_swc[index].pn]].seg_in_id + 1;
                segments[seg_id].end_id = list_swc[index].id;
                segments[seg_id].segment_vertex_ids[list_swc[index].seg_in_id] = list_swc[index].id;
                segments[seg_id].size = list_swc[index].seg_in_id + 1;
                for( auto v = segments[seg_id].segment_vertex_ids.begin() ; v != segments[seg_id].segment_vertex_ids.end() ; v ++ ){ //更新总长度
                    if(list_swc[hash_swc_ids[v->second]].seg_id == seg_id)list_swc[hash_swc_ids[v->second]].seg_size = segments[seg_id].size;
                }
            }
            Vertex v;
            v.line_id = list_swc[index].line_id;
            v.id = list_swc[index].id;
            v.radius = list_swc[index].radius;
            v.x = list_swc[index].x;
            v.y = list_swc[index].y;
            v.z = list_swc[index].z;
            v.type = list_swc[index].type;
            v.hash_linked_seg_ids[seg_id] = true;
            v.linked_vertex_ids[segments[seg_id].start_id] = true;
            lines[v.line_id].hash_vertexes[segments[seg_id].start_id].linked_vertex_ids[v.id] = true;
            lines[v.line_id].hash_vertexes[v.id] = v;
        }else if( vertexLinkedCount[list_swc[index].id].size() == 2 ){ //段中间节点
            if( list_swc[hash_swc_ids[list_swc[index].pn]].seg_size != -1 ){//&& segments[list_swc[hash_swc_ids[list_swc[index].pn]].seg_id].end_id == list_swc[index].pn){
                //上一段已经结束，该段为新段
                int seg_id = getNewSegmentId();
                list_swc[index].seg_id = seg_id;
                list_swc[index].color = list_swc[index].color;
                list_swc[index].name = list_swc[index].name;
                list_swc[index].user_id = list_swc[index].user_id;
                list_swc[index].line_id = list_swc[hash_swc_ids[list_swc[index].pn]].line_id;
                list_swc[index].seg_in_id = 1;

                lines[list_swc[index].line_id].hash_vertexes[list_swc[index].pn].hash_linked_seg_ids[seg_id] = true;
                segments[seg_id].start_id = list_swc[index].pn;
                segments[seg_id].segment_vertex_ids[0] = list_swc[index].pn;
                segments[seg_id].segment_vertex_ids[1] = list_swc[index].id;
                segments[seg_id].id = seg_id;
                segments[seg_id].line_id = list_swc[index].line_id;
                segments[seg_id].name = list_swc[index].name;
                segments[seg_id].color = list_swc[index].color;
                segments[seg_id].size = -1;
            }else{ //中间点
                int seg_id = list_swc[hash_swc_ids[list_swc[index].pn]].seg_id;
                list_swc[index].seg_id = seg_id;
                list_swc[index].seg_in_id = list_swc[hash_swc_ids[list_swc[index].pn]].seg_in_id + 1;
                list_swc[index].color = list_swc[hash_swc_ids[list_swc[index].pn]].color;
                list_swc[index].name = list_swc[hash_swc_ids[list_swc[index].pn]].name;
                list_swc[index].user_id = list_swc[hash_swc_ids[list_swc[index].pn]].user_id;
                list_swc[index].line_id = segments[seg_id].line_id;
                segments[seg_id].segment_vertex_ids[list_swc[index].seg_in_id] = list_swc[index].id;
            }

        }else if( vertexLinkedCount[list_swc[index].id].size() > 2 ){ //关键节点，对每个子节点分段
            //该节点为旧的终点，新的起点
            int pn = list_swc[index].pn;
            segments[list_swc[hash_swc_ids[pn]].seg_id].end_id = list_swc[index].id;
            list_swc[index].color = list_swc[hash_swc_ids[list_swc[index].pn]].color;
            list_swc[index].name = list_swc[hash_swc_ids[list_swc[index].pn]].name;
            list_swc[index].user_id = list_swc[hash_swc_ids[list_swc[index].pn]].user_id;
            list_swc[index].line_id = list_swc[hash_swc_ids[pn]].line_id;
            list_swc[index].seg_id = list_swc[hash_swc_ids[pn]].seg_id;
            segments[list_swc[pn].seg_id].size = list_swc[hash_swc_ids[pn]].seg_in_id + 2;
            segments[list_swc[pn].seg_id].segment_vertex_ids[list_swc[hash_swc_ids[pn]].seg_in_id+1] = list_swc[index].id;
            //更新所有前seg节点长度
            for( auto v = segments[list_swc[pn].seg_id].segment_vertex_ids.begin() ; v != segments[list_swc[pn].seg_id].segment_vertex_ids.end() ; v ++ ){ //更新总长度
                    if(list_swc[hash_swc_ids[v->second]].seg_id == list_swc[pn].seg_id ) list_swc[hash_swc_ids[v->second]].seg_size = segments[list_swc[pn].seg_id].size;
            }
            Vertex v; //新节点
            v.line_id = list_swc[index].line_id;
            v.id = list_swc[index].id;
            v.radius = list_swc[index].radius;
            v.x = list_swc[index].x;
            v.y = list_swc[index].y;
            v.z = list_swc[index].z;
            v.type = list_swc[index].type;
            v.hash_linked_seg_ids[list_swc[hash_swc_ids[pn]].seg_id] = true;
            v.linked_vertex_ids[segments[list_swc[hash_swc_ids[pn]].seg_id].start_id] = true;
            lines[v.line_id].hash_vertexes[segments[list_swc[hash_swc_ids[pn]].seg_id].start_id].linked_vertex_ids[v.id] = true;
            lines[v.line_id].hash_vertexes[v.id] = v;
            return 0;
        }
        return true;
    }
    return true;
}


string NeuronGraph::getLinestoJson(NeuronPool * np){
    Poco::JSON::Object package;
    package.set("type","structure");
    Poco::JSON::Parser parser;
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
    std::string result = DataBase::showAllTables();
    Poco::Dynamic::Var cur = parser.parse(result);
    Poco::JSON::Object curObj = *cur.extract<Poco::JSON::Object::Ptr>();
    cur = curObj.get("cursor");
    curObj = *cur.extract<Poco::JSON::Object::Ptr>();
    Poco::Dynamic::Var batch = curObj.get("firstBatch");
    Poco::JSON::Array batchArray = *batch.extract<Poco::JSON::Array::Ptr>();

    Poco::JSON::Array tableList;
    for( int i = 0 ; i < batchArray.size(); i ++ ){
        Poco::JSON::Object table;
        Poco::Dynamic::Var t = batchArray.get(i);
        Poco::JSON::Object tObj = *t.extract<Poco::JSON::Object::Ptr>();
        table.set("value",tObj.get("name").toString());
        table.set("label",tObj.get("name").toString());
        table.set("title",tObj.get("name").toString());
        tableList.add(table);
    }
    package.set("tableList",tableList); 
    package.set("graphs",graphs);
    package.set("selectedVertexIndex",np->getSelectedVertexIndex());
    package.set("selectedMapIndex",np->getSelectedLineIndex());
    package.set("selectedTableName",tableName);
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

bool NeuronPool::addLine(){
    int line_id = graph->addLine();
    if( line_id == -1 ) return false;
    m_selected_line_index = line_id;
    m_selected_vertex_index = -1;
    return true;
}

long int NeuronGraph::addLine(){
    int line_id = getNewLineId();
    if( line_id == -1 ) return -1;

    Line l;
    stringstream fmt;
    l.id = line_id;
    fmt << "deault" << line_id;
    l.name = fmt.str();
    l.color = "#aa0000";
    l.user_id = 1;
    lines[line_id] = l;
    return line_id;
}

bool NeuronPool::addVertex(Vertex *v){
   if( m_selected_line_index == -1 ){
       std::cout << "用户未选择路径" << std::endl;
       return false;
   }
   v->id = graph->getNewVertexId();
   v->name = graph->lines[m_selected_line_index].name;
   v->color = graph->lines[m_selected_line_index].color;
   v->line_id = m_selected_line_index;
   if( m_selected_vertex_index == -1 ){ // this vertex is the first picked vertex in this line
        if( graph->addVertex(v) ){
            m_selected_vertex_index = v->id;
            return true;
        }
   }else{ // draw a line
        v->linked_vertex_ids[m_selected_vertex_index] = true;
        if( graph->addSegment(m_selected_vertex_index,v) ){
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

NeuronGraph::NeuronGraph(const char * filePath, const char * tableName){
    SWCP::Parser parser;
    this->tableName = tableName;
    this->cur_max_vertex_id = -1;
    this->cur_max_seg_id = -1;
    this->cur_max_line_id = -1;
    bool result = parser.ReadSWCFromFile(filePath, *this,0);
    if( result )std::cout << " Build Graph From File Successfully!" << std::endl;
    else std::cout << " Build Graph From File Error!" << std::endl;
}


NeuronGraph::NeuronGraph(const char * string, int type){
    if( type == 0 ){// built by DataBase
        SWCP::Parser parser;
        this->tableName = string;
        this->cur_max_vertex_id = -1;
        this->cur_max_seg_id = -1;
        this->cur_max_line_id = -1;
        std::string str = DataBase::getSWCFileStringFromTable(string);
        bool result = parser.ReadSWC(str.c_str(), *this,0);
        if( result )std::cout << " Build Graph From File Successfully!" << std::endl;
        else std::cout << " Build Graph From File Error!" << std::endl;
    }else{
        SWCP::Parser parser;
        this->tableName = "default";
        this->cur_max_vertex_id = -1;
        this->cur_max_seg_id = -1;
        this->cur_max_line_id = -1;
        bool result = parser.ReadSWCFromFile(string, *this,0);
        if( result )std::cout << " Build Graph From File Successfully!" << std::endl;
        else std::cout << " Build Graph From File Error!" << std::endl;
    }
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
    time_t t;
    swc.timestamp = v->timestamp = time(&t);
    swc.color = v->color = lines[v->line_id].color;
    list_and_hash_mutex.lock();
    {
        list_swc.push_back(swc);
        hash_swc_ids[v->id] = list_swc.size() - 1;
    }
    list_and_hash_mutex.unlock();
    lines[v->line_id].hash_vertexes[v->id] = *v;
    if( DataBase::insertSWC(swc,tableName) ) return true;
    return false;
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
    vEndswc.user_id = lines[v->line_id].user_id;
    vEndswc.line_id = v->line_id;
    vEndswc.seg_id = segId;
    vEndswc.seg_in_id = 1;
    vEndswc.seg_size = 2;
    time_t t;
    vEndswc.timestamp = v->timestamp = time(&t);
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
    if( DataBase::insertSWC(vEndswc,tableName) ) return true;
    return false;
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

int NeuronPool::getSelectedLineIndex(){
    return m_selected_line_index;
}

int NeuronPool::getSelectedVertexIndex(){
    return m_selected_vertex_index;
}

void NeuronPool::initSelectedLineIndex(){
    m_selected_line_index = graph->getDefaultSelectedLineIndex();
}

void NeuronPool::initSelectedVertexIndex(){
    if( m_selected_line_index == -1 )initSelectedLineIndex();
    m_selected_vertex_index = graph->getDefaultSelectedVertexIndex(m_selected_line_index);
}

int NeuronGraph::getDefaultSelectedLineIndex(){
    auto it = lines.begin();
    if( it != lines.end() ) return it->first;
    return -1;
}

int NeuronGraph::getDefaultSelectedVertexIndex(int line_id){
    if( line_id == -1 ) return -1;
    auto v = lines[line_id].hash_vertexes.begin();
    if( v != lines[line_id].hash_vertexes.end() ) return v->first;
    return -1;
}

void NeuronPool::setCamera(Camera c){
    m_camera = c;
    b_set_camera = true;
}
Camera NeuronPool::getCamera(){
    return m_camera;
}

bool NeuronPool::changeVisible(int line_id, bool visible){
    line_id_visible[line_id] = visible;
    return true;
}

bool NeuronPool::changeColor(int line_id, string color){
    return graph->changeColor(line_id,color);
}

bool NeuronPool::changeName(int line_id, string name){
    return graph->changeName(line_id,name);
}

bool NeuronGraph::changeName(int line_id, string name){
    lines[line_id].name = name;
    std::vector<std::shared_ptr<NeuronSWC> > modifySWCs;
    for( auto v = lines[line_id].hash_vertexes.begin() ; v != lines[line_id].hash_vertexes.end() ; v++ ){
        v->second.name = name;
        NeuronSWC *swc = &list_swc[hash_swc_ids[v->second.id]];
        modifySWCs.push_back(make_shared<NeuronSWC>(*swc));
        swc->name = name;
        for( auto seg = v->second.hash_linked_seg_ids.begin() ; seg != v->second.hash_linked_seg_ids.end() ; seg++ ){
            Segment *s = &segments[seg->first];
            s->name = name;
            for( auto p = s->segment_vertex_ids.begin() ; p != s->segment_vertex_ids.end() ; p ++ ){
                NeuronSWC *pSWC = &list_swc[hash_swc_ids[p->second]];
                pSWC->name = name;
            }
        }
    }
    if( DataBase::modifySWCs(modifySWCs,tableName) ) return true;
    else return false;
}

bool NeuronGraph::changeColor(int line_id, string color){
    lines[line_id].color = color;
    for( auto v = lines[line_id].hash_vertexes.begin() ; v != lines[line_id].hash_vertexes.end() ; v++ ){
        v->second.color = color;
        NeuronSWC *swc = &list_swc[hash_swc_ids[v->second.id]];
        swc->color = color;
        for( auto seg = v->second.hash_linked_seg_ids.begin() ; seg != v->second.hash_linked_seg_ids.end() ; seg++ ){
            Segment *s = &segments[seg->first];
            s->color = color;
            for( auto p = s->segment_vertex_ids.begin() ; p != s->segment_vertex_ids.end() ; p ++ ){
                NeuronSWC *pSWC = &list_swc[hash_swc_ids[p->second]];
                pSWC->color = color;
            }
        }
    }
    return true;
}

bool NeuronPool::deleteLine(int line_id){
    return graph->deleteLine(line_id);
}

bool NeuronGraph::deleteLine(int line_id){
    Line *l = &lines[line_id];
    lines.erase(line_id);
    for( auto v = lines[line_id].hash_vertexes.begin() ; v != lines[line_id].hash_vertexes.end() ; v++ ){
        //NeuronSWC *swc = &list_swc[hash_swc_ids[v->second.id]]; 数据库删除该节点TODO
        for( auto seg = v->second.hash_linked_seg_ids.begin() ; seg != v->second.hash_linked_seg_ids.end() ; seg++ ){
            Segment *s = &segments[seg->first];
            for( auto p = s->segment_vertex_ids.begin() ; p != s->segment_vertex_ids.end() ; p ++ ){
                // NeuronSWC *pSWC = &list_swc[hash_swc_ids[p->second]]; 数据库删除该节点TODO
            }
            free(s);
        }
        free(&v->second);
    }
    free(l);
    return true;
}

bool NeuronPool::hasCamera(){
    return b_set_camera;
}

bool NeuronPool::changeTable(string tableName){
    if( graphs_pool->find(tableName) != graphs_pool->end() ){
        graph = graphs_pool->at(tableName);
        initSelectedLineIndex();
        initSelectedVertexIndex();
        return true;
    }else{
        (*graphs_pool)[tableName] = make_shared<NeuronGraph>(tableName.c_str(),0);
        graph = graphs_pool->at(tableName);
        initSelectedLineIndex();
        initSelectedVertexIndex();
        return true;
    }
    return false;
}