#include "editdiagram.h"
#include <QtWidgets/QApplication>
#include <QWindow>
#include <QThread>

#include "editcontroldefine.h"
#include "globalmessagedefinition.h"



int main(int argc, char *argv[])
{

    QApplication a(argc, argv);
    EditDiagram w;

    if(argc == 2){
        WId wid = WId(QString(argv[1]).toInt());
        QWindow *window = QWindow::fromWinId(wid);

        w.setProperty("_q_embedded_native_parent_handle", QVariant(wid));
        w.winId();
        w.windowHandle()->setParent(window);
        w.show();


        GlobalMessageDefinition::StWindowsEmbedded windowsEmbedded;
        windowsEmbedded.progressType = GlobalMessageDefinition::StWindowsEmbedded::EditDiagram;
        windowsEmbedded.windowGroup.insert(w.winId(), "MainWin");

        QByteArray buffer;
        if(GlobalMessageDefinition::StructToJson(buffer, windowsEmbedded)){//转换成功
            fprintf(stderr, "%s", buffer.data());
            QThread::msleep(20);
        }
    }
    else
        w.show();



    return a.exec();
}
