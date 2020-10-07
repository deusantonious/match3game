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

    int getGameFieldWidth() const;
    int getGameFieldHeight() const;
    int getSelectedItemId() const;

    Q_INVOKABLE void deleteAllEmptyRowsAndColumns();
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

    enum {
        ItemColor = Qt::UserRole + 1,
        IsSelected,
        BallIsVisible
    };
    enum direction{
        UP,
        DOWN,
        UPDOWN,
        LEFT,
        RIGHT,
        LEFTRIGHT
    };

private: // methods
    bool removeAviableIfSwap(std::pair<int, int> oldPosition, std::pair<int, int> sourceElement, QVector<int> directions);
    bool anySwapAviable();

    int getIndexFrom2dPosition(std::pair <int,int> position);
    int getIndexFrom2dPosition(int row, int column);

    void swapItems(std::pair<int, int> firstElementPos, std::pair<int, int> secondElementPos);

    QJsonObject readFromJsonFile(QString settingsFileName);

    std::pair <int,int> findLastElement(std::pair <int,int> startPosition, int deltaI, int deltaJ, QString color);
    std::pair <int,int> get2dPosition(int index);
private: // vars
    int m_gameFieldWidth;
    int m_gameFieldHeight;
    int m_size;

    std::pair<int,int> m_selectedItem;

    QVector<QVector<Ball>> m_gameField;
    QVector<QString> m_colors;
};

