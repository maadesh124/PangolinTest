#ifndef OBJECT_H
#define OBJECT_H

#include <pangolin/pangolin.h>
#include <pangolin/gl/glsl.h>
#include <vector>
#include <string>



std::string LoadFile(const std::string& path);
// Object class: loads and renders a mesh from OBJ
class Object {
public:
    Object();

    Eigen::Vector3f pos;//relative position of obj with respect to anchor
    Eigen::Matrix3f ori;//relative orientation of obj with respect to anchor


    // Load mesh from OBJ file
    bool LoadFromObj(const std::string& filename);

    // Upload mesh data to GPU
    void UploadToGPU();

    // Render the mesh
    void Draw() const;

private:
    struct Vertex {
        float x, y, z;
        float nx, ny, nz;
    };

    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    pangolin::GlBuffer vbo, ibo;
    size_t index_count;


};

#endif