#ifndef BOUNDARYDETECTOR_H
#define BOUNDARYDETECTOR_H

#include <algorithm>
#include <memory>
#include <vector>

#include <unordered_map>
#include <unordered_set>

#include "triangle.h"
#include "quad.h"

template <typename T>
class BoundaryDetector
{
public:

    std::vector <Edge <T>> detect(std::vector <Triangle <T>>& triangles)
    {
        std::vector <Edge <T>> prepared;

        for (const auto& triangle : triangles){
            auto edges = triangle.edges();
            for (const auto& edge : edges){
                prepared.push_back(edge);
            }
        }

        detectBoundary(prepared);

        return mBoundary;
    }

    std::vector <Edge <T>> detect(std::vector <Quad <T>>& quads)
    {
        std::vector <Edge <T>> prepared;

        for (const auto& quad : quads){
            auto edges = quad.edges();
            for (const auto& edge : edges){
                prepared.push_back(edge);
            }
        }

        detectBoundary(prepared);

        return mBoundary;
    }

    static std::vector <Edge <T>> simpleTinBoundary(std::shared_ptr <std::vector <Triangle <T>>> triangles)
    {
        std::vector <std::pair <Edge<T>,int>> edges;
        std::vector <Edge <T>> result;

        for (const auto& t : *triangles){
            auto edge_1 = std::pair <Edge<double>,int>(t.AB(),0);
            auto edge_2 = std::pair <Edge<double>,int>(t.BC(),0);
            auto edge_3 = std::pair <Edge<double>,int>(t.AC(),0);
            edges.emplace_back(edge_1);
            edges.emplace_back(edge_2);
            edges.emplace_back(edge_3);
        }

        for (auto& edge_1 : edges){
            for (const auto& triangle : *triangles){
                if (triangle.sharedWithEdge(edge_1.first)){
                    edge_1.second++;
                }
            }
        }

        auto condition = [](const std::pair <Edge<T>,int>& edge){
            return edge.second != 1;
        };

        //edges.erase(std::remove_if(edges.begin(), edges.end(),condition), edges.end());

        for (const auto& edge : edges){
            if(edge.second == 1)
                result.push_back(edge.first);
        }

        return result;
    }

    static std::vector <Point3D <T>>  uniformGridBoundary(std::vector <Point3D <T>> points)
    {
        std::vector <Point3D <T>> result;

        if (points.size() < 3)
            return result;

        auto comparator = [](const Point3D <T>& p1, const Point3D <T>& p2){
            if (p1.x() < p2.x()) return true;
            else if (p1.x() == p2.x()){
                return p1.y() < p2.y();
            }else return false;
        };

        std::sort(points.begin(),points.end(), comparator);

        std::vector <std::vector <Point3D <T>>> hull;

        std::vector <Point3D <T>> column{points.front()};
        for (int i = 1; i < points.size(); i++){
            if (column.back().x() != points[i].x()){
                hull.push_back(column);
                column.clear();
            }
            column.push_back(points[i]);

            if (i == points.size()-1){
                hull.push_back(column);
            }
        }

        for (int i = 0; i < hull.size(); i++){
            //if (hull[i].first().y() != result.last().y() && ){
            //    result.append(hull[i].first());
            //}
            result.push_back(hull[i].front());
        }


        for (const auto& p : hull.back()){
            //if (p.x() != result.last().x())
                result.push_back(p);
        }

        for (int i = hull.size()-1; i > 0; i--){
            //if (hull[i].front().y() != result.back().y()){
                result.push_back(hull[i].back());
            //}
        }

        for (int i = hull.front().size()-1; i > 0; i--){
            result.push_back(hull.front()[i]);
        }

        return result;
    }




    // Alpha Shape 算法：从三角网中提取边界轮廓
    // alpha: 阈值参数，较小的值会保留更多细节，较大的值会生成更平滑的轮廓
    static std::vector<Edge<T>> alphaShapeBoundary(std::vector<Triangle<T>>& triangles, double alpha = 0.0)
    {
        std::vector<Edge<T>> allEdges;
        std::unordered_map<Edge<T>, uint64_t> edgeCount;

        // 如果没有指定 alpha，自动计算一个合理值
        if (alpha <= 0.0) {
            alpha = estimateAlpha(triangles);
        }

        // 遍历所有三角形，收集边界边候选
        for (const auto& triangle : triangles) {
            // 判断三角形是否属于 Alpha Shape（外接圆半径 <= alpha）
            double circumRadius = triangle.circle().radius();
            if (circumRadius <= alpha) {
                auto edges = triangle.edges();
                for (const auto& edge : edges) {
                    allEdges.push_back(edge);
                    edgeCount[edge]++;
                }
            }
        }

        // 提取边界边（只出现一次的边）
        std::vector<Edge<T>> boundaryEdges;
        for (const auto& pair : edgeCount) {
            if (pair.second == 1) {
                boundaryEdges.push_back(pair.first);
            }
        }

        return boundaryEdges;
    }

    // 将边列表转换为有序的点序列（多边形轮廓）
    static std::vector<Point3D<T>> edgesToPolygon(std::vector<Edge<T>>& edges)
    {
        std::vector<Point3D<T>> polygon;
        if (edges.empty()) return polygon;

        // 构建边的映射：起点 -> 终点
        std::unordered_map<Point3D<T>, Point3D<T>, Point3DHash<T>> edgeMap;
        std::unordered_set<Point3D<T>, Point3DHash<T>> allPoints;

        for (const auto& edge : edges) {
            edgeMap[edge.p1()] = edge.p2();
            allPoints.insert(edge.p1());
            allPoints.insert(edge.p2());
        }

        // 找到起点（入度为0的点）
        Point3D<T> current = edges[0].p1();
        Point3D<T> start = current;

        // 遍历构建多边形
        polygon.push_back(current);
        size_t maxIterations = edges.size() + 1;
        size_t iterations = 0;

        while (iterations < maxIterations) {
            auto it = edgeMap.find(current);
            if (it == edgeMap.end()) break;

            Point3D<T> next = it->second;
            if (next == start && polygon.size() > 2) break;

            polygon.push_back(next);
            current = next;
            iterations++;
        }

        return polygon;
    }


private:
    // 自动估计 alpha 值（基于平均三角形外接圆半径）
    static double estimateAlpha(const std::vector<Triangle<T>>& triangles)
    {
        if (triangles.empty()) return 1.0;

        double totalRadius = 0.0;
        for (const auto& triangle : triangles) {
            totalRadius += triangle.circle().radius();
        }

        double avgRadius = totalRadius / triangles.size();
        return avgRadius * 1.5;
    }

    void detectBoundary(std::vector <Edge <T>>& edges)
    {
        mBoundary.clear();

        std::unordered_map <Edge <T>, uint64_t> map;

        for (const auto& edge : edges){
            auto it = map.find(edge);
            it != map.end() ? it->second++ : map[edge] = 1;
        }

        auto it = map.begin();
        while(it != map.end()){
            if (it->second == 1)  mBoundary.push_back(it->first);
            it++;
        }

    }

    std::vector <Edge <T>> mBoundary;

    // Point3D 的哈希函数，用于 unordered_map
    template <typename U>
    struct Point3DHash {
        size_t operator()(const Point3D<T>& p) const {
            auto h1 = std::hash<T>{}(p.x());
            auto h2 = std::hash<T>{}(p.y());
            auto h3 = std::hash<T>{}(p.z());
            return h1 ^ (h2 << 1) ^ (h3 << 2);
        }
    };
};

#endif // BOUNDARYDETECTOR_H
