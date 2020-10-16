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
    void gameWonChanged();
    void gameLostChanged();

public:
    Q_PROPERTY(int currentColumnCount READ getCurrentColumnCount CONSTANT)
    Q_PROPERTY(int currentRowCount READ getcurrentRowCount CONSTANT)
    Q_PROPERTY(int selectedItemId READ getSelectedItemId CONSTANT)
    Q_PROPERTY(bool gameWon MEMBER m_gameWon NOTIFY gameWonChanged)
    Q_PROPERTY(bool gameLost MEMBER m_gameLost NOTIFY gameLostChanged)

    GameModel(QObject* parent = nullptr, QString settingsFileName = ":\settings.json");

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = GameModel::ItemColor) const override;
    QHash <int, QByteArray> roleNames() const override;

    int getCurrentColumnCount() const;
    int getcurrentRowCount() const;
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
    bool removeAviableIfSwap(std::pair<int, int> newPosition, std::pair<int, int> sourceElement, QVector<int> directions);
    bool anySwapAviable();

    int getIndexFrom2dPosition(std::pair <int,int> position);
    int getIndexFrom2dPosition(int row, int column);

    void swapItems(std::pair<int, int> firstElementPos, std::pair<int, int> secondElementPos);

    QJsonObject readFromJsonFile(QString settingsFileName);

    std::pair <int,int> findLastElement(std::pair <int,int> startPosition, int deltaI, int deltaJ, QString color);
    std::pair <int,int> get2dPosition(int index);
private: // vars
    int m_currentColumnCount;
    int m_currentRowCount;
    int m_size;

    std::pair<int,int> m_selectedItem;

    QVector<QVector<Ball>> m_gameField;
    QVector<QString> m_colors;

    bool m_gameWon;
    bool m_gameLost;
};

