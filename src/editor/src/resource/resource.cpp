#include <agz/editor/entity/entity_factory.h>
#include <agz/editor/envir_light/envir_light_factory.h>
#include <agz/editor/geometry/geometry_factory.h>
#include <agz/editor/material/material_factory.h>
#include <agz/editor/medium/medium_factory.h>
#include <agz/editor/texture2d/texture2d_factory.h>
#include <agz/editor/texture3d/texture3d_factory.h>

AGZ_EDITOR_BEGIN

ObjectContext::ObjectContext(Editor *editor)
{
    init_entity_factory     (factory<tracer::Entity>());
    init_envir_light_factory(factory<tracer::EnvirLight>());
    init_geometry_factory   (factory<tracer::Geometry>());
    init_material_factory   (factory<tracer::Material>());
    init_medium_factory     (factory<tracer::Medium>());
    init_texture2d_factory  (factory<tracer::Texture2D>());
    init_texture3d_factory  (factory<tracer::Texture3D>());

    std::get<Box<ResourcePool<tracer::Material>>>(pools_) =
        newBox<MaterialPool>(*this, editor, "Ideal Diffuse");
    std::get<Box<ResourcePool<tracer::Texture2D>>>(pools_) =
        newBox<Texture2DPool>(*this, editor, "Constant");
    std::get<Box<ResourcePool<tracer::Texture3D>>>(pools_) =
        newBox<Texture3DPool>(*this, editor, "Constant");
    std::get<Box<ResourcePool<tracer::Geometry>>>(pools_) =
        newBox<GeometryPool>(*this, editor, "Sphere");
    std::get<Box<ResourcePool<tracer::Medium>>>(pools_) =
        newBox<MediumPool>(*this, editor, "Void");
}

AGZ_EDITOR_END
