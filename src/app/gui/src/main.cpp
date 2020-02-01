#include <QApplication>

#include <agz/gui/gui.h>

#if defined(_WIN32) && defined(_DEBUG)
#include <crtdbg.h>
#endif

int main(int argc, char *argv[])
{
#if defined(_WIN32) && defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif

    QApplication app(argc, argv);

    GUI gui;
    gui.setWindowTitle("Atrc Renderer by AirGuanZ");
    gui.resize(640, 480);
    gui.show();

    return QApplication::exec();
}
