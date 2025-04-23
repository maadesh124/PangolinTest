#include <pangolin/geometry/geometry.h>
#include <pangolin/geometry/geometry_obj.h>
#include <memory>                            
#include <vector>                            
#include <pangolin/geometry/glgeometry.h> 
#include <chrono> 
#include <rendertree.h>


      


class Object {
public:
    explicit Object(const std::string& path)
        : filepath(path), loaded(false)
    {
        geometry = pangolin::LoadGeometry(filepath);  // Load from file but not uploaded to GPU
    }

    const pangolin::Geometry& GetGeometry() const { return geometry; }
    const std::string& GetPath() const { return filepath; }
    bool IsLoaded() const { return loaded; }

    void MarkLoaded() { loaded = true; }

private:
    std::string filepath;
    pangolin::Geometry geometry;
    bool loaded;
};


std::vector<std::shared_ptr<GlGeomRenderable>> renderables;

void LoadObjectToGPU(Object& obj)
{
    if (obj.IsLoaded()) return;

    pangolin::GlGeometry glgeom = pangolin::ToGlGeometry(obj.GetGeometry());
    auto renderable = std::make_shared<GlGeomRenderable>(
    std::move(glgeom),
    pangolin::GetAxisAlignedBox(obj.GetGeometry())
);

    renderables.push_back(renderable);

    obj.MarkLoaded();
}
