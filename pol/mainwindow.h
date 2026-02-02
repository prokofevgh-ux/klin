#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QPushButton>
#include <QLineEdit>
#include <QDateEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QTabWidget>
#include <QListWidget>
#include <QProgressBar>
#include <QStackedWidget>
#include "taskmanager.h"
#include "gamestats.h"
#include "englishdata.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTask();
    void onTaskStatusChanged(int row, int column);
    void onDateChanged();
    void refreshGameWidget();
    void onEnglishLevelChanged(int index);
    void onEnglishLessonSelected(int index);
    void onEnglishAddWord();
    void onEnglishRemoveWord();
    void onPrayerGospelChanged(int index);
    void onPrayerChapterSelected(int index);
    void onPrayerAddImage();

private:
    void setupUI();
    void setupTasksTab();
    void setupEnglishTab();
    void setupPrayerTab();
    void loadPrayerImageForChapter();
    QString prayerImagePath(int gospelIndex, int chapterNum) const;
    void updateDailyTasks();
    void showTaskDialog(const Task* task = nullptr);
    QColor getPriorityColor(Priority priority) const;
    QColor getPriorityTextColor(Priority priority) const;
    QString formatDate(const QDate& date) const;
    bool isMobile() const;

    TaskManager* taskManager;
    GameStats gameStats;
    EnglishData englishData;

    // Основные элементы
    QTabWidget* tabWidget;
    QDateEdit* dateSelector;
    QTableWidget* tasksTable;
    QPushButton* addButton;
    QLabel* dateLabel;

    // Геймификация
    QLabel* levelLabel;
    QProgressBar* xpBar;
    QLabel* streakLabel;

    // Английский
    QComboBox* englishLevelCombo;
    QListWidget* englishLessonList;
    QTableWidget* englishWordsTable;
    QPushButton* englishAddWordBtn;
    QPushButton* englishRemoveWordBtn;
    QLineEdit* englishWordEdit;
    QLineEdit* englishTranslationEdit;

    // Молитва
    QComboBox* prayerGospelCombo;
    QListWidget* prayerChapterList;
    QTextEdit* prayerChapterText;
    QLabel* prayerImageLabel;
    QPushButton* prayerAddImageBtn;
};

#endif // MAINWINDOW_H
