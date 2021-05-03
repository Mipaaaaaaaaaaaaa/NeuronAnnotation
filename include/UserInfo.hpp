#ifndef NEURONANNOTATION_USERINFO_H
#define NEURONANNOTATION_USERINFO_H

#include <map>
#include <string>
#include "Camera.hpp"
using namespace std;

//服务器管理的用户数据包
class UserInfo{
public:
    string m_conn_id;
    Camera m_camera; //视角信息
    int m_selectedVertexIndex; //当前编辑顶点
    int m_selectedMapIndex; //当前选择路径
    map<int,bool> m_visible; //路径可视的映射
    UserInfo( string conn_id, Camera camera, int selectedVertexIndex, int selectedMapIndex, vector<int> mapIndex)
    :m_conn_id(conn_id)
    ,m_camera(camera)
    ,m_selectedVertexIndex(selectedVertexIndex)
    ,m_selectedMapIndex(selectedMapIndex)
    {
        for( int i = 0 ; i < mapIndex.size() ; i ++ ){
            m_visible.insert({mapIndex[i],true}); //初始化可视化信息
        }
    }

    void setVisible(int index, bool visible){
        m_visible[index] = visible;
    }

    void deleteMap(int index){ //删除图时删除图对应的可视情况
        m_visible.erase(index);
    }

    void addMap(int index){ //添加视图
        m_visible[index] = true;
    }
};


#endif