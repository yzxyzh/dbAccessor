#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "tixml/tinyxml.h"
#include <string>
#include <QMenu>
#include <QAction>

using namespace std;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    
    class dbAtom{
      
    public:
        string path;
        string segPath;
        int phase;
        int quality;
        int tumorQuality;
        int isError;
        int layer;
        string description;

        void PrintSelf(std::ostream& os)
        {
            os<<"path = "<<path<<endl;
            os<<"segPath ="<<segPath<<endl;
            os<<"phase = "<<phase<<endl;
            os<<"quality = "<<quality<<endl;
            os<<"tumorQuality = "<<tumorQuality<<endl;
            os<<"isError = "<<isError<<endl;
            os<<"layer = "<<layer<<endl;
            os<<"description = "<<description<<endl;
        }
        
    };
    
    typedef vector<dbAtom> dbElems;
    
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:
    //filters;
    int phasefilter;
    int qualityfilter;
    int tumorFilter;
    int segErrorFilter;
    int layerFilter;
    QString tagFilter;
    
    QMenu*                rmbMenu;
    QAction*              dcmFinderAction;
    QAction*              mhdFinderAction;
    
    dbElems dataBase;
    
    void                  ShowError(QString str);
    
    void                  ReviewInFinder(QWidget* parent, QString& pathIn);
    
    bool                  JudgeFilter(dbAtom& atm);
    
    void                  Encode(const string& input,string& output);
    
    string                int2QStr(int a);
protected slots:
    
    void Reset();
    void SetupDB();
    
    //update db filters
    void UpdateFilters(int);
    
    void ReadDBFromFile();
    
    void DummyReceiver();
    
    void ctmExec(const QPoint& p);
    
    void ShowDcmInFinder();
    
    void ShowMhdInFinder();
    
    void OutputPath();
    
private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
