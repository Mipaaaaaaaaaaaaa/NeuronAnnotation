import React from "react";
import { Radio } from 'antd';
import {
    DragOutlined,
    ScissorOutlined,
    EditOutlined,
    RadiusSettingOutlined,
    DeleteOutlined,
  } from '@ant-design/icons';

class ToolsHeader extends React.Component {
    constructor(props){
        super(props)
    }

    render(){
        return (
            <div>
                <Radio.Group defaultValue='0' onChange={this.props.handleToolsChange}>
                <Radio.Button value='0'><DragOutlined /></Radio.Button>
                <Radio.Button value='1'><EditOutlined /></Radio.Button>
                <Radio.Button value='2'><ScissorOutlined /></Radio.Button>
                <Radio.Button value='3'><RadiusSettingOutlined /></Radio.Button>
                <Radio.Button value='4'><DeleteOutlined /></Radio.Button>
            </Radio.Group>
          </div>
        );
    }
};

export default ToolsHeader;