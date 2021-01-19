#include <QtGui/qpa/qplatformscreen.h>
#include <QtGui/qpa/qplatformwindow.h>
#include <QtWidgets/QtWidgets>
#include <iostream>

/*
    DprGadget: The focused High-DPI settings debug utility

    DprGadget displays the device pixel ratio ("DPR") for the screen
    it's on in a large font, as well as the inputs (from the platform
    plugin or environment) currently in use for determinging the DPR.

    Non-relevant inputs are not displayed. See qttools/src/qtdiag for
    an utility which displays all inputs.
*/

bool g_qtUsePhysicalDpi = false;
bool g_qtScaleFactor = false;

class DprGadget : public QWidget
{
public:
    std::function<void(void)> m_clearFn;
    std::function<void(void)> m_updateFn;
    qreal m_currentDpr = -1;
    QString m_eventsText;

    DprGadget() {
        setWindowTitle("DprGadget");

        QFont tinyFont;
        tinyFont.setPointSize(8);
        QFont smallFont;
        smallFont.setPointSize(12);
        QFont bigFont;
        bigFont.setPointSize(42);
        QFont biggerFont;
        biggerFont.setPointSize(80);

        QLabel *dprLabel = new QLabel("Device Pixel Ratio");
        dprLabel->setFont(bigFont);
        dprLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *dprValue = new QLabel();
        dprValue->setFont(biggerFont);
        dprValue->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *screenLabel = new QLabel("Current Screen:");
        screenLabel->setFont(smallFont);

        QLabel *sizeLabel = new QLabel("Window size:");
        sizeLabel->setFont(smallFont);
        sizeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *nativeSizeLabel = new QLabel("Native:");
        sizeLabel->setFont(smallFont);
        sizeLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *dpiLabel = new QLabel("Logical DPI:");
        dpiLabel->setFont(smallFont);
        dpiLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *windowDpiLabel = new QLabel("Window DPI:");
        windowDpiLabel->setFont(smallFont);
        windowDpiLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *platformDpiLabel = new QLabel("Native Device Pixel Ratio:");
        platformDpiLabel->setFont(smallFont);
        platformDpiLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *windowDprLabel = new QLabel("Window DPR:");
        windowDprLabel->setFont(smallFont);
        windowDprLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *plarformDprLabel = new QLabel("Native Logical DPI:");
        plarformDprLabel->setFont(smallFont);
        plarformDprLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *eventsLabel = new QLabel(m_eventsText);
        eventsLabel->setFont(tinyFont);
        eventsLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(dprLabel);
        layout->setAlignment(dprLabel, Qt::AlignHCenter);
        layout->addWidget(dprValue);
        layout->setAlignment(dprValue, Qt::AlignHCenter);

        bool displayLogicalDpi = false;
        if (displayLogicalDpi)
            layout->addWidget(dpiLabel);

        layout->addStretch();

        QHBoxLayout *screenLabelLayout = new QHBoxLayout();
        screenLabelLayout->addStretch();
        screenLabelLayout->addWidget(screenLabel);
        screenLabelLayout->addStretch();
        layout->addLayout(screenLabelLayout);

        QHBoxLayout *windowSizeLayout = new QHBoxLayout();
        windowSizeLayout->addWidget(sizeLabel);
        windowSizeLayout->addStretch();
        windowSizeLayout->addWidget(nativeSizeLabel);
        layout->addLayout(windowSizeLayout);

        QHBoxLayout *dpiLayout = new QHBoxLayout();
        dpiLayout->addWidget(windowDpiLabel);
        dpiLayout->addStretch();
        dpiLayout->addWidget(platformDpiLabel);
        layout->addLayout(dpiLayout);

        QHBoxLayout *dprLayout = new QHBoxLayout();
        dprLayout->addWidget(windowDprLabel);
        dprLayout->addStretch();
        dprLayout->addWidget(plarformDprLabel);
        layout->addLayout(dprLayout);

        layout->addWidget(eventsLabel);

        if (g_qtScaleFactor) {
            layout->addWidget(new QLabel("Active Environent:"));
            QString scaleFactorText = QString("QT_SCALE_FACTOR ") + qgetenv("QT_SCALE_FACTOR");
            layout->addWidget(new QLabel(scaleFactorText));
        }

        auto updateValues = [=]() {
            dprValue->setText(QString("%1").arg(devicePixelRatioF()));
            windowDpiLabel->setText(QString("Window DPI: %1").arg(logicalDpiX()));
            dpiLabel->setText(QString("Logical DPI: %1").arg(logicalDpiX()));
            sizeLabel->setText(QString("Window size: %1 %2").arg(width()).arg(height()));

            QPlatformWindow *platformWindow = windowHandle()->handle();
            nativeSizeLabel->setText(QString("native size %1 %2").arg(platformWindow->geometry().width())
                                                            .arg(platformWindow->geometry().height()));
            QPlatformScreen *pscreen = screen()->handle();
            if (g_qtUsePhysicalDpi)
                platformDpiLabel->setText(QString("Native Physical DPI: TODO"));
            else
                platformDpiLabel->setText(QString("native logical DPI: %1").arg(pscreen->logicalDpi().first));

            windowDprLabel->setText(QString("Window DPR: %1").arg(windowHandle()->devicePixelRatio()));
            plarformDprLabel->setText(QString("native DPR: %1").arg(pscreen->devicePixelRatio()));

            screenLabel->setText(QString("Current Screen: %1").arg(screen()->name()));
            eventsLabel->setText(QString(m_eventsText));
        };
        m_updateFn = updateValues;

        m_clearFn = [=]() {
            dprValue->setText(QString(""));
            m_eventsText.clear();
        };

        create();

        QObject::connect(this->windowHandle(), &QWindow::screenChanged, [updateValues, this](QScreen *screen){
            Q_UNUSED(screen);
            this->m_eventsText.prepend(QString("ScreenChange "));
            this->m_eventsText.truncate(80);
            updateValues();
        });

        setLayout(layout);

        updateValues();
    }

    void paintEvent(QPaintEvent *) override  {

        // Update the UI in the paint event - normally not good
        // practice but it looks like we can get away with it there
        this->m_eventsText.prepend(QString("Paint "));
        this->m_eventsText.truncate(80);

        // Dpr change should trigger a repaint, update display values here
        if (m_currentDpr == devicePixelRatioF())
            return;
        m_currentDpr = devicePixelRatioF();

        m_updateFn();
    }

    void resizeEvent(QResizeEvent *event) override {
        qDebug() << "resize";
        QSize size = event->size();
        m_eventsText.prepend(QString("Resize(%1 %2) ").arg(size.width()).arg(size.height()));
        m_eventsText.truncate(80);
        m_updateFn();
    }

    void mousePressEvent(QMouseEvent *) override {
        m_clearFn();
        QTimer::singleShot(500, this, [this](){
            m_updateFn();
        });
    }
};

int main(int argc, char **argv) {

    // Set sensible defaults
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    // react to (some) high-dpi eviornment variables.
    g_qtUsePhysicalDpi = qgetenv("QT_USE_PHYSICAL_DPI") == QByteArray("1");
    g_qtScaleFactor = qEnvironmentVariableIsSet("QT_SCALE_FACTOR");

    QApplication app(argc, argv);

    DprGadget dprGadget;

    // Set inital size. We expect this size to be preserved across screen
    // and DPI changes
//    dprGadget.resize(560, 380);

    dprGadget.show();

    QTimer::singleShot(1000, [&] () {
        for( int i = 0; i < qApp->screens().size(); i++ ) {
           std::cout << "Screen factor: " << qApp->screens().at( i )->devicePixelRatio() << std::endl;
        }
    });

    return app.exec();
}
