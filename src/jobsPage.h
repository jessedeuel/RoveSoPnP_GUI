#pragma once

#include <QComboBox>
#include <QDebug>
#include <QDoubleValidator>
#include <QFileDialog>
#include <QFontMetrics>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QTableWidget>
#include <QTextEdit>
#include <QWidget>

#include "board.hpp"
#include <map>
#include <string>

class PositiveFloatDelegate : public QStyledItemDelegate
{
    public:
        explicit PositiveFloatDelegate(QObject* parent = nullptr) : QStyledItemDelegate(parent) {}

        QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
        {
            QLineEdit* editor           = new QLineEdit(parent);

            QDoubleValidator* validator = new QDoubleValidator(editor);
            validator->setBottom(0.0);
            validator->setNotation(QDoubleValidator::StandardNotation);
            editor->setValidator(validator);
            return editor;
        }
};

class jobsPage : public QWidget
{
        Q_OBJECT

        enum
        {
            AMOUNT,
            VALUE,
            PACKAGE,
            PITCH,
            WIDTH,
            ORIENT,
            REMOVE,
            COUNT
        };

    public:
        jobsPage(std::shared_ptr<Components> board_instance, QWidget* parent = nullptr);
        ~jobsPage();

    private:
        // Private qt widgets
        QGridLayout* m_pLayout;
        QTableWidget* m_pTable;
        QPushButton* m_pUploadJobButton;
        QPushButton* m_pUpdateComponentsButton;
        QTextEdit* m_pCurrentJobTextEdit;
        QLabel* m_pPlacementMapSize;
        QLabel* m_pLostMapSize;
        QLabel* m_pCuttapeImage;
        QLabel* m_pEmptyLabel;

        // Private information variables
        std::string m_sJobFilePath;
        std::shared_ptr<Components> m_pBoard_instance;

    private:
        void onUploadJobButtonClicked();
        void onUpdateComponentsButtonClicked();
        void updatePage();
};
