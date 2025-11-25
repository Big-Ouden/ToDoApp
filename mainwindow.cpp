#include "mainwindow.h"
#include "./ui_mainwindow.h"

/**
 * @brief
 *
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

/**
 * @brief
 *
 */
MainWindow::~MainWindow()
{
    delete ui;
}
