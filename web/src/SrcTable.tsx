import React, { useEffect, useRef, useState, useContext } from 'react';
import { Table, Input, Button, Popconfirm, Form, Space, message } from 'antd';
import { EditOutlined, SearchOutlined, EyeInvisibleOutlined, EyeOutlined, FileAddOutlined, MenuOutlined } from '@ant-design/icons';
import { SortableContainer, SortableElement, SortableHandle } from 'react-sortable-hoc';
import arrayMove from 'array-move';

import Highlighter from 'react-highlight-words';
import InputColor, { Color } from 'react-input-color';
import { FormInstance } from 'antd/lib/form';

const EditableContext = React.createContext<FormInstance<any> | null>(null);
const _SOCKETLINK = "ws://127.0.0.1:12121/render";

interface Item {
  key: string;
  name: string;
  color: string;
  status: boolean;   
}

interface EditableRowProps {
  index: number;
}

const EditableRow: React.FC<EditableRowProps> = ({ index, ...props }) => {
    const [form] = Form.useForm();
    return (
      <Form form={form} component={false}>
        <EditableContext.Provider value={form}>
          <tr {...props} />
        </EditableContext.Provider>
      </Form>
    );
  };

interface EditableCellProps {
    title: React.ReactNode;
    editable: boolean;
    children: React.ReactNode;
    dataIndex: keyof Item;
    record: Item;
    handleSave: (record: Item) => void;
}

const EditableCell: React.FC<EditableCellProps> = ({
    title,
    editable,
    children,
    dataIndex,
    record,
    handleSave,
    ...restProps
  }) => {
    const [editing, setEditing] = useState(false);
    const inputRef = useRef<Input>(null);
    const form = useContext(EditableContext)!;
  
    useEffect(() => {
      if (editing) {
        inputRef.current!.focus();
      }
    }, [editing]);
  
    const toggleEdit = () => {
      setEditing(!editing);
      form.setFieldsValue({ [dataIndex]: record[dataIndex] });
    };
  
    const save = async () => {
      try {
        const values = await form.validateFields();
  
        toggleEdit();
        handleSave({ ...record, ...values });
      } catch (errInfo) {
        console.log('Save failed:', errInfo);
      }
    };
  
    let childNode = children;
  
    if (editable) {
      childNode = editing ? (
        <Form.Item
          style={{ margin: 0 }}
          name={dataIndex}
          rules={[
            {
              required: true,
              message: `${title} 不能为空`,
            },
          ]}
        >
          <Input ref={inputRef} onPressEnter={save} onBlur={save} />
        </Form.Item>
      ) : (
        <div className="editable-cell-value-wrap" style={{ paddingRight: 24 }} onClick={toggleEdit}>
          {children}
        </div>
      );
    }
  
    return <td {...restProps}>{childNode}</td>;
  };
  
  type EditableTableProps = Parameters<typeof Table>[0];
  
  interface DataType {
    key: React.Key;
    name: String;
    color: String;
    status: Boolean;
    index: number;
    sub: Array<SubDataType>;
  }
  
  interface SubDataType {
    key: React.Key;
    index: number;
    name: String;
    length: String;
    lastEditTime: String;
  }

  interface EditableTableState {
    popVisible: Boolean;
    searchText: string;
    searchedColumn: string;
    selectedRowKeys: React.Key[];
    //dataSource: DataType[];
    count: number;
  }
  
type ColumnTypes = Exclude<EditableTableProps['columns'], undefined>;

class SrcTable extends React.Component<EditableTableProps, EditableTableState>{
    columns: (ColumnTypes[number] & { editable?: boolean; dataIndex: string })[];
    constructor(props:EditableTableProps){
        super(props);
        this.columns = [
            {
              title: '名称',
              dataIndex: 'name',
              key: 'name',
              editable: true,
            },
            {
              title: '颜色',
              dataIndex: 'color',
              key: 'color',
              render: (c,row) => (
                <div>
                <InputColor
                  initialValue={c}
                  onChange={(color:Color)=>this.setColor(color,row)}
                />
                </div>
              )
            },
            {
                title: '状态',
                dataIndex: 'status',
                key: 'status',
                render: (st,row) => (
                    <div>
                        <a onClick={()=>this.changeStatus(st,row)}>
                            {st ? (<EyeOutlined/>):(<EyeInvisibleOutlined/>)}
                        </a>
                    </div>
                )
            },
            {
              title: '操作',
              key: 'action',
              dataIndex: 'action',
              render: (_, record) => (
                  <div>
                    <Space size="middle">
                        <Popconfirm title="确定删除吗？" onConfirm={() => this.handleDelete(record)}>
                            <Button danger type="primary" size="small" onClick={()=>this.showPopconfirm()}>删除</Button>
                        </Popconfirm>
                    </Space>
                </div>               
              ),
            },
          ];
        this.state = {
            popVisible: false,
            searchText: '',
            selectedRowKeys: [],
            searchedColumn: '',
            count: this.props.data ? this.props.data.graphs.length : 0,
        } 
    }

    getColumnSearchProps = (dataIndex: string) => ({
      filterDropdown: ({ setSelectedKeys, selectedKeys, confirm, clearFilters }) => (
        <div style={{ padding: 8 }}>
          <Input
            ref={node => {
              this.searchInput = node;
            }}
            placeholder={`Search ${dataIndex}`}
            value={selectedKeys[0]}
            onChange={e => setSelectedKeys(e.target.value ? [e.target.value] : [])}
            onPressEnter={() => this.handleSearch(selectedKeys, confirm, dataIndex)}
            style={{ width: 188, marginBottom: 8, display: 'block' }}
          />
          <Space>
            <Button
              type="primary"
              onClick={() => this.handleSearch(selectedKeys, confirm, dataIndex)}
              icon={<SearchOutlined />}
              size="small"
              style={{ width: 90 }}
            >
              Search
            </Button>
            <Button onClick={() => this.handleReset(clearFilters)} size="small" style={{ width: 90 }}>
              Reset
            </Button>
            <Button
              type="link"
              size="small"
              onClick={() => {
                confirm({ closeDropdown: false });
                this.setState({
                  searchText: selectedKeys[0],
                  searchedColumn: dataIndex,
                });
              }}
            >
              Filter
            </Button>
          </Space>
        </div>
      ),
      filterIcon: (filtered: any) => <SearchOutlined style={{ color: filtered ? '#1890ff' : undefined }} />,
      onFilter: (value: string, record: { [x: string]: { toString: () => string; }; }) =>
        record[dataIndex]
          ? record[dataIndex].toString().toLowerCase().includes(value.toLowerCase())
          : '',
      onFilterDropdownVisibleChange: (visible: any) => {
        if (visible) {
          setTimeout(() => this.searchInput.select(), 100);
        }
      },
      render: (text: { toString: () => string; }) =>
        this.state.searchedColumn === dataIndex ? (
          <Highlighter
            highlightStyle={{ backgroundColor: '#ffc069', padding: 0 }}
            searchWords={[this.state.searchText]}
            autoEscape
            textToHighlight={text ? text.toString() : ''}
          />
        ) : (
          text
        ),
    });
  
    handleSearch = (selectedKeys: any[], confirm: () => void, dataIndex: any) => {
      confirm();
      this.setState({
        searchText: selectedKeys[0],
        searchedColumn: dataIndex,
      });
    };
  
    handleReset = (clearFilters: () => void) => {
      clearFilters();
      this.setState({ searchText: '' });
    };  

    handleDelete = (record) => {
        //服务器自删除后更新数据，并提示删除成功
        const ws = new WebSocket("ws://192.168.1.26:9999");
            ws.binaryType = "arraybuffer";
            ws.onopen = () => {
                console.log("连接成功，准备发送更新数据");
                ws.send(
                    JSON.stringify({
                        type: "deleteMap",
                        index: record.index
                    })
                );
            }
    };

    handleAdd = () => {
        const ws = new WebSocket(_SOCKETLINK);
        ws.onopen = () => {
            console.log("连接成功，添加路径");
            ws.send(
                JSON.stringify({
                    type: "addMap"
                })
            );
            const hide = message.loading('正在添加...', 0);
            setTimeout(hide, 500);
        }
      };

    setColor = (color:Color,row:DataType) =>{
        const newData = [...this.props.data.graphs];
        const index = newData.findIndex(item => row.key === item.key);
        const item = newData[index];
        if(item.color == color.hex) return;
        
        item.color = color.hex;
        const ws = new WebSocket(_SOCKETLINK);
        ws.onopen = () => {
            console.log("连接成功，准备发送更新数据");
            ws.send(
                JSON.stringify({
                    type: "modifyMap",
                    map: item
                })
            );
            const hide = message.loading('正在修改...', 0);
            setTimeout(hide, 500);
        }
        ws.onerror = () =>{
          console.log("连接渲染服务器出错！");
          message.error("连接渲染服务器出错！");
        } 
    }

    showPopconfirm = () =>{
        this.setState({
            popVisible: true,
        })
    }

    changeStatus = ( st:boolean,row:DataType ) =>{
        console.log("修改可见")
        //const newData = [...this.state.dataSource];
        const newData = [...this.props.data.graphs];
        const index = newData.findIndex(item => row.key === item.key);
        const item = newData[index];
        item.status = !item.status;
        newData.splice(index, 1, {
        ...item,
        ...row,
        });
        this.props.setData({ graphs: newData });
        //this.setState({ dataSource: newData });
    };

    check = (record:DataType) => {
        console.log(record)
    }

    handleSave = (row:DataType) => {
        const newData = [...this.props.data.graphs];
        const index = newData.findIndex(item => row.key === item.key);
        const item = newData[index];
        // newData.splice(index, 1, {
        // ...item,
        // ...row,
        // });
        // this.props.setData({ graphs: newData });
        if( item.name == row.name ) return;

        const ws = new WebSocket(_SOCKETLINK);
        ws.onopen = () => {
            console.log("连接成功，准备发送更新数据");
            ws.send(
                JSON.stringify({
                    type: "modifyMap",
                    map: row
                })
            );
            const hide = message.loading('正在修改...', 0);
            setTimeout(hide, 1000);
        }
    };

    onClickJump = (record) =>{
        console.log(record);
        this.props.onClickJumpToVex(record);
    }

    render(){
        let self = this;
        var dataSource;
        if ( self.props.data && self.props.data.graphs ) {
            dataSource = [...self.props.data.graphs];
        }else{
            dataSource = [];
        }
        const expandedRowRender = (data: { sub: readonly any[] | undefined; }) => {
          const columns = [
            { title: '', dataIndex: 'name', key: 'name' },
            { title: '最后编辑时间', dataIndex: 'lastEditTime', key: 'lastEditTime'},
            { title: '操作', key:'action', dataIndex: 'action',
            render: ( _, record)=>(
              //如何传入row TODO
              <div>
                <Button onClick={()=>self.onClickJump(record)}>跳转</Button>
              </div>
            )
          }
          ];
          return <Table columns={columns} dataSource={data.sub} pagination={false} />;
        };

        const components = {
            body: {
              row: EditableRow,
              cell: EditableCell,
            },
          };
          const columns = self.columns.map(col => {
            if (!col.editable) {
              return col;
            }
            return {
              ...col,
              ...self.getColumnSearchProps('name'), //search
              onCell: (record: DataType) => ({
                record,
                editable: col.editable,
                dataIndex: col.dataIndex,
                title: col.title,
                handleSave: self.handleSave,
              }),
            };
          });
        return(
        <div style={{margin:'0 8px'}}>
            <Button onClick={()=>self.handleAdd()} type="primary" style={{ marginBottom: 16 }} ><FileAddOutlined />添加新路径</Button>
            <Table components={components}
            rowSelection={{type:"radio",...this.props.rowSelection}}
            rowClassName={() => 'editable-row'}
            rowKey="index"
            columns={columns as ColumnTypes}
            dataSource={dataSource} 
            expandable={{ expandedRowRender }}
            defaultPageSize="10"
            showHeader={true}
            size='small'
            style={{border:10}}
            />
        </div>
        )
    };
};

export default SrcTable;
