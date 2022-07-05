#pragma once
#include <string>
#include <vector>

class uv_location
{
public:
    float x;
    float y;

public:
    uv_location(float x_inport, float y_inport)
    {
        x = x_inport;
        y = y_inport;
    }
};

class point_uv
{
private:
    int id;
    uv_location* point_loc;

public:
    point_uv(std::string uv_line);
    uv_location* get_location(){return point_loc;}
};

class prim_uv
{
private:
    std::vector<point_uv *> uv_point;
    int id = 0;

public:
    prim_uv(int id_inport) { id = id_inport; }
    void add_point(std::string uv_line) { uv_point.push_back(new point_uv(uv_line)); }
    uv_location centor();
    std::vector<point_uv *> get_uv_point(){return uv_point;}
};

class prim_uv_factory
{
private:
    std::vector<prim_uv *> uv_prim;
    int id = 0;

public:
    prim_uv_factory(std::string uv_file);
    std::vector<prim_uv *> get_uv_prim(){return uv_prim;}
};