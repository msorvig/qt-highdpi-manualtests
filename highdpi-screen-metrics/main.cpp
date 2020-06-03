#include <QtGui>
#include <QtWidgets>

#include <QtGui/qpa/qplatformscreen.h>

// ScreenDisplayer based on original impl from qtbase/tests/manual/highdpi
class ScreenDisplayer : public QWidget
{
public:
    ScreenDisplayer() = default;

    void timerEvent(QTimerEvent *) override
    {
        update();
    }

    void mousePressEvent(QMouseEvent *) override
    {
        if (!moveLabel)
            moveLabel = new QLabel(this,Qt::BypassWindowManagerHint|Qt::FramelessWindowHint|Qt::Window );
        moveLabel->setText("Hello, Qt this is a label\nwith some text");
        moveLabel->show();
    }

    void mouseMoveEvent(QMouseEvent *e) override
    {
        if (!moveLabel)
            return;
        moveLabel->move(e->pos() / scaleFactor);
        QString str;
        QDebug dbg(&str);
        dbg.setAutoInsertSpaces(false);
        dbg << moveLabel->geometry();
        moveLabel->setText(str);
    }

    void mouseReleaseEvent(QMouseEvent *) override
    {
        if (moveLabel)
            moveLabel->hide();
    }

    void showEvent(QShowEvent *) override
    {
        refreshTimer.start(300, this);
    }

    void hideEvent(QHideEvent *) override
    {
        refreshTimer.stop();
    }

    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        QRectF total;
        const auto screens = QGuiApplication::screens();
        for (const QScreen *screen : screens)
            total |= screen->geometry();
        if (total.isEmpty())
            return;

        scaleFactor = qMin(width()/total.width(), height()/total.height());

        p.fillRect(rect(), Qt::black);
        p.scale(scaleFactor, scaleFactor);
        p.translate(-total.topLeft());
        p.setPen(QPen(Qt::white, 10));
        p.setBrush(Qt::gray);

        for (const QScreen *screen : screens) {
            p.drawRect(screen->geometry());
            QFont f("Courier New");
            f.setPixelSize(screen->geometry().height() / 16);
            p.setFont(f);
            
            auto physicalDpi = [](QPlatformScreen *screen) {
                return screen->geometry().height() / screen->physicalSize().height() * qreal(25.4);
            };
            
            QPlatformScreen *pscreen = screen->handle();
            
            QString text = screen->name() + "\n" + "Native:" + "\n"
                + " Native geometry    : " + QString::number(pscreen->geometry().x()) + " " + QString::number(pscreen->geometry().y()) + " " + QString::number(pscreen->geometry().width()) + " " + QString::number(pscreen->geometry().height()) + "\n"
                + " Native logical DPI : " + QString::number(pscreen->logicalDpi().first) + "\n"
                + " Native physical DPI: " + QString::number(physicalDpi(pscreen)) + "\n"
                + " Native DPR         : " + QString::number(pscreen->devicePixelRatio()) + "\n"
                    
                + "\n";

            bool displayDeviceIndependentInfo = true;
            if (displayDeviceIndependentInfo) {
                text += QString("Device Independent\n") +
                    " DI geometry      : " + QString::number(screen->geometry().x()) + " " + QString::number(screen->geometry().y()) + " " + QString::number(screen->geometry().width()) + " " + QString::number(screen->geometry().height()) + "\n"
                    " DI logical DPI   : " + QString::number(screen->logicalDotsPerInch()) + "\n"
                    " DI physical DPI  : " + QString::number(screen->physicalDotsPerInch()) + "\n"
                    " DI DPR           : " + QString::number(screen->devicePixelRatio()) + "\n"
                ;
            }

            p.drawText(screen->geometry(), Qt::AlignLeft | Qt::AlignVCenter, text);
        }
        p.setBrush(QColor(200,220,255,127));

        const auto topLevels = QApplication::topLevelWidgets();
        for (QWidget *widget : topLevels) {
            if (!widget->isHidden())
                p.drawRect(widget->geometry());
        }

        QPolygon cursorShape;
        cursorShape << QPoint(0,0) << QPoint(20, 60)
                    << QPoint(30, 50) << QPoint(60, 80)
                    << QPoint(80, 60) << QPoint(50, 30)
                    << QPoint(60, 20);
        cursorShape.translate(QCursor::pos());
        p.drawPolygon(cursorShape);
    }

private:
    QLabel *moveLabel = nullptr;
    qreal scaleFactor = 1;
    QBasicTimer refreshTimer;
};

int main(int argc, char **argv) {
    
    QApplication app(argc, argv);
    
    ScreenDisplayer displayer;
    displayer.resize(300, 200);
    displayer.show();
    
    return app.exec();
}