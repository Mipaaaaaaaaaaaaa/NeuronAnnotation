import React from "react";
import { Button, Radio, Space, Divider } from 'antd';
import {BulbOutlined} from '@ant-design/icons'
class RenderSelecter extends React.Component {
    constructor(props){
        super(props)
        this.state = {
            selectedRender: "MIP",
        };
    }
    handleRenderChange = (v:string) => {
        this.setState({selectedRender:v});
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