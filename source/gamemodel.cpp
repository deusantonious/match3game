#include "include/gamemodel.h"

GameModel::GameModel(QObject *parent, QString settingsFileName)
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
    if (role == ItemColor) {
        return QVariant::fromValue(m_gameField[index.row()]);
    }

    return {};
}

QHash<int, QByteArray> GameModel::roleNames() const
{
    QHash <int, QByteArray> roles;

    roles[ItemColor] = "itemColor";

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
