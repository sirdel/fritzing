#include "gcodedialog.h"

#define PROGRESS ":/resources/images/progress.gif"



RenderArea::RenderArea(QWidget *parent)
    : QWidget(parent)
{
    setBackgroundRole(QPalette::Dark);
    setAutoFillBackground(true);

}

RenderArea::~RenderArea(){}


QSize RenderArea::minimumSizeHint() const
{
    return QSize(200, 200);
}

QSize RenderArea::sizeHint() const
{
    return QSize(400, 400);
}

void RenderArea::AddPoly(QPolygonF *Poly)
{
    mPolys.append(Poly);
}

void RenderArea::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(Qt::black);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // draw
    for (int i = 0; i < mPolys.size(); ++i) {
        painter.drawPolygon(*mPolys.at(i));
    }
}






void GcodeDialog::SetupControls(int brdlayers, int iRooted, int iunRooted)
{
    this->setWindowTitle(QObject::tr("GCODE Viewer"));

    QVBoxLayout * windowLayout = new QVBoxLayout();
    this->setLayout(windowLayout);

    // warning if rooting still not complete
    if (iunRooted > 0) {
        QLabel * warn = new QLabel( tr("Warning %1 connector(s) still to be routed").arg(iunRooted), this );
        warn->setWordWrap(false);
        windowLayout->addWidget(warn);
    }

    QLabel * label = new QLabel(tr("Display top & bottom copper layers as gcode.\nFound %1 net(s) and xx pad(s).").arg(iRooted),this);
    label->setWordWrap(true);
    windowLayout->addWidget(label);

    // add the pcb display
    millRender = new RenderArea();
    windowLayout->addWidget(millRender);

    // display board layers
    QGroupBox *boardBox  = new QGroupBox(tr("Board Layers"), this);
    QCheckBox *chkBot = new QCheckBox(tr("bottom layer"), this);
    QCheckBox *chkTop = new QCheckBox(tr("top layer"), this);
    chkBot->setChecked(true);
    if ( brdlayers>1 ) chkTop->setChecked(true);
    else chkTop->setEnabled(false);
    QHBoxLayout *hbox = new QHBoxLayout;
    hbox->addWidget(chkBot);
    hbox->addWidget(chkTop);
    boardBox->setLayout(hbox);
    windowLayout->addWidget(boardBox);

    // drill options
    QGroupBox *drillBox  = new QGroupBox( tr("Drill options"), this);
    QCheckBox *chkMill = new QCheckBox(tr("Mill Drill"), this);
    QLabel * lblDrill = new QLabel(tr("Combine with"), this);
    QComboBox *cboDrill = new QComboBox(this);
    cboDrill->addItem(tr("as seperate file"));
    cboDrill->addItem(tr("bottom"));
    if ( brdlayers>1 ) cboDrill->addItem(tr("top"));
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addWidget(chkMill);
    vbox->addWidget(lblDrill);
    vbox->addWidget(cboDrill);
    drillBox->setLayout(vbox);
    windowLayout->addWidget(drillBox);


    // Serial options
    QGroupBox *serialBox  = new QGroupBox( tr("Miller options"), this);
    QLabel * lblSerial = new QLabel(tr("Serial Device"),this);
    QLabel * lblMovie = new QLabel(tr("progress icon"),this);
    QMovie * movie = new QMovie(PROGRESS);
    if (movie->isValid()) {
        lblMovie->setMovie(movie);
        movie->start();
    }
    QComboBox *cboSerial = new QComboBox(this);
    cboSerial->addItem(tr("ttyUSB0"));
    QPushButton *pushMill = new QPushButton(tr("Download to miller"),this);

    QVBoxLayout *serialVBbox = new QVBoxLayout;
    serialVBbox->addWidget(lblSerial);
    serialVBbox->addWidget(lblMovie);
    serialVBbox->addWidget(cboSerial);
    serialVBbox->addWidget(pushMill);
    serialBox->setLayout(serialVBbox);
    windowLayout->addWidget(serialBox);

    // add standard dialog buttons
    QDialogButtonBox * buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Close);
    buttonBox->button(QDialogButtonBox::Save)->setText(tr("Save As.."));
    buttonBox->button(QDialogButtonBox::Close)->setText(tr("Close"));
    windowLayout->addWidget(buttonBox);

    connect(buttonBox, SIGNAL(accepted()), this, SLOT(doSave()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(doClose()));
}




GcodeDialog::GcodeDialog(PCBSketchWidget *sketchWidget, QGraphicsItem *board, int unRooted, int Rooted, QWidget *parent)
    : QDialog(parent)
{ 

    // add controls to the window
    SetupControls(sketchWidget->boardLayers(), Rooted, unRooted);


    using namespace ClipperLib;

    typedef std::vector<IntPoint> Path;
    typedef std::vector<Path> Paths;


    // grab all wires
    QList<QGraphicsItem *> items = sketchWidget->scene()->collidingItems(board);

    Clipper AllCopper0Traces, AllCopper1Traces ;

    foreach (QGraphicsItem * item, items) {
        Wire * wire = dynamic_cast<Wire *>(item);
        if (wire == NULL) continue;
        if ( wire->hasFlag(ViewGeometry::PCBTraceFlag) )  {
            if (wire->parentItem() != NULL)
                continue;	// skip module wires

            // convert to real world position
            QLineF *tmpLine = new QLineF(wire->line());
            tmpLine->translate(wire->pos());
            wire->debugInfo( ( QString("wire x1:%1 y1:%2 x2:%3 y2:%4").arg(tmpLine->x1()).arg(tmpLine->y1()).arg(tmpLine->x2()).arg(tmpLine->y2()) ) );

            // Clipper only works with paths, so convert it
            Path clipLine;
            clipLine.push_back(IntPoint(tmpLine->x1(),tmpLine->y1()));
            clipLine.push_back(IntPoint(tmpLine->x2(),tmpLine->y2()));

            // inflate it
            Paths clipInflatedLine;
            ClipperOffset clipOffset(2, 0.250);
            clipOffset.AddPath(clipLine, jtRound, etOpenRound);
            clipOffset.Execute(clipInflatedLine, 0.5*wire->width());

            // add it to the top or bottom traces container
            // warning default to first path
            if (wire->viewLayerID()==ViewLayer::Copper0Trace)
            {
                if (!AllCopper0Traces.AddPath(clipInflatedLine[0], ptSubject, true) )
                    DebugDialog::debug( QString("Poly addpath failed") );
            }
            else
            {
                if (!AllCopper1Traces.AddPath(clipInflatedLine[0], ptSubject, true) )
                    DebugDialog::debug( QString("Poly addpath failed") );
            }

        }
    }

    // get connectors
    foreach (QGraphicsItem * item, items) {
        ConnectorItem * conn = dynamic_cast<ConnectorItem *>(item);
        if (conn == NULL) continue;
        if ((conn->attachedTo()->wireFlags()==ViewGeometry::NoFlag) && (conn->attachedToViewLayerID()==ViewLayer::Copper0 )) {
            conn->debugInfo( QString("Conn") );
            QRectF * connRect = new QRectF;

            DebugDialog::debug( QString("Rad %1   Rect ht %2  is circle %3").arg(conn->radius() ).arg(conn->rect().height()).arg(conn->isEffectivelyCircular()) );

            connRect->setWidth(2*conn->radius()); connRect->setHeight(2*conn->radius());
            connRect->moveCenter(conn->sceneAdjustedTerminalPoint(NULL));
            //QGraphicsEllipseItem * gCir = sketchWidget->scene()->addEllipse(* connRect, QPen(Qt::blue,0) );
            //gCir->setZValue(10);

            // create a temp path representing the connector
            Path clipConnPath;
            clipConnPath.push_back( IntPoint(connRect->bottomLeft().x(),connRect->bottomLeft().y()) );
            clipConnPath.push_back( IntPoint(connRect->topLeft().x(),connRect->topLeft().y()) );
            clipConnPath.push_back( IntPoint(connRect->topRight().x(),connRect->topRight().y()) );
            clipConnPath.push_back( IntPoint(connRect->bottomRight().x(),connRect->bottomRight().y()) );

            if (!AllCopper0Traces.AddPath(clipConnPath, ptSubject, true) )
                DebugDialog::debug( QString("Connector addpath failed") );
        }
    }


    // get board mounting holes
    foreach (QGraphicsItem * item, items) {
        Hole * hole = dynamic_cast<Hole *>(item);
        if (hole == NULL)
            continue;
        hole->debugInfo(QString("Hole "));
    }


    // get pads
    foreach (QGraphicsItem * item, items) {
        Pad * pad = dynamic_cast<Pad *>(item);
        if (pad == NULL)
            continue;
        pad->debugInfo(QString("Pad "));
    }

    // combine all wires and connections into a union of polygons for top and bottom layers
    Paths AllCopper0TracesUnion, AllCopper1TracesUnion;
    AllCopper0Traces.Execute(ctUnion, AllCopper0TracesUnion, pftPositive, pftPositive);
    AllCopper1Traces.Execute(ctUnion, AllCopper1TracesUnion, pftPositive, pftPositive);

    int ii=0;

    // covnert each clipper polygons to Qt polygons
    for(Paths::iterator poly = AllCopper0TracesUnion.begin(); poly < AllCopper0TracesUnion.end(); ++poly ){
        //DebugDialog::debug( QString("Polygon %1").arg(ii) );

        // iterate through every point of the polygon
        QPolygonF * polyF = new QPolygonF;
        for(std::vector<IntPoint>::iterator pnt = poly->begin(); pnt != poly->end(); ++pnt) {
            polyF->push_back( QPointF(pnt->X,pnt->Y) );
        }

        // add it to the renderer
        millRender->AddPoly(polyF);


        //QGraphicsPolygonItem * gPoly = sketchWidget->scene()->addPolygon( *polyF, QPen(Qt::blue,0) );
        //gPoly->setZValue(10);


        ++ii;
    }

    // force refresh
    millRender->update();

}

GcodeDialog::~GcodeDialog()
{
}


void GcodeDialog::doSave(){
    done(Accepted);
}

void GcodeDialog::doClose(){
    done(Rejected);
}
