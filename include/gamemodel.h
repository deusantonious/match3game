#pragma once

#include <QAbstractListModel>
#include <QRandomGenerator>
#include <QFile>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <exception>
#include <utility>

class GameModel : public QAbstractListModel
{
    Q_OBJECT
signals:
    void selectedItemIdChanged();

public:
    Q_PROPERTY(int gameFieldWidth READ getGameFieldWidth CONSTANT)
    Q_PROPERTY(int gameFieldHeight READ getGameFieldHeight CONSTANT)
    Q_PROPERTY(int selectedItemId READ getSelectedItemId CONSTANT)

    GameModel(QObject* parent = nullptr, QString settingsFileName = ":\settings.json");

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = GameModel::ItemColor) const override;
    QHash <int, QByteArray> roleNames() const override;

    enum {
        ItemColor = Qt::UserRole + 1,
        IsSelected,
        BallIsVisible
    };

    int getGameFieldWidth() const;
    int getGameFieldHeight() const;
    int getSelectedItemId() const;

    Q_INVOKABLE void gameFieldReset();
    Q_INVOKABLE void selectItem(int index);
    Q_INVOKABLE bool selectedItemBordersWith(int index);
    Q_INVOKABLE bool swapSelectedItemWith(int index);
    Q_INVOKABLE bool makeAllCoincidenceInvisible();
    Q_INVOKABLE bool moveToFloor();
    Q_INVOKABLE bool gameIsLost();
    Q_INVOKABLE bool gameIsWon();

    struct Ball {
       QString color;
       bool visible;

       Ball ()
       {
           color = "black";
           visible = true;
       }

       Ball (QString color)
           :color {color}
       {
         visible = true;
       }
    };


private: // methods
    bool removeAviableIfSwapRows(std::pair<int, int> oldPosition, std::pair<int, int> newPosition);
    bool removeAviableIfSwapColumns(std::pair<int, int> oldPosition, std::pair<int, int> newPosition);
    bool anySwapAviable();
    std::pair <int,int> get2dPosition(int index);
    int getIndexFrom2dPosition(std::pair <int,int> position);
    int getIndexFrom2dPosition(int row, int column);
    QJsonObject readFromJsonFile(QString settingsFileName);
private: // vars
    int m_gameFieldWidth;
    int m_gameFieldHeight;
    int m_size;
    std::pair<int,int> m_selectedItem;
    QVector<QVector<Ball>> m_gameField;
    QVector<QString> m_colors;
};

