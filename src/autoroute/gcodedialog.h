#ifndef GCODEDIALOG_H
#define GCODEDIALOG_H

#include <QDialog>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsItem>
#include <QListWidget>
#include <QLabel>
#include <QGroupBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSettings>
#include <QComboBox>
#include <QMovie>

#include "mazerouter/mazerouter.h"
#include "../sketch/pcbsketchwidget.h"
#include "../items/wire.h"
#include "../items/hole.h"
#include "../items/pad.h"
#include "../debugdialog.h"
#include "../utils/clipper.h"


class GcodeDialog : public QDialog
{
Q_OBJECT
private:
    QGraphicsScene *m_scene;
    QListWidget * m_listWidget;

    void SetupControls(int, int iRooted, int iunRooted);

public:
    GcodeDialog(class PCBSketchWidget *, QGraphicsItem * board, int, int, QWidget *parent = 0);

    ~GcodeDialog();

 public slots:
    void doSave();
    void doClose();


};


#endif // GCODEDIALOG_H
