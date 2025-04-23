#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <pangolin/pangolin.h>
//#include <pangolin/handler/handler3d.h>
#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>

// Vertex structure
struct Vertex {
    float x, y, z;
};

// Helper to load OBJ and create buffers
struct Mesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    GLuint vbo = 0, ibo = 0;
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

        std::unordered_map<int, unsigned int> index_map;
        for (const auto& shape : shapes) {
            size_t index_offset = 0;
            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
                int fv = shape.mesh.num_face_vertices[f];
                for (int v = 0; v < fv; v++) {
                    tinyobj::index_t idx = shape.mesh.indices[index_offset + v];
                    int vi = idx.vertex_index;
                    if (index_map.count(vi) == 0) {
                        Vertex vert = {
                            attrib.vertices[3 * vi + 0],
                            attrib.vertices[3 * vi + 1],
                            attrib.vertices[3 * vi + 2]
                        };
                        vertices.push_back(vert);
                        index_map[vi] = vertices.size() - 1;
                    }
                    indices.push_back(index_map[vi]);
                }
                index_offset += fv;
            }
        }
        index_count = indices.size();
        return true;
    }

    void UploadToGPU() {
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ibo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void Draw() const {
        glEnableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glVertexPointer(3, GL_FLOAT, sizeof(Vertex), 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
        glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);
        glDisableClientState(GL_VERTEX_ARRAY);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void FreeGPU() {
        if (vbo) glDeleteBuffers(1, &vbo);
        if (ibo) glDeleteBuffers(1, &ibo);
    }
};

int main() {
    // Load two meshes
    Mesh mesh1, mesh2;
    if (!mesh1.LoadFromObj("models/model.obj")) {
        std::cerr << "Failed to load model1.obj" << std::endl;
        return 1;
    }
    if (!mesh2.LoadFromObj("models/al.obj")) {
        std::cerr << "Failed to load model2.obj" << std::endl;
        return 1;
    }

    // Pangolin window setup
    const int width = 800, height = 600;
    pangolin::CreateWindowAndBind("Two OBJ Viewer", width, height);
    glEnable(GL_DEPTH_TEST);

    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(width, height, 500, 500, width/2, height/2, 0.1, 1000),
        pangolin::ModelViewLookAt(15, 15, 15, 0, 0, 0, pangolin::AxisY)
    );
    pangolin::Handler3D handler(s_cam);
    pangolin::View& d_cam = pangolin::CreateDisplay()
        .SetBounds(0.0, 1.0, 0.0, 1.0, -width/(float)height)
        .SetHandler(&handler);

    // Upload meshes to GPU
    mesh1.UploadToGPU();
    mesh2.UploadToGPU();

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

    // Main loop
    while (!pangolin::ShouldQuit()) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        d_cam.Activate(s_cam);

        // Draw mesh1 at (-3,0,0)
        glPushMatrix();
        glTranslatef(-10.0f, 0.0f, 0.0f);
        mesh1.Draw();
        glPopMatrix();

        // Draw mesh2 at (3,0,0)
        glPushMatrix();
        glTranslatef(10.0f, 0.0f, 0.0f);
        mesh2.Draw();
        glPopMatrix();

        pangolin::glDrawAxis(20.0);
        pangolin::FinishFrame();
    }

    mesh1.FreeGPU();
    mesh2.FreeGPU();

    return 0;
}
