#include "groups.h"
#include "ui_groups.h"

Groups::Groups(QWidget *parent):
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);

    // создаём модели и натравливаем на интерфейсы
    groupklassmodel=new groupClassModel(this,0);
    ui->comboBox_2->setModel(groupklassmodel);
    ui->comboBox_2->setStyle(new ComboBoxProxyStyle("Новый класс..."));
    ui->comboBox_2->installEventFilter(this);
    //installEventFilter(ui->comboBox_2);
}

Groups::~Groups()
{
    delete ui;
    // уничтожаем структуры
    /// критическую секцию?
    // списки групп
    Groups::clear();
}

////// TOOODOOO TODO DComboBox, ya?
bool Groups::event(QEvent *event)
{
    /*if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Key_Tab) {
            insertAtCurrentPosition( \t );
            return true;
        }
    }//*/
    if (event->type()==QEvent::MouseButtonDblClick) {
        QMouseEvent * mouse_event = static_cast<QMouseEvent *>(event);
        if (mouse_event->button()==Qt::LeftButton) {
            //
        }
    }
    return QWidget::event(event);
}

////// TOOODOOO TODO DComboBox, ya?
bool Groups::eventFilter(QObject *target, QEvent *event)
{
    // это фильтр для списка классов
    if (target == ui->comboBox_2) {
        if (event->type()==QEvent::MouseButtonDblClick) {
            QMouseEvent * mouse_event = static_cast<QMouseEvent *>(event);
            if (mouse_event->button()==Qt::LeftButton) {
                ui->comboBox_2->setEditable(true);
                return true;    // accept
            }
        }
        if (event->type()==QEvent::MouseButtonPress) {
            QMouseEvent * mouse_event = static_cast<QMouseEvent *>(event);
            if (mouse_event->button()==Qt::RightButton) {
                // вызов контекстного меню для комбобокса
                QMenu *menu = new QMenu(ui->comboBox_2);
                QAction *a_cbx2=new QAction("Delete",ui->comboBox_2);
                QAction *a_cbx2edit=new QAction("Edit",ui->comboBox_2);
                menu->addAction(a_cbx2edit);
                menu->addAction(a_cbx2);
                connect(a_cbx2,SIGNAL(triggered()), this, SLOT(comboBoxDeleteItem()));
                connect(a_cbx2edit,SIGNAL(triggered()), this, SLOT(comboBoxEditItem()));
                menu->exec(mouse_event->globalPos());
                return true;    // accept
            }
        }
        //
        if (event->type()==QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
            // сброс редактирования
            if (keyEvent->key() == Qt::Key_Escape) {
                emit ui->comboBox_2->currentIndexChanged(ui->comboBox_2->currentIndex());
                //return true;
            }
        }//*/
    }
    return QWidget::eventFilter(target, event);
}

////// TOOODOOO TODO DComboBox, ya?
/*void Groups::on_toolButton_clicked()
{
    ui->comboBox_2->insertItem(1,ui->comboBox_2->lineEdit()->text());
}//*/

////// TOOODOOO TODO DComboBox, ya?
void Groups::comboBoxDeleteItem()
{
    if (sender()->parent()==ui->comboBox_2) {
        //QComboBox *cb=qobject_cast<QComboBox *>(sender());
        //QAction* act = qobject_cast<QAction*>(sender());
        ui->comboBox_2->removeItem(ui->comboBox_2->currentIndex());
    }
}

////// TOOODOOO TODO DComboBox, ya?
void Groups::comboBoxEditItem()
{
    ui->comboBox_2->setEditable(true);
    ui->comboBox_2->setEditText(ui->comboBox_2->currentText());
}

////// TOOODOOO TODO DComboBox, ya?
void Groups::on_comboBox_2_currentIndexChanged(int index)
{
    if (index) index=index; /// TODO заглушка от ворнингов
    ui->comboBox_2->setEditable(false);
}
