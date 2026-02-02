#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "task.h"
#include <QList>
#include <QString>
#include <QDate>

class TaskManager {
public:
    TaskManager();
    ~TaskManager();

    // Управление задачами
    void addTask(const Task& task);
    void updateTask(const Task& task);
    void deleteTask(int taskId);
    Task* getTask(int taskId);
    QList<Task> getAllTasks() const { return tasks; }

    // Фильтрация
    QList<Task> getTasksByStatus(TaskStatus status) const;
    QList<Task> getTasksByPriority(Priority priority) const;
    QList<Task> getTasksByCategory(const QString& category) const;
    QList<Task> getOverdueTasks() const;
    QList<Task> getTodayTasks() const;
    QList<Task> getWeekTasks() const;

    // Получение категорий
    QStringList getCategories() const;

    // Сохранение и загрузка
    bool saveToFile(const QString& filename = "tasks.json");
    bool loadFromFile(const QString& filename = "tasks.json");

    // Статистика
    int getCompletedTodayCount() const;
    int getCompletedThisWeekCount() const;
    QMap<QDate, int> getDailyCompletionStats(int days = 30) const;
    QMap<QString, int> getCategoryStats() const;
    QMap<int, int> getPriorityStats() const; // день недели -> количество выполненных

private:
    QList<Task> tasks;
    QString dataFile;

    void ensureDataFile();
};

#endif // TASKMANAGER_H

