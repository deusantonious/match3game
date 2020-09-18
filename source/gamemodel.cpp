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
}
return returnValue;
}

QHash<int, QByteArray> GameModel::roleNames() const
{
    QHash <int, QByteArray> roles;

    roles[ItemColor] = "itemColor";
    roles[IsSelected] = "isSelected";

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
        if (i % m_gameFieldWidth > 2) {
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
    if(m_selectedItemId == -1 || swapItems(index) == false) {

        int oldSelectedItemId {-1};
        if (m_selectedItemId == -1) {
            oldSelectedItemId = index;
        }
        else {
            oldSelectedItemId = m_selectedItemId;
        }

        m_selectedItemId = index;

        if(oldSelectedItemId > m_selectedItemId) {
            emit dataChanged(createIndex(m_selectedItemId, 0), createIndex(oldSelectedItemId, 0), QVector<int> {IsSelected});
        }
        else {
            emit dataChanged(createIndex(oldSelectedItemId, 0), createIndex(m_selectedItemId, 0), QVector<int> {IsSelected});
        }
    }
}

bool GameModel::swapItems(int newPositionIndex)
{
    bool swapIsAviable {false};
    int moveExtender {0};

    //left
    if (newPositionIndex - m_selectedItemId == 1 &&
            newPositionIndex % m_gameFieldWidth != 0)
    {
        moveExtender = 1;
        swapIsAviable = true;
    }//right
    else if (m_selectedItemId - newPositionIndex  == 1 &&
             (newPositionIndex + 1) % m_gameFieldWidth != 0)
    {
        swapIsAviable = true;
    }//up
    else if (newPositionIndex - m_selectedItemId == m_gameFieldWidth)
    {
        moveExtender = 1;
        swapIsAviable = true;
    }//down
    else if (m_selectedItemId - newPositionIndex == m_gameFieldWidth)
    {
        swapIsAviable = true;
    }

    if (swapIsAviable)
    {
        int oldPositionIndex {m_selectedItemId};

        if (oldPositionIndex > newPositionIndex) {
            std::swap(oldPositionIndex,newPositionIndex);
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

        return true;

    }
    return false;
}
