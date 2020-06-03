#include <QtGui>
#include <QtGui/qpa/qplatformwindow.h>
#include <QtGui/private/qhighdpiscaling_p.h>

class ColorWindow : public QRasterWindow
{
public:
    ColorWindow(QColor color)
        :m_color(color) {
    }
    
    void addChildWindow(ColorWindow *window) {
        window->setParent(this);
        m_childWindows.append(window);
        window->show();
    }
    
    void paintEvent(QPaintEvent *ev) {
        QPainter p(this);
        QRect allOfIt(QPoint(0,0), size());
        p.fillRect(allOfIt, m_color);
        
        QFont font("Courier");
        font.setStyleHint(QFont::Monospace);
        p.setFont(font);
        
        auto toString = [](QRect g) {
            return QString("%1 %2 %3 %4").arg(g.x()).arg(g.y()).arg(g.width()).arg(g.height());
        };
        auto toStringP = [](QPoint g) {
            return QString("%1 %2").arg(g.x()).arg(g.y());
        };
        auto globalGeometry = [this](){
            if (this->isTopLevel()) {
                return this->geometry();
            } else {
                return QRect(QWindow::mapToGlobal(QPoint(0,0)), geometry().size());
            }
        };
        
        int y = 5;
        int incr = 15;
        
        QString windowString = QString("Window: %1 topLevel: %2").arg(m_color.name()).arg(isTopLevel());
        QString geometryString =       QString("       Geometry: ") + toString(geometry());
        QString nativeGeometryString = QString("Native Geometry: ") + toString(handle()->geometry());
        QString globalGeometryString = QString("Global Geometry: ") + toString(globalGeometry());
        
        p.drawText(5, y+=incr, windowString);
        p.drawText(5, y+=incr, geometryString);
        p.drawText(5, y+=incr, nativeGeometryString);
        p.drawText(5, y+=incr, globalGeometryString);
        
        y+=incr;
        
        // Print the window's scale factor (and the devicePixelRatio for completeness)
        QString device_native =            QString("Factor native/dip\n [QHighDpiScaling::factor(win)]: %1").arg(QHighDpiScaling::factor(this));
        QString device_deviceIndependent = QString("Factor dp/dip\n [QWindow::devicePixelRatio()] : %1").arg(devicePixelRatio());
        p.drawText(QRect(5, y+=incr, 400, 200), device_native);
        y+=incr;
        p.drawText(QRect(5, y+=incr, 400, 200), device_deviceIndependent);
        y+=incr;
        y+=incr;
        
        // Print event geometry for the last mouse press. Also print mapTo/FromGlobal mapped
        // geometry
        p.drawText(5, y+=incr, QString("Last Mouse Press:"));
        p.drawText(5, y+=incr, QString("localPos: %1").arg(toStringP(m_lastLocalPos)));
        p.drawText(5, y+=incr, QString("globalPos:  %1").arg(toStringP(m_lastGlobalPos)));
        p.drawText(5, y+=incr, QString("mapToLocal:  %1").arg(toStringP(m_lastMapToLocalPos)));
        p.drawText(5, y+=incr, QString("mapToGlobal: %1").arg(toStringP(m_lastMapToGlobalPos)));
    }
    
    void resizeEvent(QResizeEvent *) {
        int totalWindowCount = m_childWindows.count() + 1; // child + topLevel
        int slice = width() / totalWindowCount;
        slice-=10; // don't cover root
        qDebug() << "ColorWindow::resizeEvent" << this << geometry() << slice;
        for (int i = 0; i < m_childWindows.count(); ++i) {
            m_childWindows[i]->setPosition(slice * (i + 1), 0);
            m_childWindows[i]->resize(slice, height());
        }
        
//        m_lastLocalPos = QPoint();
//        m_lastGlobalPos = QPoint();
    }
    
    void mousePressEvent(QMouseEvent *ev) {
        
        qDebug() << "mousePressEvent";

        m_lastLocalPos = ev->localPos().toPoint();
        m_lastGlobalPos = ev->screenPos().toPoint();
        m_lastMapToLocalPos = QWindow::mapFromGlobal(m_lastGlobalPos);
        m_lastMapToGlobalPos = QWindow::mapToGlobal(m_lastLocalPos);
        requestUpdate();
        
        setGeometry(QRect(geometry().topLeft() + (QPoint(1,1)), geometry().size()));
    }

    QColor m_color;
    QVector<ColorWindow *> m_childWindows;
    QPoint m_lastLocalPos;
    QPoint m_lastGlobalPos;
    QPoint m_lastMapToLocalPos;
    QPoint m_lastMapToGlobalPos;
};


bool g_displayChildWindows = true;

int main(int argc, char **argv) {
    QGuiApplication app(argc, argv);
    
    ColorWindow root(QColorConstants::Svg::lightblue);
    root.setTitle("Window and Event Geometry");
    root.resize(480, 100);
    QObject::connect(&root, &QWindow::screenChanged, [&root](QScreen *screen){
        qDebug() << "root screen changed" << screen << root.geometry();
    });

    if (g_displayChildWindows) {
        root.addChildWindow(new ColorWindow(QColorConstants::Svg::blueviolet));
    //    root.addChildWindow(new ColorWindow(QColorConstants::Svg::lightgreen));
    }

    root.show();

    
    return app.exec();
}

