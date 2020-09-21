#pragma once

#include <QAbstractListModel>
#include <QRandomGenerator>
#include <QFile>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <exception>

class GameModel : public QAbstractListModel
{
    Q_OBJECT
signals:
    void selectedItemIdChanged();

public:
    Q_PROPERTY(int gameFieldWidth READ getGameFieldWidth CONSTANT)
    Q_PROPERTY(int gameFieldHeight READ getGameFieldHeight CONSTANT)
    Q_PROPERTY(int selectedItemId  MEMBER m_selectedItemId NOTIFY selectedItemIdChanged)

    GameModel(QObject* parent = nullptr, QString settingsFileName = ":\\settings.json");

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = GameModel::ItemColor) const override;
    QHash <int, QByteArray> roleNames() const override;

    enum {
        ItemColor = Qt::UserRole + 1,
        IsSelected
    };

    int getGameFieldWidth() const;
    int getGameFieldHeight() const;

    Q_INVOKABLE void gameFieldReset();
    Q_INVOKABLE void selectItem(int index);

private: // methods
    bool swapItems(int newPositionIndex);
    bool removeAviableIfSwapRows(int oldPositionIndex, int newPositionIndex);
    bool removeAviableIfSwapColumns(int oldPositionIndex, int newPositionIndex);
private: // vars
    int m_gameFieldWidth;
    int m_gameFieldHeight;
    int m_size;
    int m_aviableColorsCount;
    int m_selectedItemId;

    QVector<QString> m_gameField;
    QVector<QString> m_colors;
};

