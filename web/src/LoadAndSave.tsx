import React from "react";
import { Space, Typography, Divider } from 'antd';

const LoadAndSave: React.FC = () => {
  return (
    <Space split={<Divider type="vertical" />}>
      <Typography.Link href="#" target="_blank">载入</Typography.Link>
      <Typography.Link href="#" target="_blank">保存</Typography.Link>
    </Space>
  );
}

export default LoadAndSave;
