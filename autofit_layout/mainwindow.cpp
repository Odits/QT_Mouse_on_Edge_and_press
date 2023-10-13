#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setMouseTracking(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}


static void autoResize(QPushButton *pb)
{
    QFontMetrics fontMetrics(pb->font());
    int width = fontMetrics.width(pb->text());

//    pb->setMaximumWidth(width+50);
    pb->setFixedWidth(width+100);
}

int getGLayoutWidgetRowWidth(QGridLayout *gLayout, int rowToGet)
{
    // 获取第rowToGet行的所有组件的宽度
    int totalRowWidth = 0;
    for (int col = 0; col < gLayout->columnCount(); ++col)
    {
        QLayoutItem *item = gLayout->itemAtPosition(rowToGet, col);
        if (item)
        {
            QWidget *widget = item->widget();
            if (widget)
            {
                totalRowWidth += widget->width();
            }
        }
    }
    return totalRowWidth;
}

void add2layout(QGridLayout *gLayout, QWidget *w, int index)
{
    static int rowIndex{0};
    int layout_width = gLayout->geometry().width();
    int widget_width = getGLayoutWidgetRowWidth(gLayout, rowIndex);
    qDebug() << "Widget_width=" << widget_width << ", w->width()=" << w->width() << ", layout_width=" << layout_width;

    if (widget_width + w->width() > layout_width)   // 这里有一个bug，当创建第二行后，调整窗口的宽度，
        rowIndex++;         // 此时组件总宽度+新组件宽度永远小于当前窗口宽度，rowIndex不会++，所以新的组件会不断替换最后一行的组件

//    if (rowIndex)   // 这是一个不优雅的解决方法
//    {
//        static int old_layout_width = layout_width;
//        if (widget_width + w->width() > old_layout_width)
//            rowIndex++;
//    }

    if (rowIndex)
        gLayout->addWidget(w, rowIndex, index % gLayout->columnCount());
    else
        gLayout->addWidget(w, 0, index);
}

void autoFitGLayout(QGridLayout *gLayout)
{
    int rowIndex{0};
    int layout_width = gLayout->geometry().width();
    int widget_width = getGLayoutWidgetRowWidth(gLayout, rowIndex);

    if (widget_width < layout_width)    // 先考虑调整后布局宽度大于组件总宽度
    {
        int col_nums = gLayout->columnCount();

        auto *button = qobject_cast<QPushButton *>(gLayout->itemAt(col_nums)->widget());
        gLayout->addWidget(button, 0, col_nums);
    }
}

void save2List(QList<QWidget*> &widgetList, QGridLayout *gLayout)
{
    const int count = gLayout->count(); // 拿出来后，count会变
    for (int i = 0; i < count; ++i)
    {
        QWidget *widget = gLayout->itemAt(0)->widget();  // 永远取第0个，因为拿出来后原来的位置就空了
        if (widget)
        {
            gLayout->removeWidget(widget);
            widget->hide();
            widgetList.push_back(widget);
        }
    }
}

#include <iostream>
void addWidget2gLayout(QList<QWidget*> widgetList, QGridLayout *gLayout, int ui_width)
{
    int rowIndex{0}, colIndex, layout_colCount{0};

    for (int i{0}; i < widgetList.size(); i++)
    {
        colIndex = i - rowIndex * layout_colCount;
        if (colIndex + 1 > layout_colCount)
            layout_colCount = colIndex + 1;

        std::cout << "i=" << i << " layout_colCount=" << layout_colCount;
        std::cout << ", rowIndex=" << rowIndex << ", colIndex=" << colIndex << std::endl;

        gLayout->addWidget(widgetList[i], rowIndex, colIndex);

        widgetList[i]->show();

        if ( (i + 1 < widgetList.size()) && (getGLayoutWidgetRowWidth(gLayout, rowIndex) + widgetList[i+1]->width() > ui_width))
            rowIndex++;
    }
}


QList<QWidget*> widgetList;
static bool flag{true};

void MainWindow::resizeEvent(QResizeEvent *event)
{
    qDebug() << "enter resizeEvent layout_count=" << ui->gridLayout->count();
    // 在这里处理窗口大小变化事件，可以执行一些自定义操作

    if (flag)
        save2List(widgetList, ui->gridLayout);
//    qDebug() << "layout_count=" << ui->gridLayout->count() << ", columnCount=" << ui->gridLayout->columnCount()<< ", rowCount=" << ui->gridLayout->rowCount();
    else
    {
        addWidget2gLayout(widgetList, ui->gridLayout, this->width());
        widgetList.clear();
    }

    flag = !flag;

    QMainWindow::resizeEvent(event); // 不要忘记调用基类的resizeEvent函数
}



static bool m_isMousePressed;

#include <QMouseEvent>
void MainWindow::mousePressEvent(QMouseEvent *mouseEvent)
{
    if ( (mouseEvent->button() == Qt::LeftButton) && (this->rect().contains(mouseEvent->pos())) )
    {
        int x = mouseEvent->x();
        int y = mouseEvent->y();
        int margin = 10; // 设置边缘的宽度
        if (x >= 0 && x <= margin) {
            // 鼠标在左边缘
            m_isMousePressed = true;
        } else if (x >= this->width() - margin && x <= this->width()) {
            // 鼠标在右边缘
            m_isMousePressed = true;
        } else if (y >= 0 && y <= margin) {
            // 鼠标在上边缘
            m_isMousePressed = true;
        } else if (y >= this->height() - margin && y <= this->height()) {
            // 鼠标在下边缘
            m_isMousePressed = true;
        }
    }
    qDebug() << "m_isMousePressed=" << m_isMousePressed;
    if (m_isMousePressed)
        save2List(widgetList, ui->gridLayout);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_isMousePressed) {
        addWidget2gLayout(widgetList, ui->gridLayout, this->width());
        m_isMousePressed = false;
    }
}
void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    qDebug() << "move";
    if (this->rect().contains(event->pos())) {
        this->setCursor(Qt::SizeAllCursor);
    } else {
        this->unsetCursor();
    }
}

static int var{0};
void MainWindow::on_pushButton_clicked()
{
    auto newPB = new QPushButton(QString::number(var), this); // 使用QString::number将整数转换为字符串
//    ui->gridLayout->addWidget(newPB, 0, var);
    autoResize(newPB);
    add2layout(ui->gridLayout, newPB, var);
    var++;

}

void MainWindow::on_pushButton_2_clicked()
{
//    autoFitGLayout(ui->gridLayout);

    save2List(widgetList, ui->gridLayout);
    qDebug() << "numbs=" << widgetList.count();
}


void MainWindow::on_pushButton_3_clicked()
{
    addWidget2gLayout(widgetList, ui->gridLayout, this->width());
    widgetList.clear();
}

