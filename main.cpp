#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QCursor>
#include <QTimer>
#include <QDebug>
#include <QMouseEvent>

class MyWidget : public QWidget
{
    Q_OBJECT
public:
    MyWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        // 创建一个标签
        label = new QLabel("Resize Me", this);
        label->move(50, 50);

        // 设置定时器以检测鼠标位置
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MyWidget::checkMousePosition);
        timer->start(500); // 设置检测间隔，可以根据需要调整
    }

private slots:
    void checkMousePosition()
    {
        // 获取鼠标在屏幕上的位置
        QPoint globalMousePos = QCursor::pos();

        // 标题栏的高度
        int titleBarHeight = frameGeometry().y() - geometry().y();

        // 获取窗口的边缘矩形
        QRect windowRect = this->geometry().adjusted(-m_iMarginWidth, -m_iMarginWidth + titleBarHeight, m_iMarginWidth, m_iMarginWidth);
        QRect windowRect1 = this->geometry().adjusted(m_iMarginWidth, m_iMarginWidth + titleBarHeight, -m_iMarginWidth, -m_iMarginWidth);

        Qt::MouseButtons buttons = qApp->mouseButtons();

        // 检测鼠标是否位于windowRect内，且在windowRect1外，且鼠标左键按下
        if (windowRect.contains(globalMousePos) && !windowRect1.contains(globalMousePos) && buttons & Qt::LeftButton)
        {
            // 鼠标现在位于窗口边缘附近
            isMouseOnEdge = true; // 标记鼠标在边缘
            label->hide();
            static int i{0};
            qDebug() << "Mouse On Edge & press" << i++;
        }
        else
        {
            isMouseOnEdge = false; // 标记鼠标不在边缘
            label->show();
        }
    }
private:
    QLabel *label;
    bool isMouseOnEdge;
    int m_iMarginWidth = 5; // 边缘宽度，根据你的需求设定
};


#include <QtGui>

class MyWidget1 : public QWidget
{
    Q_OBJECT

    int m_iMarginWidth = 5; // 边缘宽度，根据你的需求设定
protected:
    bool nativeEvent(const QByteArray &eventType, void *message, long *result);

    bool isMouseNearWindowEdge(const QPoint &mousePos, int titleBarHeight)
    {
        QRect windowRect = this->frameGeometry().adjusted(-m_iMarginWidth, -m_iMarginWidth + titleBarHeight, m_iMarginWidth, m_iMarginWidth);
        QRect windowRect1 = this->frameGeometry().adjusted(m_iMarginWidth, m_iMarginWidth + titleBarHeight, -m_iMarginWidth, -m_iMarginWidth);

        return windowRect.contains(mousePos) && !windowRect1.contains(mousePos);
    }
};

#ifdef Q_OS_WIN

bool MyWidget1::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);

    MSG* msg = static_cast<MSG*>(message);

    static int i{0};
    switch (msg->wParam)
    {
    case HTTOP:         // 上
    case HTBOTTOM:      // 下
    case HTLEFT:        // 左
    case HTRIGHT:       // 右
    case HTTOPLEFT:     // 左上
    case HTTOPRIGHT:    // 右上
    case HTBOTTOMLEFT:  // 左下
    case HTBOTTOMRIGHT: // 右下
        if (msg->message == WM_NCLBUTTONDOWN)
            qDebug() << "BORDER & press" << i++;
        break;
    }

    // 继续处理事件
    return false;
}

#else

#include <QCoreApplication>
#include <QX11Info>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

bool MyWidget1::nativeEvent(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(eventType);

    if (eventType == "xcb_generic_event_t")
    {
        xcb_generic_event_t *xEvent = static_cast<xcb_generic_event_t*>(message);
        if (xEvent->response_type == XCB_BUTTON_PRESS)
        {
            static int i{0};
            qDebug() << "i=" << i++;
            xcb_button_press_event_t *buttonPressEvent = reinterpret_cast<xcb_button_press_event_t*>(xEvent);

            // 获取鼠标按钮号码，1表示左键
            if (buttonPressEvent->detail == XCB_BUTTON_INDEX_1)
            {
                qDebug() << "buttonPress" << i++;
                // 获取鼠标位置
                QPoint globalMousePos(buttonPressEvent->event_x, buttonPressEvent->event_y);
                int titleBarHeight = frameGeometry().y() - geometry().y();
                bool isMouseOnEdge = isMouseNearWindowEdge(globalMousePos, titleBarHeight);

                if (isMouseOnEdge)
                {
                    // 鼠标左键按下，且位于窗口边缘附近
                    qDebug() << "MouseOnEdge" << i++;
                }
            }
        }
    }

    // 继续处理事件
    return false;
}
#endif

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    qDebug() << "main";
    MyWidget1 widget;
    widget.resize(400, 300);
    widget.show();

    return app.exec();
}

#include "main.moc"
