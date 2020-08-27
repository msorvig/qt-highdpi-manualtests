#include <QtWidgets/QtWidgets>

#include <QtGui/qpa/qplatformscreen.h>

bool g_usePhysicalDpi = false;
bool g_devicePixelRatio = false;

class DprGadget : public QWidget
{
public:
    std::function<void(void)> m_clearFn;
    std::function<void(void)> m_updateFn;
    qreal m_currentDpr = -1;

    DprGadget() {
        setWindowTitle("DprGadget");

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

        QLabel *dpiLabel = new QLabel("Logical DPI:");
        dpiLabel->setFont(smallFont);
        dpiLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *platformDpiLabel = new QLabel("Native Device Pixel Ratio:");
        QLabel *plarformDprLabel = new QLabel("Native Logical DPI:");
        platformDpiLabel->setFont(smallFont);
        platformDpiLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
        plarformDprLabel->setFont(smallFont);
        plarformDprLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);

        QLabel *screenLabel = new QLabel("Current Screen:");
        screenLabel->setFont(smallFont);

        QVBoxLayout *layout = new QVBoxLayout();
        layout->addWidget(dprLabel);
        layout->setAlignment(dprLabel, Qt::AlignHCenter);
        layout->addWidget(dprValue);
        layout->setAlignment(dprValue, Qt::AlignHCenter);

        bool displayLogicalDpi = false;
        if (displayLogicalDpi)
            layout->addWidget(dpiLabel);

        if (g_devicePixelRatio) {
            QString text = QString("QT_SCALE_FACTOR ") + qgetenv("QT_SCALE_FACTOR");
            layout->addWidget(new QLabel(text));
        }

        layout->addStretch();
        layout->addWidget(screenLabel);
        layout->addWidget(platformDpiLabel);
        layout->addWidget(plarformDprLabel);

        auto updateValues = [=]() {
            dprValue->setText(QString("%1").arg(devicePixelRatioF()));
            dpiLabel->setText(QString("Logical DPI: %1").arg(logicalDpiX()));

            QPlatformScreen *pscreen = screen()->handle();

            if (g_usePhysicalDpi)
                platformDpiLabel->setText(QString("Native Physical DPI: TODO"));
            else
                platformDpiLabel->setText(QString("Native Logical DPI: %1").arg(pscreen->logicalDpi().first));

            plarformDprLabel->setText(QString("Native Device Pixel Ratio: %1").arg(pscreen->devicePixelRatio()));

            screenLabel->setText(QString("Current Screen: %1").arg(screen()->name()));
        };
        m_updateFn = updateValues;

        m_clearFn = [=]() {
            dprValue->setText(QString(""));
        };

        create();

        QObject::connect(this->windowHandle(), &QWindow::screenChanged, [updateValues](QScreen *screen){
            Q_UNUSED(screen);
            updateValues();
        });

        setLayout(layout);

        updateValues();
    }

    void paintEvent(QPaintEvent *) override  {
        // Dpr change should trigger a repaint, update display values here
        if (m_currentDpr == devicePixelRatioF())
            return;
        m_currentDpr = devicePixelRatioF();

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
    g_usePhysicalDpi = qgetenv("QT_USE_PHYSICAL_DPI") == QByteArray("1");
    g_devicePixelRatio = qEnvironmentVariableIsSet("QT_SCALE_FACTOR");

    QApplication app(argc, argv);

    DprGadget dprGadget;
    dprGadget.resize(320, 300);
    dprGadget.show();

    return app.exec();
}
