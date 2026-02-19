#pragma once

#include <QMenuBar>
#include <QObject>

class customMenuBar : public QMenuBar
{
    Q_OBJECT

public:
    customMenuBar(QWidget *parent = nullptr);
    ~customMenuBar();
};