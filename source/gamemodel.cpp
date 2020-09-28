#include "include/gamemodel.h"

GameModel::GameModel(QObject *parent, QString settingsFileName) :
    m_selectedItem {-1,-1}
{
    Q_UNUSED(parent)

    QString configFileContents;
    QFile configFile(settingsFileName);
    QJsonDocument jsonDocument;
    QJsonObject objects;
    QJsonArray aviableColors;

    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        throw std::exception();
    }
    /* Reading json doc */
    configFileContents = configFile.readAll();
    configFile.close();

    jsonDocument = QJsonDocument::fromJson(configFileContents.toUtf8());
    objects = jsonDocument.object();

    m_gameFieldWidth = objects["width"].toDouble();
    m_gameFieldHeight = objects["height"].toDouble();
    if (m_gameFieldWidth <= 2 || m_gameFieldHeight < 2) { //game always lost
        throw std::exception();
    }

    /* Creating game field array */
    m_size = m_gameFieldHeight * m_gameFieldWidth;
    m_gameField.resize(m_gameFieldHeight);
    for (int i{0}; i < m_gameFieldHeight; i++) {
        m_gameField[i].resize(m_gameFieldWidth);
    }

    /* Creating aviable colors array */
    aviableColors = objects["colors"].toArray();
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
    m_selectedItem = std::pair <int,int> {-1,-1};
    for (int i {0}; i < m_gameFieldHeight; i++) {

        m_gameField[i][0].color = m_colors[QRandomGenerator::global()->bounded(aviableColorsCount)];
        m_gameField[i][1].color = m_colors[QRandomGenerator::global()->bounded(aviableColorsCount)];
        m_gameField[i][0].visible = true;
        m_gameField[i][1].visible = true;

        for (int j {2}; j< m_gameFieldWidth; j++)  {
            do {
                generatedColorId = QRandomGenerator::global()->bounded(aviableColorsCount);
            }
            while (m_gameField[i][j - 1].color == m_colors[generatedColorId] &&
                   m_gameField[i][j - 2].color == m_colors[generatedColorId]);

            m_gameField[i][j] = m_colors[generatedColorId];
            m_gameField[i][j].visible = true;
        }
    }
    endResetModel();
}

void GameModel::selectItem(int index)
{
    auto position = get2dPosition(index);
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
            swapIsAviable = removeAviableIfSwapRows(oldPosition, newPosition);
        }
        else { // if we need to swap columns
            swapIsAviable = removeAviableIfSwapColumns(oldPosition, newPosition);
        }

        if (swapIsAviable) {
            std::swap(m_gameField[oldPosition.first][oldPosition.second], m_gameField[newPosition.first][newPosition.second]);
            if (newPosition.second - oldPosition.second != 1) {
                beginMoveRows(QModelIndex(), getIndexFrom2dPosition(oldPosition), getIndexFrom2dPosition(oldPosition), QModelIndex(), getIndexFrom2dPosition(newPosition));
                endMoveRows();
                beginMoveRows(QModelIndex(), getIndexFrom2dPosition(newPosition), getIndexFrom2dPosition(newPosition), QModelIndex(), getIndexFrom2dPosition(oldPosition));
                endMoveRows();
            }
            else {
                beginMoveRows(QModelIndex(), getIndexFrom2dPosition(oldPosition), getIndexFrom2dPosition(oldPosition), QModelIndex(), getIndexFrom2dPosition(newPosition) + 1);
                endMoveRows();
            }

            m_selectedItem = std::pair <int, int> {-1,-1};
            emit dataChanged(createIndex(getIndexFrom2dPosition(oldPosition), getIndexFrom2dPosition(oldPosition)), createIndex(getIndexFrom2dPosition(newPosition), 0), QVector<int> {});
            return true;
        }
    }
    return false;
}

bool GameModel::makeAllCoincidenceInvisible()
{
    int j, k;
    bool itemsDeleted {false};
    for (int i {0}; i < m_gameFieldHeight; i++) {
        j = 0;
        while (j < m_gameFieldWidth) {
            if(!m_gameField[i][j].visible) {
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

/* at start we check oldPosition row for coincidences */
/* if we dont have coincidences in oldPosition row we are calling same function with swapped parameters
   and looking for coincidences in newPosition row */
bool GameModel::removeAviableIfSwapRows(std::pair<int, int> oldPosition, std::pair<int, int> newPosition)
{
    int i;
    int rightElement {oldPosition.second};
    int leftElement {oldPosition.second};
    if (oldPosition.second != 0) {
        i = oldPosition.second - 1;
        while (i >= 0 &&
               m_gameField[oldPosition.first][i].color == m_gameField[newPosition.first][newPosition.second].color &&
               m_gameField[oldPosition.first][i].visible) {
            i--;
        }
        leftElement = i + 1;
    }
    if (newPosition.second != m_gameFieldWidth - 1) {
        i = oldPosition.second + 1;
        while (i < m_gameFieldWidth &&
               m_gameField[oldPosition.first][i].color == m_gameField[newPosition.first][newPosition.second].color &&
               m_gameField[oldPosition.first][i].visible) {
            i++;
        }
        rightElement = i - 1;
    }
    if (rightElement - leftElement < 2) {
        if (oldPosition.first > newPosition.first) {
            return false;
        }
        if (!removeAviableIfSwapRows(newPosition, oldPosition)) {
            return false;
        }
    }
    return true;
}

bool GameModel::removeAviableIfSwapColumns(std::pair<int, int> oldPosition, std::pair<int, int> newPosition)
{
    int i;

    /* <<--- */
    if (oldPosition.second >= 2) {
        i = oldPosition.second - 1;
        while (i >= 0 &&
               m_gameField[oldPosition.first][i].color == m_gameField[oldPosition.first][newPosition.second].color &&
               m_gameField[oldPosition.first][i].visible) {
            i--;
        }
        if (oldPosition.second - i >= 3) {
            return true;
        }
    }
    /* --->>> */
    if (newPosition.second < m_gameFieldWidth - 2) {
        i = newPosition.second + 1;
        while (i < m_gameFieldWidth &&
               m_gameField[newPosition.first][i].color == m_gameField[newPosition.first][oldPosition.second].color &&
               m_gameField[oldPosition.first][i].visible) {
            i++;
        }
        if (i - newPosition.second >= 3) {
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
            std::swap(m_gameField[k][j],m_gameField[i][j]);
            beginMoveRows(QModelIndex(), getIndexFrom2dPosition(k, j), getIndexFrom2dPosition(k, j), QModelIndex(), getIndexFrom2dPosition(i, j));
            endMoveRows();
            beginMoveRows(QModelIndex(), getIndexFrom2dPosition(i, j), getIndexFrom2dPosition(i, j), QModelIndex(), getIndexFrom2dPosition(k, j));
            endMoveRows();
            emit dataChanged(createIndex(getIndexFrom2dPosition(0, 0), 0), createIndex(getIndexFrom2dPosition(m_gameFieldHeight - 1, m_gameFieldWidth - 1), 0), QVector<int> {});
            itemsMoved = true;
        }
    }
    if(itemsMoved) {
        return true;
    }
    return false;
}

bool GameModel::gameIsLost()
{
    //trying to swap each item with lower item
    for (int i {0}; i < m_gameFieldHeight - 1; i++) {
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
