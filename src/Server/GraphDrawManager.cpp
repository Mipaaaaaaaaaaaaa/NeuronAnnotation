#include <AnnotationDS.hpp>

GraphDrawManager::InitGraphDrawManager(){
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferStorage(GL_ARRAY_BUFFER, MAX_VERTEX_NUMBER * sizeof(float), nullptr, GL_DYNAMIC_STORAGE_BIT);
    for( int i = 0 ; i < graph->list_swc.size() ; i ++  ){
        glNamedBufferSubData(vbo, i * sizeof(float) * 3,
                         3 * sizeof(float), {graph->list_swc[i].x,graph->list_swc[i].y,graph->list_swc[i].z});
    }
    for( auto line : graph->lines ){
        line_num_of_path[line.first] = 0; //初始化路径的线数
    }
    for( auto seg = graph->segments.begin() ; seg != graph->segments.end() ; seg ++ ){
        if( hash_lineid_vao_ebo.find(seg->second.line_id) == hash_lineid_vao_ebo.end() ){ //该条线未初始化
            GLuint vao, ebo;
            glGenVertexArrays(1, &vao);
            glGenBuffers(1, &ebo);
            glBindVertexArray(vao);
            glBindBuffer(GL_ARRAY_BUFFER, vbo); //绑定同一个图的vbo
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float),(void *)0);
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
            glBufferStorage(GL_ELEMENT_ARRAY_BUFFER, 1024 * 2 * sizeof(uint64_t), nullptr, GL_DYNAMIC_STORAGE_BIT);
            hash_lineid_vao_ebo[seg->second.line_id] == std::pair(vao,ebo);
        }
        GLuint ebo = hash_lineid_vao_ebo[seg->second.line_id].second;
        uint64_t head;
        for( auto v : seg->second.segment_vertex_ids){
            if (v.first == 0){
                head = graph->hash_swc_ids[v.second];
            }
            uint64_t idx[2] = {head,graph->hash_swc_ids[v.second]};
            head = graph->hash_swc_ids[v.second];
            glNamedBufferSubData(ebo,
                                line_num_of_path[seg->second.line_id] * 2 * sizeof(uint64_t),
                                2 * sizeof(uint64_t), idx);
            line_num_of_path[seg->second.line_id]++;
        }
    }//遍历所有segment
}
