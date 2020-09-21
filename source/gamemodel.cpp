#include "include/gamemodel.h"

GameModel::GameModel(QObject *parent, QString settingsFileName) :
    m_selectedItemId {-1}
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

    configFileContents = configFile.readAll();
    configFile.close();

    jsonDocument = QJsonDocument::fromJson(configFileContents.toUtf8());
    objects = jsonDocument.object();

    m_gameFieldWidth = objects["width"].toDouble();
    m_gameFieldHeight = objects["height"].toDouble();
    if (m_gameFieldWidth <= 2 || m_gameFieldHeight < 2) { //game always lost
        throw std::exception();
    }

    m_size = m_gameFieldHeight * m_gameFieldWidth;
    m_gameField.resize(m_size);

    aviableColors = objects["colors"].toArray();
    m_aviableColorsCount = aviableColors.size();
    if (m_aviableColorsCount <= 1) {
        throw std::exception();
    }

    for (int i {0}; i < m_aviableColorsCount; i++) {
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
    QVariant returnValue {};
    switch (role) {
    case ItemColor: {
        returnValue = QVariant {m_gameField[index.row()]};
        break;
    }
    case IsSelected: {
        if (index.row() == m_selectedItemId) {
            returnValue = QVariant {true};
            break;
        }
        else {
            returnValue = QVariant {false};
            break;
        }
    }
    case BallIsVisible: {
        if (m_gameField.at(index.row()) == "black") {
            returnValue = QVariant {false};
            break;
        }
        else {
            returnValue = QVariant {true};
            break;
        }
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

    beginResetModel();
    m_selectedItemId = -1;
    for (int i {0}; i < m_size; i++) {
        //if index in current row > 2 -> we are looking for coincidences
        if (i % m_gameFieldWidth >= 2) {
            do {
                generatedColorId = QRandomGenerator::global()->bounded(m_aviableColorsCount);
            }
            while (m_gameField[i - 1] == m_colors[generatedColorId] &&
                   m_gameField[i - 2] == m_colors[generatedColorId]);
            m_gameField[i] = m_colors[generatedColorId];
        }
        else {
            m_gameField[i] = m_colors[QRandomGenerator::global()->bounded(m_aviableColorsCount)];
        }
    }
    endResetModel();
}

void GameModel::selectItem(int index)
{
    int oldSelectedItemId {m_selectedItemId};

    if(m_selectedItemId != -1) {
        if(swapAviable(index) == true) {
            if(swapSelectedItem(index)) {
                oldSelectedItemId = index;
                m_selectedItemId = -1;
            }
            else {
                oldSelectedItemId = index;
                m_selectedItemId = -1;
            }
            //shake animation
        }
        else {
            m_selectedItemId = index;
        }
    }
    else {
        oldSelectedItemId = index;
        m_selectedItemId = index;
    }

    if(index < oldSelectedItemId) {
        std::swap(index,oldSelectedItemId);
    }
    emit dataChanged(createIndex(0, 0), createIndex(m_size - 1, 0), QVector<int> {IsSelected});
}

bool GameModel::swapAviable(int newPositionIndex)
{
    bool swapIsAviable {false};

    //left
    if (newPositionIndex - m_selectedItemId == 1 &&
            newPositionIndex % m_gameFieldWidth != 0)
    {
        swapIsAviable = true;
    }//right
    else if (m_selectedItemId - newPositionIndex  == 1 &&
             (newPositionIndex + 1) % m_gameFieldWidth != 0)
    {
        swapIsAviable = true;
    }//up
    else if (newPositionIndex - m_selectedItemId == m_gameFieldWidth)
    {
        swapIsAviable = true;
    }//down
    else if (m_selectedItemId - newPositionIndex == m_gameFieldWidth)
    {
        swapIsAviable = true;
    }

    if (swapIsAviable) {
        return true;
    }
    return false;
}

bool GameModel::swapSelectedItem(int newPositionIndex)
{
    bool swapIsAviable;
    int oldPositionIndex {m_selectedItemId};
    if (oldPositionIndex > newPositionIndex) {
        std::swap(oldPositionIndex,newPositionIndex);
    }

    if (newPositionIndex - oldPositionIndex == m_gameFieldWidth) {
        swapIsAviable = removeAviableIfSwapRows(oldPositionIndex, newPositionIndex);
    }
    else {
        swapIsAviable = removeAviableIfSwapColumns(oldPositionIndex, newPositionIndex);
    }
    if (!swapIsAviable) {
        return false;
    }
    std::swap(m_gameField[oldPositionIndex], m_gameField[newPositionIndex]);
    if (newPositionIndex - oldPositionIndex != 1) {
        beginMoveRows(QModelIndex(), oldPositionIndex, oldPositionIndex, QModelIndex(), newPositionIndex);
        endMoveRows();
        beginMoveRows(QModelIndex(), newPositionIndex, newPositionIndex, QModelIndex(), oldPositionIndex);
        endMoveRows();
    }
    else {
        beginMoveRows(QModelIndex(), oldPositionIndex, oldPositionIndex, QModelIndex(), newPositionIndex + 1);
        endMoveRows();
    }

    m_selectedItemId = -1;
    emit dataChanged(createIndex(oldPositionIndex, oldPositionIndex), createIndex(newPositionIndex, 0), QVector<int> {});
    removeIfPossible();
    return true;
}

bool GameModel::removeAviableIfSwapRows(int oldPositionIndex, int newPositionIndex)
{
    int i;
    int rightElement {oldPositionIndex};
    int leftElement {oldPositionIndex};
    if (oldPositionIndex % m_gameFieldWidth != 0) {
        i = oldPositionIndex - 1;
        while (i % m_gameFieldWidth >= 0 && m_gameField[i] == m_gameField[newPositionIndex]) {
            i--;
        }
        leftElement = i + 1;
    }
    if (oldPositionIndex % m_gameFieldWidth != m_gameFieldWidth - 1) {
        i = oldPositionIndex + 1;
        while (i < m_size && i % m_gameFieldWidth < m_gameFieldWidth && m_gameField[i] == m_gameField[newPositionIndex]) {
            i++;
        }
        rightElement = i - 1;
    }
    if (rightElement - leftElement < 2) {
        if (oldPositionIndex > newPositionIndex) {
            return false;
        }
        if (!removeAviableIfSwapRows(newPositionIndex, oldPositionIndex)) {
            return false;
        }
    }
    return true;
}

bool GameModel::removeAviableIfSwapColumns(int oldPositionIndex, int newPositionIndex)
{
    int i;
    if (oldPositionIndex % m_gameFieldWidth > 2) {
        i = oldPositionIndex - 1;
        while (i % m_gameFieldWidth >= 0 && m_gameField[i] == m_gameField[newPositionIndex]) {
            i--;
        }
        if (oldPositionIndex - i >= 3) {
            return true;
        }
    }
    if (newPositionIndex % m_gameFieldWidth < m_gameFieldWidth - 2) {
        i = newPositionIndex + 1;
        while (i % m_gameFieldWidth < m_gameFieldWidth && m_gameField[i] == m_gameField[oldPositionIndex]) {
            i++;
        }
        if (i - newPositionIndex >= 3) {
            return true;
        }
    }
    return false;
}

void GameModel::removeIfPossible()
{
    bool itemsRemoved {false};
    int i {0};
    int j, k;
    while (i < m_size - 2) {
        if(i % m_gameFieldWidth < m_gameFieldWidth - 2) {
            j = i;
            while (j < m_size - 1 && (j % m_gameFieldWidth) < m_gameFieldWidth - 1 && m_gameField[i] == m_gameField[j + 1]) {
                j++;
            }
            if (j - i >= 2) {
                for (k = i; k <= j; k++) {
                    m_gameField[k] = "black";
                }
                itemsRemoved = true;
                emit dataChanged(createIndex(0, 0), createIndex(m_size - 1, 0), QVector<int> {BallIsVisible});
            }
            i = j + 1;
        }
        else {
            i ++;
        }
    }
    if (itemsRemoved) {
        moveToFloor();
    }
}

void GameModel::moveToFloor()
{
    bool itemsMoved {false};
    int i {0};
    while (i < m_size) {
        if(m_gameField[i] != "black" && i + m_gameFieldWidth < m_size && m_gameField[i + m_gameFieldWidth] == "black") {
            std::swap(m_gameField[i], m_gameField[i + m_gameFieldWidth]);
            beginMoveRows(QModelIndex(), i, i, QModelIndex(), i + m_gameFieldWidth);
            endMoveRows();
            beginMoveRows(QModelIndex(), i + m_gameFieldWidth, i + m_gameFieldWidth, QModelIndex(), i);
            endMoveRows();
            emit dataChanged(createIndex(i, 0), createIndex(i + m_gameFieldWidth, 0), QVector<int> {});
            itemsMoved = true;
        }
        i++;
    }
    if (itemsMoved) {
        removeIfPossible();
    }
}
