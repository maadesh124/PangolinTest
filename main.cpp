#include "Object.h"
#include "shaders.h"

int main() {
    const int width = 900, height = 700;
    pangolin::CreateWindowAndBind("Two OBJ Viewer", width, height);
    glEnable(GL_DEPTH_TEST);

    // Load two objects
    Object obj1, obj2;
    if (!obj1.LoadFromObj("models/model.obj")) {
        std::cerr << "Failed to load model.obj" << std::endl;
        return -1;
    }
    if (!obj2.LoadFromObj("models/al.obj")) {
        std::cerr << "Failed to load al.obj" << std::endl;
        return -1;
    }
    obj1.UploadToGPU();
    obj2.UploadToGPU();


    pangolin::GlSlProgram shader;
    shader.AddShader(pangolin::GlSlVertexShader, vertex_shader);
    shader.AddShader(pangolin::GlSlFragmentShader, fragment_shader);
    shader.Link();

    // Camera: position at (25,0,0), look at (0,0,0), up is Y
    pangolin::OpenGlRenderState s_cam(
        pangolin::ProjectionMatrix(width, height, 500, 500, width/2, height/2, 0.1, 1000),
        pangolin::ModelViewLookAt(25, 0, 0, 0, 0, 0, pangolin::AxisY)
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

        pangolin::OpenGlMatrix view = s_cam.GetModelViewMatrix();
        pangolin::OpenGlMatrix proj = s_cam.GetProjectionMatrix();

        // Draw obj1 at (-15, 0, 0)
        pangolin::OpenGlMatrix model1;
        model1.SetIdentity();
        model1.m[12] = -15.0;
        model1.m[13] = 0.0;
        model1.m[14] = 0.0;
        shader.SetUniform("model", model1);
        shader.SetUniform("view", view);
        shader.SetUniform("projection", proj);
        obj1.Draw();

        // Draw obj2 at (15, 0, 0)
        pangolin::OpenGlMatrix model2;
        model2.SetIdentity();
        model2.m[12] = 15.0;
        model2.m[13] = 0.0;
        model2.m[14] = 0.0;
        shader.SetUniform("model", model2);
        shader.SetUniform("view", view);
        shader.SetUniform("projection", proj);
        obj2.Draw();

        shader.Unbind();

        pangolin::glDrawAxis(1.0);
        pangolin::FinishFrame();
    }
    return 0;
}