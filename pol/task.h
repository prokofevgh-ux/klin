#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QDate>
#include <QDateTime>
#include <QJsonObject>
#include <QColor>

enum class Priority {
    Low = 0,
    Medium = 1,
    High = 2
};

enum class TaskStatus {
    Pending = 0,
    Completed = 1
};

class Task {
public:
    Task();
    Task(const QString& title, const QString& description, const QDate& deadline,
         Priority priority = Priority::Medium, const QString& category = "");

    // Геттеры
    int getId() const { return id; }
    QString getTitle() const { return title; }
    QString getDescription() const { return description; }
    QDate getDeadline() const { return deadline; }
    Priority getPriority() const { return priority; }
    QString getCategory() const { return category; }
    TaskStatus getStatus() const { return status; }
    QDateTime getCreatedAt() const { return createdAt; }
    QDateTime getCompletedAt() const { return completedAt; }

    // Сеттеры
    void setTitle(const QString& title) { this->title = title; }
    void setDescription(const QString& description) { this->description = description; }
    void setDeadline(const QDate& deadline) { this->deadline = deadline; }
    void setPriority(Priority priority) { this->priority = priority; }
    void setCategory(const QString& category) { this->category = category; }
    void setStatus(TaskStatus status);
    void setId(int id) { this->id = id; }

    // Утилиты
    bool isOverdue() const;
    bool isDueToday() const;
    bool isDueThisWeek() const;
    QString priorityToString() const;
    QString statusToString() const;
    QColor priorityColor() const;

    // JSON сериализация
    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject& json);

private:
    int id;
    QString title;
    QString description;
    QDate deadline;
    Priority priority;
    QString category;
    TaskStatus status;
    QDateTime createdAt;
    QDateTime completedAt;

    static int nextId;
};

#endif // TASK_H

