#include <pangolin/pangolin.h>
#include <pangolin/gl/glsl.h>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>

// Utility: Load file as string
std::string LoadFile(const std::string& path) {
    std::ifstream file(path);
    std::stringstream ss;
    ss << file.rdbuf();
    return ss.str();
}

// Vertex struct with normals
struct Vertex {
    float x, y, z;
    float nx, ny, nz;
};

struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    pangolin::GlBuffer vbo, ibo;
    size_t index_count = 0;

    bool LoadFromObj(const std::string& filename) {
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

void UploadToGPU() {
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


    void Draw() const {
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
};

int main() {
    // Pangolin window/context
    const int width = 900, height = 700;
    pangolin::CreateWindowAndBind("Modern OpenGL OBJ Viewer (No GLAD)", width, height);
    glEnable(GL_DEPTH_TEST);

    // Load mesh
    Mesh mesh;
    if (!mesh.LoadFromObj("models/model.obj")) {
        std::cerr << "Failed to load OBJ file!" << std::endl;
        return -1;
    }
    mesh.UploadToGPU();

    // Load shaders
    std::string vsrc = LoadFile("../vertex_shader.glsl");
    std::string fsrc = LoadFile("../fragment_shader.glsl");
    pangolin::GlSlProgram shader;
    shader.AddShader(pangolin::GlSlVertexShader, vsrc);
    shader.AddShader(pangolin::GlSlFragmentShader, fsrc);
    shader.Link();

    // Camera
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(width, height, 500, 500, width/2, height/2, 0.1, 1000),
        pangolin::ModelViewLookAt(0, 2, 6, 0, 0, 0, pangolin::AxisY)
    );
    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
        .SetBounds(0.0, 1.0, 0.0, 1.0, -width/(float)height)
        .SetHandler(&handler);

    // Main loop
    while (!pangolin::ShouldQuit()) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);

        shader.Bind();

        // Model, view, projection
        pangolin::OpenGlMatrix model = pangolin::IdentityMatrix();
        pangolin::OpenGlMatrix view = s_cam.GetModelViewMatrix();
        pangolin::OpenGlMatrix proj = s_cam.GetProjectionMatrix();

        shader.SetUniform("model", model);
        shader.SetUniform("view", view);
        shader.SetUniform("projection", proj);

        mesh.Draw();

        shader.Unbind();

        pangolin::glDrawAxis(1.0);
        pangolin::FinishFrame();
    }
    return 0;
}
