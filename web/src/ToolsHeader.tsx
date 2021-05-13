import React from "react";
import { Radio, Tooltip } from 'antd';
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
                <Radio.Group defaultValue='0' onChange={this.props.handleToolsChange} size="large">
                <Tooltip
                    placement="bottom"
                    title="拖动模式"
                    arrowPointAtCenter
                    color="blue"
                    >
                    <Radio.Button value='0' ><DragOutlined /></Radio.Button>
                </Tooltip>
                <Tooltip
                    placement="bottom"
                    title="点标注模式"
                    arrowPointAtCenter
                    color="blue"
                    >
                <Radio.Button value='1'><EditOutlined /></Radio.Button>
                </Tooltip>
                <Tooltip
                    placement="bottom"
                    title="剪切模式"
                    arrowPointAtCenter
                    color="blue"
                    >
                <Radio.Button value='2'><ScissorOutlined /></Radio.Button>
                </Tooltip>
                <Tooltip
                    placement="bottom"
                    title="选框模式"
                    arrowPointAtCenter
                    color="blue"
                    >
                <Radio.Button value='3'><RadiusSettingOutlined /></Radio.Button>
                </Tooltip>
                <Tooltip
                    placement="bottom"
                    title="消除模式"
                    arrowPointAtCenter
                    color="blue"
                    >
                <Radio.Button value='4'><DeleteOutlined /></Radio.Button>
                </Tooltip>
            </Radio.Group>
          </div>
        );
    }
};

export default ToolsHeader;