#include <QtWidgets/QtWidgets>
#include <tuple>
#include <stdlib.h>

class TestWidget : public QWidget
{
public:
    QVector<std::tuple<QPoint, QColor>> m_points;
    
    
    void paintEvent(QPaintEvent *e) {
        
        QPainter painter(this);
        
        qDebug() << "# paintEvent" << rect() << screen();
        
        painter.fillRect(rect(), QColorConstants::Svg::steelblue);
        for (auto [point, color] : m_points) {
            QRect r;
            r.setSize(QSize(10, 10));
            r.moveCenter(point);
            painter.fillRect(r, color);
        }
    }
    
    enum CompareStatus { Equal, OffByone, OffByMany };
    CompareStatus testPoint(QPoint candidate, QPoint expected)
    {
        if (candidate == expected)
            return CompareStatus::Equal;
        
        if (abs(candidate.x() - expected.x()) <=1 && abs(candidate.y() - expected.y()) <= 1)
            return CompareStatus::OffByone;

        return CompareStatus::OffByMany;
    }
    
    CompareStatus testPoints(QVector<QPoint> candidates, QPoint expected)
    {
        for (QPoint candiadate: candidates) {
            CompareStatus status = testPoint(candiadate, expected);
            if (status == CompareStatus::OffByone)
                return CompareStatus::OffByone;
            if (status == CompareStatus::OffByMany)
                return CompareStatus::OffByMany;
        }
        return CompareStatus::Equal;
    }
    
    
    void mousePressEvent(QMouseEvent *e) override {
        qDebug() << "## onMousePressed e->pos()" << e->pos() << "e->globalPos()"<< e->globalPos();
        
        // mapped positions
        QPoint ePos = e->pos();
        QPoint eMapFromGlobalPos = mapFromGlobal(e->globalPos());
        QPoint eMapFromGlobalMapFromLocalPos = mapFromGlobal(mapToGlobal(e->pos()));
        
        qDebug() << "## onMousePressed e->pos()" << e->pos() 
                 << "eMapFromGlobalPos"<< eMapFromGlobalPos
                 << "eMapFromGlobalMapFromLocalPos"<< eMapFromGlobalMapFromLocalPos;
        
        CompareStatus status = testPoints({eMapFromGlobalPos, eMapFromGlobalMapFromLocalPos}, ePos);
        QColor color;
        if (status == CompareStatus::Equal)
            color = QColorConstants::Svg::green;
        if (status == CompareStatus::OffByone)
            color = QColorConstants::Svg::yellow;
        if (status == CompareStatus::OffByMany)
            color = QColorConstants::Svg::red;
                
        m_points.append(std::make_tuple(e->pos(), color));
        update();
    }
};

int main(int argc, char **argv) {
    
    QApplication app(argc, argv);
    
    TestWidget *testWidget = new TestWidget();
    testWidget->resize(320, 200);
    testWidget->show();
    testWidget->winId();
    
    QScrollArea *scrollArea = new QScrollArea();
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(testWidget);
    scrollArea->winId();
//    scrollArea->show();
    
    QWidget *topLevel = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(scrollArea); // for the margin offset from (0,0)
    topLevel->setLayout(layout);
        
    topLevel->show();
    
    return app.exec();
    
    delete topLevel;
}


