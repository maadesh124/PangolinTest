#include <pangolin/pangolin.h>
#include <opencv2/opencv.hpp>
#include "Object.h"
#include "shaders.h"

using namespace std;

void DrawImageTexture(pangolin::GlTexture &imageTexture, cv::Mat &im) {
    if(!im.empty()) {
        imageTexture.Upload(im.data, GL_RGB, GL_UNSIGNED_BYTE);
        imageTexture.RenderToViewportFlipY();
    }
}

int main() {
    // Load image and convert to RGB
    cv::Mat image = cv::imread("im1.jpg");
    cv::Mat image_rgb;
    cv::cvtColor(image, image_rgb, cv::COLOR_BGR2RGB);
    
    const int width = image.cols;
    const int height = image.rows;
    cout << "Image size: " << width << "x" << height << endl;

    // Create Pangolin window
    pangolin::CreateWindowAndBind("3D Overlay Demo", width, height);
    glEnable(GL_DEPTH_TEST);

    // Setup texture for background image
    pangolin::GlTexture imageTexture(width, height, GL_RGB, false, 0, GL_RGB, GL_UNSIGNED_BYTE);

    // Load 3D object
    Object obj1;
    if(!obj1.LoadFromObj("models/model.obj")) {
        cerr << "Failed to load 3D model" << endl;
        return -1;
    }
    obj1.UploadToGPU();

    // Compile shaders
    pangolin::GlSlProgram shader;
    shader.AddShader(pangolin::GlSlVertexShader, vertex_shader);
    shader.AddShader(pangolin::GlSlFragmentShader, fragment_shader);
    shader.Link();

    // Camera parameters (using image dimensions)
    const float fx = 500.0f;  // Focal length X (adjust as needed)
    const float fy = 500.0f;  // Focal length Y
    const float cx = width/2.0f;  // Principal point X
    const float cy = height/2.0f; // Principal point Y
pangolin::OpenGlMatrixSpec P = pangolin::ProjectionMatrixRDF_TopLeft(
    width, height, fx, fy, cx, cy, 0.001, 1000
);

        pangolin::OpenGlMatrix view = pangolin::ModelViewLookAt(
            3.0f, 3.0f, 3.0f,   // Camera position (3m from origin)
            0.0f, 0.0f, 0.0f,   // Look at object position (origin)
            0.0f, 1.0f, 0.0f    // Up vector
        );

    // Main loop
    while(!pangolin::ShouldQuit()) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // 1. Draw background image
        DrawImageTexture(imageTexture, image_rgb);
        
        // 2. Clear depth buffer for 3D objects
        glClear(GL_DEPTH_BUFFER_BIT);

        // 3. Set up projection matrix (use actual image dimensions)


        // 4. Set up view matrix (closer camera)

        // 5. Set up model matrix (reasonable scale + position)
        pangolin::OpenGlMatrix model;
        model.SetIdentity();
        model.Translate(0.0f, 0.0f, 0.0f);  // Center at origin
        float scaleFactor = 0.3f;
        model.m[0] *= scaleFactor; // scale X
        model.m[5] *= scaleFactor; // scale Y
        model.m[10] *= scaleFactor; // scale Z                // Adjust based on object size

        // 6. Render 3D object
        shader.Bind();
        glMatrixMode(GL_PROJECTION);
        P.Load(); // Applies projection matrix
        glMatrixMode(GL_MODELVIEW);
        view.Load();
        shader.SetUniform("model", model);
        obj1.Draw();
        shader.Unbind();

        pangolin::FinishFrame();
    }

    return 0;
}
