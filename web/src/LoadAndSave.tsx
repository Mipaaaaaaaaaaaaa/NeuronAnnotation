import React from "react";
import { Space, Typography, Divider, Button, Select, Tooltip } from 'antd';
import { DownloadOutlined } from '@ant-design/icons';
import { Upload, message } from 'antd';
import { UploadOutlined } from '@ant-design/icons';
import { withSuccess } from "antd/lib/modal/confirm";

const _SOCKETLINK = "ws://10.76.3.92:12121/render";
const { Option } = Select;
const LoadAndSave: React.FC = (props) => {
  const UPLOAD = {
    name: 'file',
    action: 'https://www.mocky.io/v2/5cc8019d300000980a055e76',
    headers: {
      authorization: 'authorization-text',
    },
    showUploadList: false,
    showPreviewIcon: true,
    onChange(info) {
      if (info.file.status !== 'uploading') {
        console.log(info.file, info.fileList);
      }
      if (info.file.status === 'done') {
        message.success(`${info.file.name} 上传成功`);
      } else if (info.file.status === 'error') {
        message.error(`${info.file.name} 上传失败`);
      }
    },
  };

  const downLoad = () =>{
    const ws = new WebSocket(_SOCKETLINK);
    ws.binaryType = "arraybuffer";
    ws.onopen = () => {
        console.log("连接成功，准备下载");
        ws.send(
            JSON.stringify({
                get : {
                  tableName : props.data.selectedTableName
                }
            })
        );
    }
    ws.onerror = () =>{
        console.log("连接渲染服务器出错！");
        message.error("连接渲染服务器出错！");
    }
    ws.onmessage = (msg) => {
        const { data } = msg;
        var reader = new FileReader();
        reader.onload = function(eve){
          
        }
        console.log(msg);
        try {

        } catch {
          console.log(data);
        }
    };
  }

  return (
      <div className="ls">
      <Space>
        <Tooltip
          placement="left"
          title="选择工作集合"
          arrowPointAtCenter
          color="blue"
          >
          <Select
            defaultValue="请选择工作集合"
            value={props.data.selectedTableName}
            style={{ width: 200 }}
            size="large"
            options={props.data.tableList}
            onChange={props.changeTable}></Select>
      </Tooltip>
      <Upload {...UPLOAD}>
      <Tooltip
          placement="bottom"
          title="上传SWC文件"
          arrowPointAtCenter
          color="blue"
          >
        <Button icon={<UploadOutlined />} size="large" style={{ marginLeft: 10 }} >上传</Button>
        </Tooltip>
    </Upload>
      <Tooltip
            placement="bottom"
            title="导出SWC文件"
            arrowPointAtCenter
            color="blue"
            >
        <Button icon={<DownloadOutlined />} size="large" onClick={()=>downLoad()}>下载</Button>
        </Tooltip>
          </Space>
      </div>
  );
}

export default LoadAndSave;
