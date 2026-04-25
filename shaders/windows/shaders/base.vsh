#version 140

in vec3 position;       //输入顶点位置
uniform mat4 matrix;    //变换矩阵（通常是MVP矩阵）
uniform float width;    //点的大小
out vec3 vertice;       //传递给片段着色器的顶点位置

void main()
{
    vertice = position;     //将原始位置传递给片段着色器
    gl_PointSize = width;   //设置点的渲染大小
    gl_Position = matrix * vec4(position, 1.0);   //应用变换矩阵
}
