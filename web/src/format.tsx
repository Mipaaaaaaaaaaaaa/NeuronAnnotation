import React, { useEffect, useState } from "react";
import { Button, Layout, Divider, Card, Space, Row, Col, Alert, message } from 'antd';
import Image from "./InteractiveImage";
import SrcTable from "./SrcTable";
import LoadAndSave from "./LoadAndSave";
import ToolsHeader from "./ToolsHeader";
import RenderSelecter from "./RenderSelecter";
import TreeVisualization from "./TreeVisualization";
import { useEffect, useRef, useState } from "react";
import "./style.css";

const { Header, Footer, Sider, Content } = Layout;
const _SOCKETLINK = "ws://127.0.0.1:12121/render";

const Format: React.FC = () => {

    const [data,setData] = useState(
        {
            "type":"structure",
            "graphs":[
                {
                "sub":[
                {   "key":0,
                    "index":1,
                    "lastEditTime":"2021-11-10 20:20:20",
                    "arc":[
                        {
                            "headVex" : 1,
                            "tailVex" : 3,
                            "distance" : 10.332
                        },
                        {
                            "headVex" : 1,
                            "tailVex" : 4,
                            "distance" : 7.25
                        }
                    ]
                },
                {   "key":1,
                    "index":3,
                    "lastEditTime":"2021-11-10 20:20:20",
                    "arc":[
                        {
                            "headVex" : 1,
                            "tailVex" : 3,
                            "distance" : 10.332
                        },
                    ]
                },
                {
                    "key":2,
                    "index":4,
                    "lastEditTime":"2021-11-10 20:20:21",
                    "arc":[
                        {
                            "headVex" : 1,
                            "tailVex" : 4,
                            "distance" : 7.25
                        },
                        {
                            "headVex" : 4,
                            "tailVex" : 5,
                            "distance" : 12
                        }
                    ]
                },
                {
                    "key":3,
                    "index":5,
                    "lastEditTime":"2021-4-29 20:00:21",
                    "arc":[
                        {
                            "headVex" : 4,
                            "tailVex" : 5,
                            "distance" : 12
                        },
                        {
                            "headVex" : 5,
                            "tailVex" : 6,
                            "distance" : 7
                        }
                    ]
                },{
                    "key":4,
                    "index":6,
                    "lastEditTime":"2021-4-29 08:00:54",
                    "arc":[
                        {
                            "headVex" : 5,
                            "tailVex" : 6,
                            "distance" : 7
                        }
                    ]
                }
                ],
                "color":"#cc00aa",
                "name":"haha",
                "index":0,
                "status":true,
                "key":0
                }
            ],
            "selectedVertexIndex":0,
            "selectedMapIndex":0,
            "selectedTool":0
        }
    );
    const [selectedMapIndex, setSelectedMapIndex] = useState(0);
    const [selectedVertexIndex, setSelectedVertexIndex] = useState(0);
    const [selectedTool,setSelectecTool] = useState(0);
    const handleToolsChange = (e)=>{
        console.log("handleToolsChange",e.target.value);
        setSelectecTool(e.target.value);
    }

    const rowSelection = {
        //设置默认选中的map
        selectedRowKeys: [0],
        onChange: (selectedRowKeys: React.Key[], selectedRows: any[]) => {
            changeData({selectedMapKey: selectedRows[0].index}); //index是服务器存储的顺序
            setSelectedMapIndex(selectedRows[0].key); //key是客户端存储的顺序
            console.log(`selectedRowKeys: ${selectedRowKeys}`, 'selectedRows: ', selectedRows[0]);
        },
    };

    const onClickJumpToVex = (record)=>{
        changeData({selectedVertexIndex : record.index});
        setSelectedVertexIndex(record.key);
    }

    const changeData = (_data) => {
        if(_data && data){
            console.log("changedata:",data)
            setData({...data,..._data});
            const ws = new WebSocket(_SOCKETLINK);
            ws.binaryType = "arraybuffer";
            ws.onopen = () => {
                console.log("连接成功，准备发送更新数据");
                ws.send(
                    JSON.stringify({
                        data: data
                    })
                );
            }
            ws.onerror = () =>{
                console.log("连接渲染服务器出错！");
                message.error("连接渲染服务器出错！");
            }
        }
    }
    return (
            <Card>
                <Row gutter={{ xs: 8, sm: 16, md: 24, lg: 32 }}>
                    <Col span={6}>
                    <RenderSelecter />
                        <Divider dashed />
                        <SrcTable 
                            rowSelection={rowSelection}
                            onClickJumpToVex={onClickJumpToVex}
                            data={data}
                            setData={changeData}
                            />
                        <Divider dashed />
                    </Col>
                    <Col span={10}>
                        <Row>
                            <Col span={20}>
                            <ToolsHeader handleToolsChange={handleToolsChange}/>
                            </Col>
                            <Col span={4}>
                                <LoadAndSave />
                            </Col>
                        </Row>
                        <Divider dashed />
                        <Row>
                            <Image
                                selectedTool={selectedTool}
                                setData={setData}
                            />
                        </Row>
                    </Col>
                    <Col offset={1} span={7}>
                        <TreeVisualization
                            data={data}
                            selectedMapIndex={selectedMapIndex}
                            selectedVertexIndex={selectedVertexIndex}
                        />
                    </Col>
                </Row>
                {/* <Row>
                <TreeVisualization/>
                </Row> */}
            </Card>
    )
};

export default Format;