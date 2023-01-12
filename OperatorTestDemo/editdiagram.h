#ifndef EDITDIAGRAM_H
#define EDITDIAGRAM_H

#include <QWidget>

class EditControl;


QT_BEGIN_NAMESPACE
namespace Ui { class EditDiagram; }
QT_END_NAMESPACE

class EditDiagram : public QWidget
{
    Q_OBJECT

public:
    EditDiagram(QWidget *parent = nullptr);
    ~EditDiagram();

private:
    Ui::EditDiagram *ui;
    EditControl* m_editControl;
};
#endif // EDITDIAGRAM_H
