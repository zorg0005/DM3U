#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include "QMenu"
#include "QCursor"
#include "QFile"
#include "QMessageBox"
#include "QRegExp"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include "QFileDialog"
#include "QTextCodec"
#include <QStandardItemModel>
#include <QStackedWidget>
#include <QtWidgets>
#include <QShortcut>

#include "channels.h"
#include "groups.h"


#define NORMALMODE 0
#define EPGMODE 1
#define SHOWBYSOURCES 2
extern int tablemode;

namespace Ui {
class MainWindow;
}

class FindDialog : public QDialog
{
    Q_OBJECT
public:
    FindDialog(QWidget *parent = 0, const char *name = 0);
signals:
    void findNext(const QString &str, bool caseSensitive);
private slots:
    //void findClicked();
    //void enableFindButton(const QString &text);
private:
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *findButton;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QString filename="";   // обрабатываемый первичный плэйлист

    /// TODO виртуальные колонки таблицы, модифицируются из ini
    //int colNst=0, wthNst=30; // если значение wth -1 значит колонка невидимая
    int colCN=0, wthCN=150; // название канала (с плэйлиста)
    int colLen=1, wthLen=70; // скорость воспроизведения/(смещение EPG)
    int colGroup=2, wthGroup=64; // группы канала
    int colEPG=3, wthEPG=64; // идентификатор EPG
    int colLogo=4, wthLogo=64; // иконка канала
    int colURL=5, wthURL=64;    // полный URL потока
    int colPrefix=6, wthPrefix=40; // префикс
    int colHTPXa=7, wthHTPXa=90; // адрес прокси
    int colHTPXp=8, wthHTPXp=40; // порт прокси
    int colPostfix=9, wthPostfix=64;  // постфикс
    int colSTRMa=10, wthSTRMa=90; //адрес потока
    int colSTRMp=11, wthSTRMp=40; //порт потока
    int colProv=12, wthProv=70; // провайдер
    int colState=13, wthState=-1; // состояние для сравнения
    int colCCN=14, wthCCN=150; // каноническое название канала (которое пользователь может изменить) // служит как ключ для канала из разных источников EPG
    //int colEPGCN=15, wthEPGCN=150; // название канала с источника EPG (это для таблицы channels)
    int colcnt=14+1; // количество столбцов таблицы
    // для вычисления сдвига середины сплитера при изменении размера
    int GForm1WidthOld;
    QTableWidget table2;
    void setTableM3U(QTableWidget *TW);
    // форма
    QFrame panel;   // информационная панель

    // menu
    QMenu *contextTableMenu, menuEPG, *generalMenu;
    QAction *openFile, *acompareFile, *adelrow, *saveM3U, *asaveM3Uas;
    QAction *aSetChName, *actEpgMode;
    QAction *aChannels;

    // формы
    //Ui::MainWindow *mui;
    Channels *channels;
    FindDialog *findDialog;
    QShortcut  *keyCtrlF;

    Groups *groups;
    bool testGroups();
    Groups::KlassGrp *klsChannels;
    Groups::KlassGrp *klsGroups;
    Groups::GroupList *lstKat, *lstKatm;
    Groups::GroupList *lstChan;

    // функции парсера m3u
    /// TODO выделить отдельный класс M3U, передавать его в параметрах, интерфейс работает с классом и БД
    void openM3U(QTableWidget *TW, QTextStream *stream);
    void prepareExtinf(QTableWidget *TW, QString &buffstr, int trow);   // Парсинг строки #EXT
    void prepareUrl(QTableWidget *TW, QString &buffstr, int trow);      // Парсинг строки источника
    void adjustChkb(int idx); // распределение чекбоксов на панели (each in M3Utemplate)
    struct attrItem {
        QString key; // имя аттрибута
        QString name; // имя колонки/чекбокса
        QCheckBox* chb=nullptr; // ссылка на чекбокс // также признак наличия в шаблоне
        int ncol=-1; // номер колонки
        bool exist=false; // true - принадлежность к текущему шаблону
        //attrItem() {};
    };
    QList<attrItem> attributes;
    //QHash<QString, QCheckBox*> M3Utemplate; // лист определяемого шаблона -> текущий шаблон
    //QHash<QString, QString> M3Uattribs; // список известных параметров (аттрибут - название колонки) - транслятор имён

    // функции парсера EPGID
    //QComboBox *epgcb;   // реплика для формы channels
    void getEPGID(QString source);
    QHash<QString, QString> epgid; // массив каналов с источника EPG <номер, название>
    struct CellWidgetCoord {
        int row=0;
        int column=0;
    } cellwidgetcoord; // место хранения координат виджета с EPG Channels в таблице
    QStandardItemModel epgchModel;
    bool forceLogoDownload=false;
    QComboBox *cb=nullptr;  // текущий виджет выбора из списка в таблице

    // сравнение списков
    void compareM3U();
    void checkDbl(QTableWidget *TW); // проверка дублей
    void savem3ufile(bool as);
    void findInTable(QTableWidget *TW, int col, QString text); /// TODO // поиск (? if col==-1 поиск по всей таблице)
    void setNormalModeTable();
private:
    Ui::MainWindow *ui;

public slots:
    void contextMenuComboBoxEPG();   //
    void setChannelName();   //
    void delrows();
    void sortBySources();
    void showBySources();
    void removeTable2();
    void columnCheck(int);
    void findInTable1(); /// TODO
    void findInTable2(); /// TODO
private slots:
    void openFileSlot();
    void savem3uas();
    void savem3uslot();
    void compareFile();
    void setEpgMode();
    void on_tableWidget_customContextMenuRequested(const QPoint &pos);
    void on_comboBox_4_currentIndexChanged(int index);
    void cb_currentIndexChanged(int idx);
    void comBox_currentIndexChanged(int index);
    void on_tableWidget_cellEntered(int row, int column);
    void on_pushButton_pressed();
    void on_tableWidget_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void on_MainWindow_destroyed();
    virtual void closeEvent(QCloseEvent *event);
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
};


#endif // MAINWINDOW_H
