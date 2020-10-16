#include "include/gamemodel.h"
#include <QDebug>

GameModel::GameModel(QObject *parent, QString settingsFileName) :
    m_selectedItem {-1,-1},
    m_gameWon {false},
    m_gameLost {false}
{
    Q_UNUSED(parent)

    QJsonObject objects {readFromJsonFile(settingsFileName)};

    m_currentColumnCount = objects["width"].toInt();
    m_currentRowCount = objects["height"].toInt();
    if (m_currentColumnCount <= 2 || m_currentRowCount <= 2) { //game always lost
        throw std::exception();
    }

    /* Creating game field array */
    m_size = m_currentRowCount * m_currentColumnCount;
    m_gameField.resize(m_currentRowCount);
    for (int i{0}; i < m_currentRowCount; i++) {
        m_gameField[i].resize(m_currentColumnCount);
    }

    /* Creating aviable colors array */
    QJsonArray aviableColors = objects["colors"].toArray();
    int aviableColorsCount = aviableColors.size();

    if (aviableColorsCount <= 1) {
        throw std::exception();
    }

    for (int i {0}; i < aviableColorsCount; i++) {
        m_colors.push_back(aviableColors.at(i).toString());
    }

    gameFieldReset();
}

int GameModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return  m_size;
}

QVariant GameModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }

    std::pair <int, int> index2d {index.row() / m_currentColumnCount, index.row() % m_currentColumnCount};
    QVariant returnValue {};

    switch (role)
    {
    case ItemColor:
    {
        returnValue = QVariant {m_gameField[index2d.first][index2d.second].color};
        break;
    }
    case IsSelected:
    {
        if (index2d == m_selectedItem) {
            returnValue = QVariant {true};
        }
        else {
            returnValue = QVariant {false};
        }
        break;
    }
    case BallIsVisible:
    {
        returnValue = QVariant {m_gameField[index2d.first][index2d.second].visible};
        break;
}
}
return returnValue;
}

QHash<int, QByteArray> GameModel::roleNames() const
{
    QHash <int, QByteArray> roles;

    roles[ItemColor] = "itemColor";
    roles[IsSelected] = "isSelected";
    roles[BallIsVisible] = "ballIsVisible";

    return roles;
}

int GameModel::getCurrentColumnCount() const
{
    return m_currentColumnCount;
}

int GameModel::getcurrentRowCount() const
{
    return m_currentRowCount;
}

void GameModel::gameFieldReset()
{
    int generatedColorId {0};
    int aviableColorsCount {m_colors.size()};

    beginResetModel();
    m_selectedItem = std::pair <int,int> {-1, -1};
    for (int i {0}; i < m_currentRowCount; i++) {
        for (int j {0}; j < m_currentColumnCount; j++)  {
            while (1) {
                generatedColorId = QRandomGenerator::global()->bounded(aviableColorsCount);
                if (i > 1 &&
                        m_gameField[i - 1][j].color == m_colors[generatedColorId] &&
                        m_gameField[i - 2][j].color == m_colors[generatedColorId]) {
                    continue;
                }
                if (j > 1 &&
                        m_gameField[i][j - 1].color == m_colors[generatedColorId] &&
                        m_gameField[i][j - 1].color == m_colors[generatedColorId]) {
                    continue;
                }
                break;
            }
            m_gameField[i][j] = m_colors[generatedColorId];
            m_gameField[i][j].visible = true;
        }
    }
    endResetModel();
}

void GameModel::selectItem(int index)
{
    std::pair<int, int> position {get2dPosition(index)};
    int oldSelectedId {getSelectedItemId()};

    if (m_gameField[position.first][position.second].visible) {
        m_selectedItem = get2dPosition(index);
        if (oldSelectedId > getSelectedItemId()) {
            emit dataChanged(createIndex(getSelectedItemId(), 0), createIndex(oldSelectedId, 0), QVector<int> {IsSelected});
        }
        else {
            emit dataChanged(createIndex(oldSelectedId, 0), createIndex(getSelectedItemId(), 0), QVector<int> {IsSelected});
        }
    }
}

bool GameModel::selectedItemBordersWith(int index)
{
    std::pair <int,int> newPosition = get2dPosition(index);
    bool returnValue {false};

    if (newPosition.first == m_selectedItem.first &&
            abs(newPosition.second - m_selectedItem.second) == 1) {
        returnValue =  true;
    }
    else if (newPosition.second == m_selectedItem.second &&
             abs(newPosition.first - m_selectedItem.first) == 1) {
        returnValue = true;
    }
    return returnValue;
}

bool GameModel::swapSelectedItemWith(int index)
{
    bool swapIsAviable;
    std::pair <int, int> oldPosition {m_selectedItem};
    std::pair <int, int> newPosition {get2dPosition(index)};

    if (m_gameField[newPosition.first][newPosition.second].visible) { //we cant swap with empty item
        if (index < getSelectedItemId()) {
            std::swap(oldPosition,newPosition);
        }

        if (newPosition.first - oldPosition.first == 1) { // if we need to swap rows
            swapIsAviable = removeAviableIfSwap(oldPosition, newPosition, QVector<int> {UP, LEFTRIGHT});
            if (!swapIsAviable) {
                swapIsAviable = removeAviableIfSwap(newPosition, oldPosition, QVector<int> {DOWN, LEFTRIGHT});
            }
        }
        else { // if we need to swap columns
            swapIsAviable = removeAviableIfSwap(oldPosition, newPosition, QVector<int> {UPDOWN, LEFT});
            if (!swapIsAviable) {
                swapIsAviable = removeAviableIfSwap(newPosition, oldPosition, QVector<int> {UPDOWN, RIGHT});
            }
        }

        if (swapIsAviable) {
            swapItems(oldPosition, newPosition);
            return true;
        }
    }
    return false;
}

bool GameModel::makeAllCoincidenceInvisible()
{
    int rightLast;
    int downLast;
    QVector <std::pair<int, int>> ItemsToDelete {};
    for (int i {0}; i < m_currentRowCount; i++) {
        for (int j {0}; j < m_currentColumnCount; j++) {
            rightLast = findLastElement(std::pair<int, int> {i, j}, 0, 1, m_gameField[i][j].color).second;
            if (rightLast - j >= 2) {
                for (int k {j}; k <= rightLast; k++) {
                    ItemsToDelete.push_back(std::pair<int, int> {i,k});
                }
            }
            downLast = findLastElement(std::pair<int, int> {i, j}, 1, 0, m_gameField[i][j].color).first;
            if (downLast - i >= 2) {
                for (int k {i}; k <= downLast; k++) {
                    ItemsToDelete.push_back(std::pair<int, int> {k, j});
                }
            }
        }
    }
    if (!ItemsToDelete.empty()) {
        for (auto i: ItemsToDelete) {
          m_gameField[i.first][i.second].visible = false;
        }
        emit dataChanged(createIndex(0, 0), createIndex(rowCount(), 0));
        return true;
    }
    return false;
}

int GameModel::getSelectedItemId() const
{
    return m_selectedItem.first != -1 ?
                m_selectedItem.first * m_currentColumnCount + m_selectedItem.second :
                -1;
}

void GameModel::deleteAllEmptyRowsAndColumns()
{
    int i;
    for (int j {0}; j < m_currentColumnCount; j++) {
        i = 0;
        while (i < m_currentRowCount && !m_gameField[i][j].visible) {
            i++;
        }
        if (i == m_currentRowCount) {
            for (int k {0}; k < m_currentRowCount; k++) {
                for (int p {j}; p < m_currentColumnCount - 1; p++) {
                    swapItems(std::pair<int, int> {k, p}, std::pair<int, int> {k, p + 1});
                }
            }
        }
    }
}

bool GameModel::removeAviableIfSwap(std::pair<int, int> newPosition, std::pair<int, int> sourceElement, QVector<int> directions)
{
    if(!m_gameField[sourceElement.first][sourceElement.second].visible ||
           !m_gameField[newPosition.first][newPosition.second].visible ) {
        return false;
    }
    if (directions.indexOf(UP) != -1 && newPosition.first >= 2) {
        std::pair<int, int> upperLast = findLastElement(std::pair<int, int> {newPosition.first - 1, newPosition.second}, -1, 0, m_gameField[sourceElement.first][sourceElement.second].color);
        if (newPosition.first - upperLast.first >= 2) {
            return true;
        }
    }
    if (directions.indexOf(DOWN) != -1 && newPosition.first <= m_currentRowCount - 3) {
        std::pair<int, int> downLast = findLastElement(std::pair<int, int> {newPosition.first + 1, newPosition.second}, 1, 0, m_gameField[sourceElement.first][sourceElement.second].color);
        if (downLast.first - newPosition.first >= 2) {
            return true;
        }
    }
    if (directions.indexOf(UPDOWN) != -1) {
        int topElement {newPosition.first};
        int downElement {newPosition.first};

        if (newPosition.first > 0)  {
            topElement = findLastElement(std::pair<int, int> {newPosition.first - 1, newPosition.second}, -1, 0, m_gameField[sourceElement.first][sourceElement.second].color).first;
        }
        if (newPosition.second < m_currentRowCount - 1) {
            downElement = findLastElement(std::pair<int, int> {newPosition.first + 1, newPosition.second}, 1, 0, m_gameField[sourceElement.first][sourceElement.second].color).first;
        }
        if (downElement - topElement >= 2) {
            return true;
        }
    }
    if (directions.indexOf(LEFT) != -1 && newPosition.second >= 2) {
        std::pair<int, int> leftLast = findLastElement(std::pair<int, int> {newPosition.first, newPosition.second - 1}, 0, -1, m_gameField[sourceElement.first][sourceElement.second].color);
        if (newPosition.second - leftLast.second >= 2) {
            return true;
        }
    }
    if (directions.indexOf(RIGHT) != -1 && newPosition.second <= m_currentColumnCount - 3) {
        std::pair<int, int> rightLast = findLastElement(std::pair<int, int> {newPosition.first, newPosition.second + 1}, 0, 1, m_gameField[sourceElement.first][sourceElement.second].color);
        if (rightLast.second - newPosition.second >= 2) {
            return true;
        }
    }
    if (directions.indexOf(LEFTRIGHT) != -1) {
        int leftElement {newPosition.second};
        int rightElement {newPosition.second};
        if (newPosition.second > 0)  {
            leftElement = findLastElement(std::pair<int, int> {newPosition.first, newPosition.second - 1}, 0, -1, m_gameField[sourceElement.first][sourceElement.second].color).second;
        }
        if (newPosition.second < m_currentColumnCount - 1) {
            rightElement = findLastElement(std::pair<int, int> {newPosition.first, newPosition.second + 1}, 0, 1, m_gameField[sourceElement.first][sourceElement.second].color).second;
        }
        if (rightElement - leftElement >= 2) {
            return true;
        }
    }
    return false;
}

bool GameModel::moveToFloor()
{
    int k {};
    bool itemsMoved {false};
    for (int j {0}; j < m_currentColumnCount; j++) {
        for (int i {m_currentRowCount - 1}; i > 0; i--) {
            if (m_gameField[i][j].visible) {
                continue;
            }
            k = i;
            /* looking for first visible item in column */
            while (k > 0 && !m_gameField[k][j].visible) {
                k--;
            }

            swapItems(std::pair<int, int> {k,j}, std::pair<int, int> {i,j});
            itemsMoved = true;
        }
    }

    if (itemsMoved) {
        return true;
    }
    return false;
}

bool GameModel::gameIsLost()
{
    for (int i {0}; i < m_currentRowCount; i++) {
        for (int j {0}; j < m_currentColumnCount; j++) {
            if (!m_gameField[i][j].visible) {
                continue;
            }

            if (j > 0 && removeAviableIfSwap(std::pair<int, int> {i, j}, std::pair<int, int> {i, j - 1}, QVector<int> {UPDOWN, RIGHT})) {
                return false;
            }
            if (i > 0 && removeAviableIfSwap(std::pair<int, int> {i, j}, std::pair<int, int> {i - 1, j}, QVector<int> {DOWN, LEFTRIGHT})) {
                return false;
            }
            if (j < m_currentColumnCount - 1 &&
                    removeAviableIfSwap(std::pair<int, int> {i, j}, std::pair<int, int> {i, j + 1}, QVector<int> {UPDOWN, LEFT})) {
                return false;
            }
            if (i < m_currentRowCount - 1 &&
                    removeAviableIfSwap(std::pair<int, int> {i, j}, std::pair<int, int> {i + 1, j}, QVector<int> {UP, LEFTRIGHT})) {
                return false;
            }

        }
    }
    return true;
}

bool GameModel::gameIsWon()
{
    for (int i {0}; i < m_currentRowCount; i++) {
        for (int j {0}; j < m_currentColumnCount; j++) {
            if (m_gameField[i][j].visible) { // we have at least one visible element
                return false;
            }
        }
    }
    return true;
}


std::pair<int, int> GameModel::get2dPosition(int index)
{
    return std::pair <int, int> {index / m_currentColumnCount, index % m_currentColumnCount};
}

int GameModel::getIndexFrom2dPosition(std::pair<int, int> position)
{
    return position.first * m_currentRowCount + position.second;
}

int GameModel::getIndexFrom2dPosition(int row, int column)
{
    return row * m_currentRowCount + column;
}

QJsonObject GameModel::readFromJsonFile(QString settingsFileName)
{
    QString configFileContents;
    QFile configFile{settingsFileName};
    QJsonDocument jsonDocument;

    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::exception();
    }

    /* Reading json doc */
    configFileContents = configFile.readAll();
    configFile.close();

    jsonDocument = QJsonDocument::fromJson(configFileContents.toUtf8());
    return jsonDocument.object();
}

std::pair<int, int> GameModel::findLastElement(std::pair<int, int> startPosition, int deltaI, int deltaJ, QString color)
{
    int i {startPosition.first};
    int j {startPosition.second};

    while (i >= 0 && j >= 0 && i < m_currentRowCount && j < m_currentColumnCount) {
        if (m_gameField[i][j].color != color || !m_gameField[i][j].visible) {
           break;
        }
        i += deltaI;
        j += deltaJ;
    }
    i-=deltaI;
    j-=deltaJ;
    return std::pair<int, int> {i, j};
}

void GameModel::swapItems(std::pair<int, int> firstElementPos, std::pair<int, int> secondElementPos)
{
    std::swap(m_gameField[firstElementPos.first][firstElementPos.second], m_gameField[secondElementPos.first][secondElementPos.second]);

    int firstIndex {getIndexFrom2dPosition(firstElementPos)};
    int secondIndex {getIndexFrom2dPosition(secondElementPos)};

    if (secondElementPos.second - firstElementPos.second != 1) {
        beginMoveRows(QModelIndex(), firstIndex, firstIndex, QModelIndex(), secondIndex);
        endMoveRows();
        beginMoveRows(QModelIndex(), secondIndex, secondIndex, QModelIndex(), firstIndex);
        endMoveRows();
    }
    else {
        beginMoveRows(QModelIndex(), firstIndex, firstIndex, QModelIndex(), secondIndex + 1);
        endMoveRows();
    }

    m_selectedItem = std::pair <int, int> {-1, -1};
    emit dataChanged(createIndex(firstIndex, 0), createIndex(secondIndex, 0), QVector<int> {});
}
