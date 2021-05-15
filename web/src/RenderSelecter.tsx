import React from "react";
import { Button, Radio, Space, Divider } from 'antd';
import {BulbOutlined} from '@ant-design/icons'
import axios from 'axios';


const serverURL = "http://localhost:12121";
class RenderSelecter extends React.Component {
    constructor(props){
        super(props)
        this.state = {
            selectedRender: "MIP",
        };
    }

    handleRenderChange = (v:string) => {
        console.log("555555")
        this.setState({selectedRender:v});
        axios
            .post(serverURL+"/info",{              
                modify:{
                    selectedRender : v
                }
            })
            .then((res)=>{
                console.log("八八八")
                console.log(res);
            })
            .catch((e)=>{
                console.log(e);
            })

    };

    render(){
        const {selectedRender} = this.state;
        return (
            <div style={{margin:'0 20px'}}>
                <Space align="baseline" split={<Divider type="vertical" />}>
                    <h4><BulbOutlined />选择渲染方式</h4>
                        <Radio.Group value={selectedRender} onChange={(v)=>this.handleRenderChange(v.target.value)}>
                            <Radio.Button value="MIP">MIP</Radio.Button>
                            <Radio.Button value="DVR">DVR</Radio.Button>
                            <Radio.Button value="MUTI">MUTI</Radio.Button>
                        </Radio.Group>
                </Space>
          </div>
        );
    }
};

export default RenderSelecter;