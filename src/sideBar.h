#pragma once

#include <QDebug>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QPushButton>

#include <memory>

class FlowControl;    // Forward declaration

class sideBar : public QWidget
{
        Q_OBJECT

    public:
        sideBar(QWidget* parent = nullptr);
        ~sideBar();

        // Bind the sidebar to FlowControl to receive live updates
        void bindFlowControl(FlowControl* fc);

    private:
        QGridLayout* m_pSideBarLayout;
        QLabel* m_pStateLabel;
        QLabel* m_pConnectionStatusLabel;
        QLabel* m_pPositionLabel;
        QLabel* m_pCurrentComponentLabel;
        QLabel* m_pCurrentJobLabel;
        QPushButton* m_pPauseButton;
        QPushButton* m_pStartEndProgramButton;

        FlowControl* m_fc = nullptr;    // Reference to backend

    private slots:
        void onPauseButtonClicked();
        void onStartEndProgramButtonClicked();

        // Handlers for incoming live data
        void updateState(const QString& stateStr);
        void updatePosition(float x, float y, float z);
};
