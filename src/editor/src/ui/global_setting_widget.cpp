#include <QVBoxLayout>

#include <agz/editor/imexport/asset_loader.h>
#include <agz/editor/imexport/asset_saver.h>
#include <agz/editor/ui/global_setting_widget.h>

AGZ_EDITOR_BEGIN

GlobalSettingWidget::GlobalSettingWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi(this);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(gridLayoutWidget);
}

void GlobalSettingWidget::save_asset(AssetSaver &saver) const
{
    saver.write(scene_eps->value());
}

void GlobalSettingWidget::load_asset(AssetLoader &loader)
{
    blockSignals(true);
    scene_eps->setValue(loader.read<double>());
    blockSignals(false);
}

AGZ_EDITOR_END
