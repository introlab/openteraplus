#ifndef DANCECONFIGWIDGET_H
#define DANCECONFIGWIDGET_H

#include <QWidget>
#include "managers/ComManager.h"
#include "DanceComManager.h"

namespace Ui {
class DanceConfigWidget;
}

class DanceConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DanceConfigWidget(ComManager* comManager, int projectId, QWidget *parent = nullptr);
    ~DanceConfigWidget();

private slots:
    void on_btnUpload_clicked();

private:
    Ui::DanceConfigWidget *ui;

    ComManager* m_comManager;
    int         m_idProject;

    DanceComManager* m_danceComManager;
};

#endif // DANCECONFIGWIDGET_H
