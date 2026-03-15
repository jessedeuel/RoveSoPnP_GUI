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
    // Private qt widgets
    QGridLayout *m_pJobsPageLayout;
    QTableWidget *m_pJobsTable;
    QPushButton *m_pUploadJobButton;
    QTextEdit *m_pCurrentJobTextEdit;

    // Private information variables
    std::string m_sJobFilePath;

private slots:
    void onUploadJobButtonClicked();
};