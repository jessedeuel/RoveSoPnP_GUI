#ifndef OPERATORPAGE_H
#define OPERATORPAGE_H

#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QListWidget>
#include <QProgressBar>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QWidget>

class OperatorPage : public QWidget
{
        Q_OBJECT

    public:
        explicit OperatorPage(QWidget* parent = nullptr);
        ~OperatorPage();

    public slots:
        // --- Interface for Vision Backend ---
        // Pass the active OpenCV/Vision frame here to update the GUI
        void updateCameraFrame(const QImage& image, const QString& activeCameraName);

        // --- Interface for FlowControl Backend ---
        void setMachineStateToSetup();
        void setMachineStateToRun();
        void updateRunStatus(const QString& currentTask, int progressPercentage);
        void logMessage(const QString& msg);

        // Trigger this from flow control when it runs out of tape
        void triggerTapeSwapRequired(const QString& componentName, const QString& packageType);

    signals:
        // --- Signals to FlowControl Backend ---
        void requestJog(const QString& axis, double distance);
        void requestSetFiducial(int fiducialIndex);
        void requestStartJob();
        void requestPauseJob();
        void requestStopJob();
        void requestTapeSwapConfirmation();

    private slots:
        // Internal UI Handlers
        void onStartClicked();
        void onPauseClicked();
        void onStopClicked();
        void onTapeSwapConfirmed();

    private:
        void setupUI();
        void createVisionPanel(QHBoxLayout* mainLayout);
        void createControlPanel(QHBoxLayout* mainLayout);

        QWidget* createSetupPage();
        QWidget* createRunPage();
        QWidget* createTapeSwapPage();
        QGroupBox* createJogPanel();

        // -- UI Elements --

        // Vision
        QLabel* cameraFeedLabel;
        QLabel* cameraStatusLabel;

        // Control Stack
        QStackedWidget* modeStackedWidget;

        enum ModeIndex
        {
            SetupMode    = 0,
            RunMode      = 1,
            TapeSwapMode = 2
        };

        // Run Elements
        QLabel* currentTaskLabel;
        QProgressBar* jobProgressBar;
        QListWidget* runLogList;
        QPushButton* pauseBtn;

        // Tape Swap Elements
        QLabel* tapeSwapInstructionsLabel;

        // Jog Elements
        QGroupBox* jogGroupBox;
        QDoubleSpinBox* jogStepSpinBox;
};

#endif    // OPERATORPAGE_H
