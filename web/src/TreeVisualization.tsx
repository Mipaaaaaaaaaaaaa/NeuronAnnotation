import {Tabs, Radio, Space, Tooltip} from 'antd';
import React from "react";
import * as lw from "@euphrasiologist/lwphylo";
import * as d3 from "d3";

const {TabPane} = Tabs;
const w = 1024;
const h = 200;
const scaleRect = 15;
const xScaleRect = d3
    .scaleLinear()
    .domain([
        -scaleRect * 0.03,
        scaleRect * 0.5
    ])
    .range([0, w]);
const yScaleRect = d3
    .scaleLinear()
    .domain([
        -scaleRect * 0.1,
        scaleRect * 0.5
    ])
    .range([h,0]);
const scaleUnroot = 3.5;
const xScaleUnroot = d3
    .scaleLinear()
    .domain([
        -scaleUnroot,
        scaleUnroot
    ])
    .range([0, w]);
const yScaleUnroot = d3
    .scaleLinear()
    .domain([
        -scaleUnroot,
        scaleUnroot
    ])
    .range([h, 0]);

//数据部分
interface DataType {
    key: number;
    name: String;
    color: String;
    status: Boolean;
    index: number;
    sub: Array<SubDataType>;
}
    
interface SubDataType {
    key: number;
    index: number;
    name: String;
    arc: Array<ArcType>;
    lastEditTime: String;
}

interface ArcType{
    tailVex: number;
    headVex: number;
    distance: number;
}

const GraphToNewick = ( graph:DataType, pkey:number ) =>{
    console.log(graph);
    var maxLength = 0;
    for( let i = 0 ; i < graph.sub.length ; i ++ ){
        for( let j = 0 ; j < graph.sub[i].arc.length ; j ++ ){
            if( maxLength < graph.sub[i].arc[j].distance ){
                maxLength = graph.sub[i].arc[j].distance; //找到最大值，权重为1
            }
        }
    }
    let dicMap = new Map(); //index和key转换
    var visitedArray = new Array(graph.sub.length);
    for( let i = 0; i < graph.sub.length ; i ++ ){
        visitedArray[i] = false;
        dicMap.set(graph.sub[i].index,i); //服务器的index,对应客户端的key
    }
    const dfs = (graph:DataType,key:number,length:number) =>{
        let parsedStr = "";
        console.log("visite:",key);
        visitedArray[key] = true;
        for( let i = 0 ; i < graph.sub[key].arc.length ; i ++ ){
            if( graph.sub[key].arc[i].headVex == graph.sub[key].index ){
                let vexKey = dicMap.get(graph.sub[key].arc[i].tailVex);
                if( !visitedArray[vexKey] ){
                    let subStr = dfs(graph,vexKey,graph.sub[key].arc[i].distance);
                    if( parsedStr.charAt(parsedStr.length - 1) >= '0' && parsedStr.charAt(parsedStr.length - 1) <= '9' ) 
                        parsedStr = parsedStr + "," + subStr;
                    else{
                        parsedStr = parsedStr + subStr;
                    }
                }
            } else if( graph.sub[key].arc[i].tailVex == graph.sub[key].index ){
                let vexKey = dicMap.get(graph.sub[key].arc[i].headVex);
                if( !visitedArray[vexKey] ){ 
                    let subStr = dfs(graph,vexKey,graph.sub[key].arc[i].distance);
                    if( parsedStr.charAt(parsedStr.length - 1) >= '0' && parsedStr.charAt(parsedStr.length - 1) <= '9' ) 
                        parsedStr = parsedStr + "," + subStr;
                    else{
                        parsedStr = parsedStr + subStr;
                    }
                }
            }
        }
        if( parsedStr == "" ){
            return key + ":" + length/maxLength;
        }
        if( length == 0 ){ //最后一个顶点
            return "(" + parsedStr + ")" ;
        }
        else {
            return "(" + parsedStr + ")" + key + ":" + length/maxLength;
        }
    }
    console.log(visitedArray);
    let x = dfs(graph,pkey,0);
    console.log(x);
    return x;
}



class TreeVisualization extends React.Component {
    constructor(props) {
        super(props);
        this.state = {
            nowPage : "1"
        };
    }

    componentDidUpdate(){
        console.log(this.props);
        if(this.props.data.graphs[this.props.selectedMapKey].sub && this.props.data.graphs[this.props.selectedMapKey].sub[this.props.selectedVertexKey].arc){
            if (this.state.nowPage == '1') {
                this.RectPhyloPlot();
            } else if (this.state.nowPage == '3') {
                this.UnrootedPhyloPlot();
            }
        }
    }

    componentDidMount() {
        if(this.props.data.graphs[this.props.selectedMapKey].sub[this.props.selectedVertexKey]){
            this.RectPhyloPlot();
            this.UnrootedPhyloPlot();
        }
    }

    RectPhyloPlot = () => {
        const treeString = GraphToNewick(this.props.data.graphs[this.props.selectedMapKey],this.props.selectedVertexKey);
        const parsedTree = lw.readTree(treeString);
        const rectPhylo = lw.rectangleLayout(parsedTree);
        const self = this;

        d3
        .select("#Rectangle")
        .select("svg")
        .selectAll("*")
        .remove()
    
        const svg = d3
            .select("#Rectangle")
            .select("svg")
            .attr("width", w)
            .attr("height", h)
            .attr("font-family", "sans-serif")
            .attr("font-size", 10);

        // create a grouping variable
        const group = svg.append('g');

        const stroke_width = 3;
        // draw horizontal lines
        group
            .append('g')
            .attr('class', 'phylo_lines')
            .selectAll('lines')
            .data(rectPhylo.data)
            .join('line')
            .attr('class', 'lines')
            .attr('x1', d => xScaleRect(d.x0) - stroke_width / 2)
            .attr('y1', d => yScaleRect(d.y0))
            .attr('x2', d => xScaleRect(d.x1) - stroke_width / 2)
            .attr('y2', d => yScaleRect(d.y1))
            .attr('stroke-width', stroke_width)
            .attr('stroke', this.props.data.graphs[this.props.selectedMapKey].color); //线段上色

        // draw vertical lines
        group
            .append('g')
            .attr('class', 'phylo_lines')
            .selectAll('lines')
            .data(rectPhylo.vertical_lines)
            .join('line')
            .attr('class', 'lines')
            .attr('x1', d => xScaleRect(d.x0))
            .attr('y1', d => yScaleRect(d.y0))
            .attr('x2', d => xScaleRect(d.x1))
            .attr('y2', d => yScaleRect(d.y1))
            .attr('stroke-width', stroke_width)
            .attr('stroke', this.props.data.graphs[this.props.selectedMapKey].color); //线段上色

        // draw nodes
        group
            .append('g')
            .attr('class', 'phylo_points')
            .selectAll('.dot')
            // remove rogue dot.
            .data(rectPhylo.data.filter(d => d.x1 > 0))
            .join('circle')
            .attr('class', 'dot')
            .attr('r', function (d) {
                if (d.thisLabel == self.props.selectedVertexKey) return 6;
                else return 4;
            })
            .attr('cx', d => xScaleRect(d.x1))
            .attr('cy', d => yScaleRect(d.y1))
            .attr('stroke', function(d){
              if (d.thisLabel == self.props.selectedVertexKey)
                return 'red';
              else return 'black';
            })
            .attr('stroke-width', function (d) {
                if (d.thisLabel == self.props.selectedVertexKey) return 3;
                if (d.isTip) {
                    return 2;
                } else {
                    return 1;
                }
            })
            .attr('fill', 'white');

        group.selectAll('.dot')
            .on("mouseover", (d,i) => {
                console.log(d)
            d3.select("#tooltip").remove();
            d3.select("#Rectangle")
                .select("svg")
                .append("text")
                .attr("x", d.layerX)
                .attr("y", d.layerY-18)
                .attr("id", "tooltip")
                .attr("class","tooltip")
                .attr("text-anchor", "middle")
                .attr("font-size", "13px")
                .text( function(){
                    let num = i.thisLabel ? i.thisLabel : 0;
                    return "跳转至" + num + "号点";
                });
        })
        .on("click",(d,i)=>{
            let record = {
                index:self.props.data.graphs[self.props.selectedMapKey].sub[i.thisLabel].index,
                key:i.thisLabel
            };
            self.props.onClickJumpToVex(record);
        })
        .on("mouseout", (d) => {
            d3.select("#tooltip").remove();
        })

    }

    UnrootedPhyloPlot = () => {
        const treeString = GraphToNewick(this.props.data.graphs[this.props.selectedMapKey],this.props.selectedVertexKey);
        const parsedTree = lw.readTree(treeString);
        const unrootedPhylo = lw.unrooted(parsedTree);
        const self = this;
        d3
            .select("#EqualAngle")
            .select("svg")
            .selectAll("*")
            .remove()
        
        const svg = d3
            .select("#EqualAngle")
            .select("svg")
            .attr("width", w)
            .attr("height", h)
            .attr("font-family", "sans-serif")
            .attr("font-size", 10);

        var group = svg.append("g");

        // draw lines
        group
            .append("g")
            .attr("class", "phylo_lines")
            .selectAll("lines")
            .data(unrootedPhylo.edges)
            .enter()
            .append("line")
            .attr("class", "lines")
            .attr("x1", d => xScaleUnroot(d.x1))
            .attr("y1", d => yScaleUnroot(d.y1))
            .attr("x2", d => xScaleUnroot(d.x2))
            .attr("y2", d => yScaleUnroot(d.y2))
            .attr("stroke-width", 3)
            .attr("stroke", self.props.data.graphs[self.props.selectedMapKey].color);

        // draw points
        group
            .append("g")
            .attr("class", "phylo_points")
            .selectAll(".dot")
            .data(unrootedPhylo.data)
            .enter()
            .append("circle")
            .attr("class", "dot")
            .attr("r", function (d) {
              if (d.thisLabel == self.props.selectedVertexKey) return 6;
              else return 4;
            })
            .attr("cx", d => xScaleUnroot(d.x))
            .attr("cy", d => yScaleUnroot(d.y))
            .attr('stroke', function(d){
              if (d.thisLabel == self.props.selectedVertexKey)
                return 'red';
              else return 'black';
            })
            .attr('stroke-width', function (d) {
                if (d.thisLabel == self.props.selectedVertexKey) return 3;
                if (d.isTip) {
                    return 2;
                } else {
                    return 1;
                }
            })
            .attr('fill', 'white')
            
        group.selectAll('.dot')
            .on("mouseover", (d,i) => {
                console.log(d)
            d3.select("#tooltip").remove();
            d3.select("#EqualAngle")
                .select("svg")
                .append("text")
                .attr("x", d.layerX)
                .attr("y", d.layerY-18)
                .attr("id", "tooltip")
                .attr("class","tooltip")
                .attr("text-anchor", "middle")
                .attr("font-size", "13px")
                .text( function(){
                    let num = i.thisLabel ? i.thisLabel : 0;
                    return "跳转至" + num + "号点";
                });
        })
        .on("click",(d,i)=>{
            let record = {
                index:self.props.data.graphs[self.props.selectedMapKey].sub[i.thisLabel?i.thisLabel:0].index,
                key:i.thisLabel ? i.thisLabel : 0
            };
            self.props.onClickJumpToVex(record);
        })
        .on("mouseout", (d) => {
            d3.select("#tooltip").remove();
        })

    }

    onChange = (activeKey : String) => {
        this.setState({
            nowPage : activeKey
        });
        if (activeKey == '1') {
            console.log("Rect")
            this.RectPhyloPlot();
        } else if (activeKey == '3') {
            console.log("UnrootedPhyloPlot")
            this.UnrootedPhyloPlot();
        }
    }

    render() {

        return (
            <> < Tabs tabPosition = "right" onChange = {
                (activeKey) => this.onChange(activeKey)
            }
            type = {
                "card"
            }
            animated = {{ inkBar: true, tabPane: true }} > <TabPane
                tab="Rectangle"
                key="1"
                forceRender={true}
                style={{
                    overflowY: 'auto',
                    overflowX: 'auto'
                }}>
                <div className="Rectangle" id="Rectangle" ref="Rectangle">
                    <svg></svg>
                </div>
            </TabPane>
            <TabPane
                tab="EqualAngle"
                key="3"
                forceRender={true}
                style={{
                    overflowY: 'auto',
                    overflowX: 'auto'
                }}>
                <div className="EqualAngle" id="EqualAngle" ref="EqualAngle">
                    <svg></svg>
                </div>
            </TabPane>
        </Tabs>
    </>
        );
    }
}

export default TreeVisualization;