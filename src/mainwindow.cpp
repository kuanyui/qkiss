#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QToolButton>
#include <QString>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setupMainWindow();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupMainWindow()
{
    // File
    m_toolbarFiles = new QToolBar(this);
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarFiles);
    m_toolbarFiles->addAction(ui->action_openFile);
    m_toolbarFiles->addAction(ui->action_openDirectory);
    // Zoom
    m_toolbarZooms = new QToolBar(this);
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarZooms);
    m_cboxZoomScale = new QComboBox(m_toolbarZooms);
    m_cboxZoomScale->addItem("1", QVariant(1));
    m_cboxZoomScale->addItem("2", QVariant(2));
    m_cboxZoomScale->addItem("3", QVariant(3));
    m_cboxZoomScale->addItem("4", QVariant(4));
    m_toolbarZooms->addAction(ui->action_zoomOut);
    m_toolbarZooms->addAction(ui->action_zoomIn);
    m_toolbarZooms->addWidget(m_cboxZoomScale);
    // Dock Window
    m_toolbarWindows = new QToolBar(this);
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarWindows);
    m_toolbarWindows->addAction(ui->action_window_fileSystemBrowser);
    m_toolbarWindows->addAction(ui->action_window_cnfSelector);
    m_toolbarWindows->addAction(ui->action_window_logs);
    // Page Sets
    m_toolbarPageSets = new QToolBar(this);
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarPageSets);
    m_toolbarPageSets->addWidget(new QLabel(tr("Page Set"), m_toolbarPageSets));
    for (int i = 0; i < 9; i++) {
        QToolButton *btn = new QToolButton(m_toolbarPageSets);
        btn->setText(QString::number(i+1));
        btn->setCheckable(true);
        m_toolbarPageSets->addWidget(btn);
    }
    // Color Sets
    m_toolbarColorSets = new QToolBar(this);
    this->addToolBar(Qt::ToolBarArea::TopToolBarArea, m_toolbarColorSets);
    m_toolbarColorSets->addWidget(new QLabel(tr("Color Set"), m_toolbarColorSets));
    for (int i = 0; i < 9; i++) {
        QToolButton *btn = new QToolButton(m_toolbarColorSets);
        btn->setText(QString::number(i+1));
        btn->setCheckable(true);
        m_toolbarColorSets->addWidget(btn);
    }

}

