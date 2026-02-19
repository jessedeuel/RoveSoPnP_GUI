#pragma once

#include <QWidget>
#include <QGridLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QTextEdit>
#include <QFontMetrics>
#include <QFileDialog>
#include <QDebug>

class jobsPage : public QWidget
{
    Q_OBJECT

public:
    jobsPage(QWidget *parent = nullptr);
    ~jobsPage();

private:
    QGridLayout* m_jobsPageLayout;
    QTableWidget* m_jobsTable;
    QPushButton* m_uploadJobButton;
    QTextEdit* m_currentJobTextEdit;

private slots:
    
};