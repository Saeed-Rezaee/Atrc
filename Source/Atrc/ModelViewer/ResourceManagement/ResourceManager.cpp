#include <Atrc/ModelViewer/ResourceManagement/CameraCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/EntityCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/FilmFilterCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/FresnelCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/GeometryCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/LightCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/MaterialCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/PathTracingIntegratorCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/RendererCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/SamplerCreator.h>
#include <Atrc/ModelViewer/ResourceManagement/TextureCreator.h>

void RegisterResourceCreators(ResourceManager &rscMgr)
{
    RegisterCameraCreators(rscMgr);
    RegisterEntityCreators(rscMgr);
    RegisterFilmFilterCreators(rscMgr);
    RegisterFresnelCreators(rscMgr);
    RegisterGeometryCreators(rscMgr);
    RegisterLightCreators(rscMgr);
    RegisterMaterialCreators(rscMgr);
    RegisterPathTracingIntegratorCreators(rscMgr);
    RegisterRendererCreators(rscMgr);
    RegisterSamplerCreators(rscMgr);
    RegisterTextureCreators(rscMgr);
}