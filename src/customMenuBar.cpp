#include "customMenuBar.h"

customMenuBar::customMenuBar(QWidget *parent) : QMenuBar(parent)
{
    this->addMenu("File");
    this->addMenu("Edit");
}

customMenuBar::~customMenuBar()
{
    qDebug() << "Destroying customMenuBar...";
}