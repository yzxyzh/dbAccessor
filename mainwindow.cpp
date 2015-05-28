#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

#include <QMessageBox>

#include <QString>
#include <sstream>

#include <QProcess>
#include <QMenu>
#include <fstream>

void MainWindow::ReviewInFinder(QWidget *parent, QString &pathIn)
{
    // Mac, Windows support folder or file.
#if defined(Q_OS_WIN)
    const QString explorer = Environment::systemEnvironment().searchInPath(QLatin1String("explorer.exe"));
    if (explorer.isEmpty()) {
        QMessageBox::warning(parent,
                             tr("Launching Windows Explorer failed"),
                             tr("Could not find explorer.exe in path to launch Windows Explorer."));
        return;
    }
    QString param;
    if (!QFileInfo(pathIn).isDir())
        param = QLatin1String("/select,");
    param += QDir::toNativeSeparators(pathIn);
    QString command = explorer + " " + param;
    QProcess::startDetached(command);
#elif defined(Q_OS_MAC)
    Q_UNUSED(parent)
    QStringList scriptArgs;
    scriptArgs << QLatin1String("-e")
    << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
    .arg(pathIn);
    QProcess::execute(QLatin1String("/usr/bin/osascript"), scriptArgs);
    scriptArgs.clear();
    scriptArgs << QLatin1String("-e")
    << QLatin1String("tell application \"Finder\" to activate");
    QProcess::execute("/usr/bin/osascript", scriptArgs);
#else
    // we cannot select a file here, because no file browser really supports it...
    const QFileInfo fileInfo(pathIn);
    const QString folder = fileInfo.absoluteFilePath();
    const QString app = Utils::UnixUtils::fileBrowser(Core::ICore::instance()->settings());
    QProcess browserProc;
    const QString browserArgs = Utils::UnixUtils::substituteFileBrowserParameters(app, folder);
    if (debug)
        qDebug() <<  browserArgs;
    bool success = browserProc.startDetached(browserArgs);
    const QString error = QString::fromLocal8Bit(browserProc.readAllStandardError());
    success = success && error.isEmpty();
    if (!success)
        showGraphicalShellError(parent, app, error);
#endif
}

void MainWindow::ctmExec(const QPoint &p)
{
    QTableWidgetItem *item = ui->databaseWidget->itemAt(p);
    if(!item)
        return;
    
    this->rmbMenu->exec(QCursor::pos());
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    ui->phaseAtery->setChecked(true);
    ui->phaseEquil->setChecked(true);
    ui->phaseNonEnhance->setChecked(true);
    ui->phaseHepaticPortal->setChecked(true);

    ui->thickLess100->setChecked(true);
    ui->thick100to200->setChecked(true);
    ui->thick200to400->setChecked(true);
    ui->thickMore400->setChecked(true);
    
    ui->tumorNo->setChecked(true);
    ui->tumorMinor->setChecked(true);
    ui->tumorCritical->setChecked(true);
    
    ui->qualityPoor->setChecked(true);
    ui->qualityGood->setChecked(true);
    ui->qualityNormal->setChecked(true);
    
    ui->segOK->setChecked(true);
    ui->segError->setChecked(true);
    
    rmbMenu = new QMenu(this);
    dcmFinderAction = new QAction(this);
    dcmFinderAction->setText("在Finder中显示Dicom路径");
    mhdFinderAction = new QAction(this);
    mhdFinderAction->setText("在Finder中显示mhd路径");
    
    rmbMenu->addAction(dcmFinderAction);
    rmbMenu->addAction(mhdFinderAction);
    
    ui->databaseWidget->setContextMenuPolicy(Qt::CustomContextMenu);
    
    ui->outputNameEdit->setText("点此输入路径集名");
    
    connect(ui->loadDBButton, SIGNAL(clicked()), this, SLOT(ReadDBFromFile()));
    connect(ui->phaseAtery, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->phaseEquil, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->phaseNonEnhance, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->phaseHepaticPortal, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->thickLess100, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->thick200to400, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->thickMore400, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->thick100to200, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->tumorNo, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->tumorMinor, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->tumorCritical, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->qualityNormal, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->qualityGood, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->qualityPoor, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->segError, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    connect(ui->segOK, SIGNAL(stateChanged(int)), this, SLOT(UpdateFilters(int)));
    
    connect(ui->userdefFilter, SIGNAL(textChanged()), this, SLOT(DummyReceiver()));
    
    connect(ui->databaseWidget, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ctmExec(const QPoint &)));
    
    connect(dcmFinderAction, SIGNAL(triggered()), this, SLOT(ShowDcmInFinder()));
    connect(mhdFinderAction, SIGNAL(triggered()), this, SLOT(ShowMhdInFinder()));
    
    connect(ui->savePathButton, SIGNAL(clicked()), this, SLOT(OutputPath()));
    
    Reset();
    
    SetupDB();

}

void MainWindow::OutputPath()
{
    string baseName = ui->outputNameEdit->toPlainText().toStdString();
    string dcmOutName = baseName+"原始数据路径集.txt";
    string mhdOutName = baseName+"肝脏分割结果路径集.txt";
    
    string dcmSetName = baseName+"原始数据路径集";
    string mhdSetName = baseName+"肝脏分割结果路径集";
    
    
    std::ofstream dcmFS,mhdFS;
    
    dcmFS.open(dcmOutName,ios::out|ios::binary);
    mhdFS.open(mhdOutName,ios::out|ios::binary);
    
    //下面开始写
    dcmFS<<dcmSetName<<" ";
    mhdFS<<mhdSetName<<" ";
    bool isAbsolutePath = true;
    dcmFS<<isAbsolutePath<<" ";
    isAbsolutePath = false;
    mhdFS<<isAbsolutePath<<" ";
    size_t sz = ui->databaseWidget->rowCount();
    dcmFS<<sz<<" ";
    mhdFS<<sz<<" ";
    
    //下面开始写正式文件
    for (int i=0; i<sz; i++) {
        
        QTableWidgetItem* itm = ui->databaseWidget->item(i, 0);
        int rowSelected = itm->row();
        int index = ui->databaseWidget->item(rowSelected, 0)->text().toInt();
        
        string dcmPath = dataBase[index].path;
        string mhdPath = dataBase[index].segPath;
        
        string encodedDCMPath,encodedmhdPath;
        Encode(dcmPath, encodedDCMPath);
        Encode(mhdPath, encodedmhdPath);
        
        dcmFS<<encodedDCMPath<<" ";
        mhdFS<<encodedmhdPath<<" ";

    }
    
    dcmFS.close();
    mhdFS.close();
    
    ShowError("写入路径完毕");
}

void MainWindow::DummyReceiver()
{
    UpdateFilters(0);
}

void MainWindow::Reset()
{
    phasefilter = 0;
    qualityfilter = 0;
    tumorFilter = 0;
    segErrorFilter = 0;
    layerFilter = 0;
    tagFilter = "";
}

void MainWindow::ShowError(QString str)
{
    QMessageBox msgBox;
    msgBox.setText(str);
    msgBox.exec();
}

void MainWindow::Encode(const string &input, string &output)
{
    output = input;
    std::replace(output.begin(), output.end(),' ', '@');
}

void MainWindow::ReadDBFromFile()
{
    //读入数据
    QString filename = QFileDialog::getOpenFileName(this,"open database","","database (db.xml)");
    
    string filenameStr = filename.toStdString();
    
    if("" == filenameStr) return;

    cout<<"filename = "<<filenameStr<<endl;

    TiXmlDocument doc;
    
    if(!doc.LoadFile(filenameStr))
    {
        ShowError("读取指定数据库失败。");
        return;
    }
    
    
    //清空已有的数据
    dataBase.clear();
    Reset();
    
    TiXmlElement* firstElem = doc.FirstChildElement()->FirstChildElement();
    while (NULL!=firstElem) {
        
        TiXmlElement* pathElem = firstElem->FirstChildElement();
        TiXmlElement* segPathElem = pathElem->NextSiblingElement();
        TiXmlElement* phaseElem = segPathElem->NextSiblingElement();
        TiXmlElement* qualityElem = phaseElem->NextSiblingElement();
        TiXmlElement* tumorQualityElem = qualityElem->NextSiblingElement();
        TiXmlElement* layerElem = tumorQualityElem->NextSiblingElement();
        TiXmlElement* isSegErrorElem = layerElem->NextSiblingElement();
        TiXmlElement* descriptionElem = isSegErrorElem->NextSiblingElement();
        
        //cout<<"description = "<<(string)descriptionElem->Value()<<endl;
        
        dbAtom newAtom;
        newAtom.path = pathElem->GetText();
        newAtom.segPath = segPathElem->GetText();
        newAtom.phase = atoi(phaseElem->GetText());
        newAtom.quality = atoi(qualityElem->GetText());
        newAtom.tumorQuality = atoi(tumorQualityElem->GetText());
        
        int currLayer = atoi(layerElem->GetText());
        
        //newAtom.layer = (currLayer>100)+(currLayer>200)+(currLayer>400);
        
        newAtom.layer = currLayer;
        
        if(0 == strcmp("TRUE", isSegErrorElem->GetText()))
        {
            newAtom.isError = 0;//有错是0，没错是1
        }else{
            newAtom.isError = 1;
        }
        
        if(NULL != descriptionElem->FirstChild())
            newAtom.description = descriptionElem->GetText();
        else
            newAtom.description = "无描述";
        
        //newAtom.PrintSelf(std::cout);
        
        this->dataBase.push_back(newAtom);
        
        firstElem = firstElem->NextSiblingElement();
    }
    
    
    UpdateFilters(0);
    //SetupDB();
}

string MainWindow::int2QStr(int a)
{
    stringstream ss;
    ss<<a;
    string str;
    ss>>str;
    
    return str;
}

void MainWindow::SetupDB()
{
    //clear database
    while (ui->databaseWidget->rowCount()>0) {
        ui->databaseWidget->removeRow(0);
    }
    
    QSize dbSize = ui->databaseWidget->size();
    ui->databaseWidget->setColumnWidth(0, dbSize.width()/5);
    ui->databaseWidget->setColumnWidth(1, dbSize.width()/5);
    ui->databaseWidget->setColumnWidth(2, dbSize.width()/2);
    
    int nCounter = 0;
    for (int i=0; i<dataBase.size(); i++) {
        
        bool isMatch = JudgeFilter(dataBase[i]);
        
        if(isMatch)
        {

            //cout<<"layer = "<<int2QStr(dataBase[i].layer).toStdString()<<endl;
            //int rNum = ui->databaseWidget->rowCount();
            ui->databaseWidget->insertRow(nCounter);
            string str = int2QStr(i);
            string str1 = int2QStr(dataBase[i].layer);
            QTableWidgetItem* itm0 = new QTableWidgetItem(QString::fromStdString(str));
            QTableWidgetItem* itm1 = new QTableWidgetItem(QString::fromStdString(str1));
            QTableWidgetItem* itm2 = new QTableWidgetItem(QString::fromStdString(dataBase[i].description));
            itm0->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
            itm1->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
            itm2->setFlags(Qt::ItemIsEnabled|Qt::ItemIsSelectable);
            
            ui->databaseWidget->setItem(nCounter, 0, itm0);
            ui->databaseWidget->setItem(nCounter, 1, itm1);
            ui->databaseWidget->setItem(nCounter, 2, itm2);
            
            ui->databaseWidget->setRowHeight(nCounter, 70);
            
            nCounter++;
        }
        
    }
    
}

void MainWindow::ShowDcmInFinder()
{
    QTableWidgetItem* itm = ui->databaseWidget->selectedItems()[0];
    int rowSelected = itm->row();
    int index = ui->databaseWidget->item(rowSelected, 0)->text().toInt();
    QString dcmPath = QString::fromStdString( dataBase[index].path );
    
    cout<<"dcmPath = "<<dcmPath.toStdString()<<endl;
    
    ReviewInFinder(this, dcmPath);
}

void MainWindow::ShowMhdInFinder()
{
    QTableWidgetItem* itm = ui->databaseWidget->selectedItems()[0];
    int rowSelected = itm->row();
    int index = ui->databaseWidget->item(rowSelected, 0)->text().toInt();
    QString dcmPath = QString::fromStdString( dataBase[index].segPath );
    
    cout<<"mhdPath = "<<dcmPath.toStdString()<<endl;
    
    ReviewInFinder(this, dcmPath);
}

bool MainWindow::JudgeFilter(MainWindow::dbAtom &atm)
{
    bool result = false;
    
    int currLayer = atm.layer;
    
    int layerJudge = (currLayer>100)+(currLayer>200)+(currLayer>400);
    
    result = (1<<atm.phase == (1<<atm.phase & phasefilter))
           && (1<<atm.quality == (1<<atm.quality & qualityfilter))
           && (1<<atm.tumorQuality == (1<<atm.tumorQuality & tumorFilter))
           && (1<<atm.isError == (1<<atm.isError & segErrorFilter))
           && (1<<layerJudge == (1<<layerJudge & layerFilter));
    
    QString dName = QString::fromStdString(atm.description);
    
    result &= ( (bool)dName.contains(tagFilter) || tagFilter == "");
    
    return result;
}

void MainWindow::UpdateFilters(int)
{
    
    phasefilter = ((ui->phaseAtery->isChecked())<<0)
                + ((ui->phaseHepaticPortal->isChecked())<<1)
                + ((ui->phaseEquil->isChecked())<<2)
                + ((ui->phaseNonEnhance->isChecked())<<3);
    
    qualityfilter = ((ui->qualityPoor->isChecked())<<0)
                  + ((ui->qualityNormal->isChecked())<<1)
                  + ((ui->qualityGood->isChecked())<<2);
    
    tumorFilter = ((ui->tumorNo->isChecked())<<0)
                + ((ui->tumorMinor->isChecked())<<1)
                + ((ui->tumorCritical->isChecked())<<2);
    
    segErrorFilter = ((ui->segError->isChecked())<<0)+((ui->segOK->isChecked())<<1);
    
    layerFilter = ((ui->thickLess100->isChecked())<<0)
                + ((ui->thick100to200->isChecked())<<1)
                + ((ui->thick200to400->isChecked())<<2)
                + ((ui->thickMore400->isChecked())<<3);
    
    tagFilter = ui->userdefFilter->toPlainText();
    
    SetupDB();
    
}

MainWindow::~MainWindow()
{
    delete ui;
    if(rmbMenu) delete rmbMenu;
    if(dcmFinderAction) delete dcmFinderAction;
    if(mhdFinderAction) delete mhdFinderAction;
    
}
