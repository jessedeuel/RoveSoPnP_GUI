#pragma once

#include <QLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

class sideBar : public QWidget
{
    Q_OBJECT

public:
    sideBar(QWidget *parent = nullptr);
    ~sideBar();

private:
    QGridLayout* m_pSideBarLayout;
    QLabel* m_pStateLabel;
    QLabel* m_pConnectionStatusLabel;
    QLabel* m_pPositionLabel;
    QLabel* m_pCurrentComponentLabel;
    QLabel* m_pCurrentJobLabel;
    QPushButton* m_pPauseButton;
    QPushButton* m_pEndProgramButton;

private slots:
    void onPauseButtonClicked();
    void onEndProgramButtonClicked();
};