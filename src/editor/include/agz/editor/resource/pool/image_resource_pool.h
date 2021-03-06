#pragma once

#include <agz/editor/resource/pool/image_text_icon.h>
#include <agz/editor/resource/pool/image_resource_pool_ui.h>
#include <agz/editor/resource/resource.h>
#include <agz/editor/ui/utility/flow_layout.h>

AGZ_EDITOR_BEGIN

class Editor;

template<typename TracerObject>
class ImageResourcePool : public QObject, public ResourcePool<TracerObject>
{
public:

    ImageResourcePool(
        ObjectContext &obj_ctx, Editor *editor, const QString &default_type);

    Box<ResourceReference<TracerObject>> select_resource() override;

    ResourceInPool<TracerObject> *add_resource(
        const QString &name, Box<ResourcePanel<TracerObject>> panel) override;

    void save_asset(AssetSaver &saver) const override;

    void load_asset(AssetLoader &loader) override;

    void to_config(
        tracer::ConfigGroup &scene_grp, JSONExportContext &ctx) const override;

    ResourceInPool<TracerObject> *name_to_rsc(const QString &name) override;

    bool is_valid_name(const QString &name) const override;

    QString to_valid_name(const QString &name) const override;

    void show_edit_panel(
        ResourcePanel<TracerObject> *rsc, bool display_rsc_panel) override;

    QWidget *get_widget() override;

private:

    struct Record
    {
        QString name;
        Box<ResourceInPool<TracerObject>> rsc;
        ImageTextIcon *icon = nullptr;
    };

    void set_selected_rsc(Record *record);

    QString default_type_;
    ObjectContext &obj_ctx_;
    Editor *editor_ = nullptr;

    std::map<QString, Box<Record>> name2rsc_;

    Record *selected_rsc_ = nullptr;

    QFont icon_font_;

    FlowLayout              *layout_ = nullptr;
    ImageResourcePoolWidget *ui_     = nullptr;
};

AGZ_EDITOR_END
