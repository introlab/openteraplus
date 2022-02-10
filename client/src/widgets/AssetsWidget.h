#ifndef ASSETSWIDGET_H
#define ASSETSWIDGET_H

#include <QWidget>
#include <QTreeWidgetItem>

#include "managers/ComManager.h"
#include "managers/AssetComManager.h"
#include "TeraAsset.h"

namespace Ui {
class AssetsWidget;
}

class AssetsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AssetsWidget(ComManager* comMan = nullptr, QWidget *parent = nullptr);
    ~AssetsWidget();

    void setComManager(ComManager* comMan);

    bool isEmpty() const;
    void clearAssets();

    void displayAssetsForSession(const int &id_session);

private:
    Ui::AssetsWidget *ui;

    ComManager*         m_comManager;
    AssetComManager*    m_comAssetManager;

    QMap<int, QTreeWidgetItem*>  m_treeAssets;
    QMap<int, QTreeWidgetItem*>  m_treeSessions;
    QMap<int, QTreeWidgetItem*>  m_treeParticipants;
    QMap<int, TeraData*>         m_assets;

    void resizeAssetsColumnsToContent();

    void queryAssetsInfos();


private slots:
    void processAssetsReply(QList<TeraData> assets, QUrlQuery reply_query);

};

#endif // ASSETSWIDGET_H
