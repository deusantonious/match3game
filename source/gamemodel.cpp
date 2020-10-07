#include "include/gamemodel.h"

GameModel::GameModel(QObject *parent, QString settingsFileName) :
    m_selectedItem {-1,-1}
{
    Q_UNUSED(parent)

    QJsonObject objects {readFromJsonFile(settingsFileName)};

    m_gameFieldWidth = objects["width"].toInt();
    m_gameFieldHeight = objects["height"].toInt();
    if (m_gameFieldWidth <= 2 || m_gameFieldHeight <= 2) { //game always lost
        throw std::exception();
    }

    /* Creating game field array */
    m_size = m_gameFieldHeight * m_gameFieldWidth;
    m_gameField.resize(m_gameFieldHeight);
    for (int i{0}; i < m_gameFieldHeight; i++) {
        m_gameField[i].resize(m_gameFieldWidth);
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

    std::pair <int, int> index2d {index.row() / m_gameFieldWidth, index.row() % m_gameFieldWidth};
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

int GameModel::getGameFieldWidth() const
{
    return m_gameFieldWidth;
}

int GameModel::getGameFieldHeight() const
{
    return m_gameFieldHeight;
}

void GameModel::gameFieldReset()
{
    int generatedColorId {0};
    int aviableColorsCount {m_colors.size()};

    beginResetModel();
    m_selectedItem = std::pair <int,int> {-1, -1};
    for (int i {0}; i < m_gameFieldHeight; i++) {
        for (int j {0}; j < m_gameFieldWidth; j++)  {
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
    int i, j, k;
    bool itemsDeleted {false};
    for (i = 0; i < m_gameFieldHeight; i++) {
        j = 0;
        while (j < m_gameFieldWidth) {
            if (!m_gameField[i][j].visible) {
                j++;
                continue;
            }
            k = j + 1;
            while (k < m_gameFieldWidth && m_gameField[i][k].visible && m_gameField[i][k].color == m_gameField[i][j].color) {
                k++;
            }
            if (k - j >= 3) {
                for (int p {j}; p < k; p++) {
                    m_gameField[i][p].visible = false;
                }
                emit dataChanged(createIndex(getIndexFrom2dPosition(std::pair <int, int> {i, j}), 0), createIndex(getIndexFrom2dPosition(std::pair <int, int> {i, k - 1}), 0), QVector<int> {});
                itemsDeleted = true;
            }
            j++;
        }
    }
    for (j = 0; j < m_gameFieldWidth; j++) {
        i = 0;
        while (i < m_gameFieldHeight) {
            if (!m_gameField[i][j].visible) {
                i++;
                continue;
            }
            k = i + 1;
            while (k < m_gameFieldHeight && m_gameField[k][j].visible && m_gameField[k][j].color == m_gameField[i][j].color) {
                k++;
            }
            if (k - i >= 3) {
                for (int p {i}; p < k; p++) {
                    m_gameField[p][j].visible = false;
                }
                emit dataChanged(createIndex(getIndexFrom2dPosition(std::pair <int, int> {i, j}), 0), createIndex(getIndexFrom2dPosition(std::pair <int, int> {k -1 , j}), 0), QVector<int> {});
                itemsDeleted = true;
            }
            i++;
        }
    }
    if (itemsDeleted) {
        return true;
    }
    return false;
}

int GameModel::getSelectedItemId() const
{
    return m_selectedItem.first != -1 ?
                m_selectedItem.first * m_gameFieldWidth + m_selectedItem.second :
                -1;
}

void GameModel::deleteAllEmptyRowsAndColumns()
{
    int i;
    for (int j {0}; j < m_gameFieldWidth; j++) {
        i = 0;
        while (i < m_gameFieldHeight && !m_gameField[i][j].visible) {
            i++;
        }
        if (i == m_gameFieldHeight) {
            for (int k {0}; k < m_gameFieldHeight; k++) {
                for (int p {j}; p < m_gameFieldWidth - 1; p++) {
                    swapItems(std::pair<int, int> {k, p}, std::pair<int, int> {k, p + 1});
                }
            }
        }
    }
}

bool GameModel::removeAviableIfSwap(std::pair<int, int> oldPosition, std::pair<int, int> sourceElement, QVector<int> directions)
{
    if (directions.indexOf(UP) != -1 && oldPosition.first >= 2) {
        std::pair<int, int> upperLast = findLastElement(std::pair<int, int> {oldPosition.first - 1, oldPosition.second}, -1, 0, m_gameField[sourceElement.first][sourceElement.second].color);
        if (oldPosition.first - upperLast.first >= 2) {
            return true;
        }
    }
    if (directions.indexOf(DOWN) != -1 && oldPosition.first <= m_gameFieldHeight - 2) {
        std::pair<int, int> downLast = findLastElement(std::pair<int, int> {oldPosition.first + 1, oldPosition.second}, 1, 0, m_gameField[sourceElement.first][sourceElement.second].color);
        if (downLast.first - oldPosition.first >= 2) {
            return true;
        }
    }
    if (directions.indexOf(UPDOWN) != -1) {
        int topElement {oldPosition.first};
        int downElement {oldPosition.first};

        if (oldPosition.first > 0)  {
            topElement = findLastElement(std::pair<int, int> {oldPosition.first - 1, oldPosition.second}, -1, 0, m_gameField[sourceElement.first][sourceElement.second].color).first;
        }
        if (oldPosition.second < m_gameFieldHeight - 1) {
            downElement = findLastElement(std::pair<int, int> {oldPosition.first + 1, oldPosition.second}, 1, 0, m_gameField[sourceElement.first][sourceElement.second].color).first;
        }
        if (downElement - topElement >= 2) {
            return true;
        }
    }
    if (directions.indexOf(LEFT) != -1 && oldPosition.second >= 2) {
        std::pair<int, int> leftLast = findLastElement(std::pair<int, int> {oldPosition.first, oldPosition.second - 1}, 0, -1, m_gameField[sourceElement.first][sourceElement.second].color);
        if (oldPosition.second - leftLast.second >= 2) {
            return true;
        }
    }
    if (directions.indexOf(RIGHT) != -1 && oldPosition.second <= m_gameFieldWidth - 2) {
        std::pair<int, int> rightLast = findLastElement(std::pair<int, int> {oldPosition.first, oldPosition.second + 1}, 0, 1, m_gameField[sourceElement.first][sourceElement.second].color);
        if (rightLast.second - oldPosition.second >= 2) {
            return true;
        }
    }
    if (directions.indexOf(LEFTRIGHT) != -1) {
        int leftElement {oldPosition.second};
        int rightElement {oldPosition.second};
        if (oldPosition.second > 0)  {
            leftElement = findLastElement(std::pair<int, int> {oldPosition.first, oldPosition.second - 1}, 0, -1, m_gameField[sourceElement.first][sourceElement.second].color).second;
        }
        if (oldPosition.second < m_gameFieldWidth - 1) {
            rightElement = findLastElement(std::pair<int, int> {oldPosition.first, oldPosition.second + 1}, 0, 1, m_gameField[sourceElement.first][sourceElement.second].color).second;
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
    for (int j {0}; j < m_gameFieldWidth; j++) {
        for (int i {m_gameFieldHeight - 1}; i > 0; i--) {
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
    //trying to swap each item with lower item
    /*for (int i {0}; i < m_gameFieldHeight - 1; i++) {
        for (int j {0}; j < m_gameFieldWidth; j++) {
            if (m_gameField[i][j].visible &&
                    m_gameField[i + 1][j].visible &&
                    removeAviableIfSwapRows(std::pair <int, int> {i, j}, std::pair <int, int> {i + 1,j})) {
                return false;
            }
        }
    }
    //trying to swap each item with upper item
    for (int i {1}; i < m_gameFieldHeight; i++) {
        for (int j {0}; j < m_gameFieldWidth; j++) {
            if (m_gameField[i][j].visible &&
                    m_gameField[i - 1][j].visible &&
                    removeAviableIfSwapRows(std::pair <int, int> {i - 1, j}, std::pair <int, int> {i,j})) {
                return false;
            }
        }
    }
    //trying to swap each item with left item
    for (int i {0}; i < m_gameFieldHeight; i++) {
        for (int j {0}; j < m_gameFieldWidth - 1; j++) {
            if (m_gameField[i][j].visible &&
                    m_gameField[i][j + 1].visible &&
                    removeAviableIfSwapColumns(std::pair <int, int> {i, j}, std::pair <int, int> {i,j + 1})) {
                return false;
            }
        }
    }
    //trying to swap each item with right item
    for (int i {0}; i < m_gameFieldHeight; i++) {
        for (int j {1}; j < m_gameFieldWidth; j++) {
            if (m_gameField[i][j].visible &&
                    m_gameField[i][j - 1].visible &&
                    removeAviableIfSwapColumns(std::pair <int, int> {i, j - 1}, std::pair <int, int> {i,j})) {
                return false;
            }
        }
    }
    return true;
    */
    return false;
}

bool GameModel::gameIsWon()
{
    for (int i {0}; i < m_gameFieldHeight; i++) {
        for (int j {0}; j < m_gameFieldWidth; j++) {
            if (m_gameField[i][j].visible) { // we have at least one visible element
                return false;
            }
        }
    }
    return false;
}


std::pair<int, int> GameModel::get2dPosition(int index)
{
    return std::pair <int, int> {index / m_gameFieldWidth, index % m_gameFieldWidth};
}

int GameModel::getIndexFrom2dPosition(std::pair<int, int> position)
{
    return position.first * m_gameFieldHeight + position.second;
}

int GameModel::getIndexFrom2dPosition(int row, int column)
{
    return row * m_gameFieldHeight + column;
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
    //1 0 -1
    int i {startPosition.first};
    int j {startPosition.second};

    while(i >= 0 && j >= 0 && i < m_gameFieldHeight && j < m_gameFieldWidth) {
        if (m_gameField[i][j].color == color && m_gameField[i][j].visible) {
            i += deltaI;
            j += deltaJ;
        }
        else {
            break;
        }
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
