#include "Object.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <string>

// Utility: Load file as string
std::string LoadFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

Object::Object() : index_count(0) {}

bool Object::LoadFromObj(const std::string& filename) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filename.c_str());
    if (!warn.empty()) std::cout << "WARN: " << warn << std::endl;
    if (!err.empty()) std::cerr << "ERR: " << err << std::endl;
    if (!ret) return false;

    struct IndexHash {
        size_t operator()(const std::pair<int, int>& p) const {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        }
    };
    std::unordered_map<std::pair<int, int>, unsigned int, IndexHash> index_map;
    for (const auto& shape : shapes) {
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            int fv = shape.mesh.num_face_vertices[f];
            for (int v = 0; v < fv; v++) {
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                int vi = idx.vertex_index;
                int ni = idx.normal_index;
                std::pair<int, int> key = {vi, ni};
                if (index_map.count(key) == 0) {
                    Vertex vert;
                    vert.x = attrib.vertices[3 * vi + 0];
                    vert.y = attrib.vertices[3 * vi + 1];
                    vert.z = attrib.vertices[3 * vi + 2];
                    if (ni >= 0) {
                        vert.nx = attrib.normals[3 * ni + 0];
                        vert.ny = attrib.normals[3 * ni + 1];
                        vert.nz = attrib.normals[3 * ni + 2];
                    } else {
                        vert.nx = vert.ny = vert.nz = 0.0f;
                    }
                    vertices.push_back(vert);
                    index_map[key] = vertices.size() - 1;
                }
                indices.push_back(index_map[key]);
            }
            index_offset += fv;
        }
    }
    index_count = indices.size();
    return true;
}

void Object::UploadToGPU() {
    vbo.Reinitialise(
        pangolin::GlArrayBuffer,
        vertices.size(),
        GL_FLOAT,
        6, // x, y, z, nx, ny, nz
        GL_STATIC_DRAW,
        vertices.data()
    );
    ibo.Reinitialise(
        pangolin::GlElementArrayBuffer,
        indices.size(),
        GL_UNSIGNED_INT,
        1,
        GL_STATIC_DRAW,
        indices.data()
    );
}

void Object::Draw() const {
    vbo.Bind();
    ibo.Bind();
    glEnableVertexAttribArray(0); // position
    glEnableVertexAttribArray(1); // normal
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    vbo.Unbind();
    ibo.Unbind();
}
