#include "diagnostictemplatemodel.h"
#include "../share/configfiles.h"

#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

DiagnosticTemplateModel::DiagnosticTemplateModel(QObject *parent) :
    rootNode(new DT_Node),
    isModified(false),
    QAbstractItemModel(parent)
{
    setupModel();
}

void DiagnosticTemplateModel::setupModel()
{
    QFile file(DIAGNOSTIC_TEMPLATE_XML);
    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(&file);
        DT_Node *current = rootNode;
        QXmlStreamReader::TokenType type;
        while ((type = reader.readNext()) != QXmlStreamReader::Invalid) {
            switch (type) {
            case QXmlStreamReader::StartDocument:
                break;
            case QXmlStreamReader::StartElement:
                if (reader.name() == "Sentences" ||
                    reader.name() == "DeptKind" ||
                    reader.name() == "PartKind" ||
                    reader.name() == "SickKind") {
                    current = new DT_Node(current);
                } else if (reader.name() == "name") {
                    current->title = reader.readElementText();
                } else if (reader.name() == "Report" &&
                           QXmlStreamReader::Characters == reader.readNext()) {
                    current->findings = reader.text().toString();
                } else if (reader.name() == "Conclusion" &&
                           QXmlStreamReader::Characters == reader.readNext()) {
                    current->conclusion = reader.text().toString();
                }
                break;
            case QXmlStreamReader::EndElement:
                if (reader.name() == "Sentences" ||
                    reader.name() == "DeptKind" ||
                    reader.name() == "PartKind" ||
                    reader.name() == "SickKind") {
                    current = current->parent;
                }
                break;
            }
        }
        file.close();
    }
}

void DiagnosticTemplateModel::xmlWriteHelpper(QXmlStreamWriter &writer, DT_Node *srcnode, int level)
{
    foreach (DT_Node *node, srcnode->childList) {
        switch (level) {
        case 0:
            writer.writeStartElement("Sentences");
            break;
        case 1:
            writer.writeStartElement("DeptKind");
            break;
        case 2:
            writer.writeStartElement("PartKind");
            break;
        case 3:
            writer.writeStartElement("SickKind");
            break;
        default:
            writer.writeStartElement("SickKind");
            break;
        }

        writer.writeTextElement("name", node->title);
        if (node->childList.size()==0) {
            writer.writeStartElement("Sentence");
            writer.writeStartElement("Report");
            writer.writeCDATA(node->findings);
            writer.writeEndElement();
            writer.writeStartElement("Conclusion");
            writer.writeCDATA(node->conclusion);
            writer.writeEndElement();
            writer.writeEndElement();
        } else {
            xmlWriteHelpper(writer, node, level+1);
        }
        writer.writeEndElement();
    }
}

bool DiagnosticTemplateModel::saveData()
{
    if (isModified) {
        QFile file(DIAGNOSTIC_TEMPLATE_XML);
        if (file.open(QIODevice::WriteOnly)) {
            QXmlStreamWriter writer(&file);
            writer.setAutoFormatting(true);
            writer.setCodec("GB2312");
            writer.writeStartDocument();
            writer.writeStartElement("root");
            xmlWriteHelpper(writer, rootNode, 0);
            writer.writeEndElement();
            writer.writeEndDocument();
            file.close();
            isModified = false;
        } else return false;
    }
    return true;
}

int DiagnosticTemplateModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) return rootNode->childList.size();
    else return static_cast<DT_Node*>(parent.internalPointer())->childList.size();
}

QModelIndex DiagnosticTemplateModel::parent(const QModelIndex &child) const
{
    if (!child.isValid()) return QModelIndex();

    DT_Node *childNode = static_cast<DT_Node*>(child.internalPointer());
    DT_Node *parentNode = childNode->parent;
    if (parentNode == rootNode) return QModelIndex();
    else return createIndex(parentNode->parent->childList.indexOf(parentNode), 0, parentNode);
}

QModelIndex DiagnosticTemplateModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent)) return QModelIndex();
    if (!parent.isValid()) return createIndex(row, column, rootNode->childList.at(row));
    DT_Node *parentNode = static_cast<DT_Node*>(parent.internalPointer());
    return createIndex(row, column, parentNode->childList.at(row));
}

QVariant DiagnosticTemplateModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || Qt::DisplayRole != role) return QVariant();
    else return static_cast<DT_Node*>(index.internalPointer())->title;
}

bool DiagnosticTemplateModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && Qt::EditRole==role) {
        DT_Node *node = static_cast<DT_Node*>(index.internalPointer());
        if (!value.toString().isEmpty()) {
            node->title = value.toString();
            dataChanged(index, index, QVector<int>()<<Qt::DisplayRole);
            isModified = true;
            return true;
        } else return false;
    }
    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags DiagnosticTemplateModel::flags(const QModelIndex &/*index*/) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

void DiagnosticTemplateModel::onDoubleClicked(const QModelIndex &index)
{
    if (index.isValid()) {
        DT_Node *node = static_cast<DT_Node*>(index.internalPointer());
        if (node->childList.size() == 0)
            emit itemSelected(node->findings, node->conclusion);
    }
}

void DiagnosticTemplateModel::onNewItemAfter(const QModelIndex &index)
{
    if (index.isValid()) {
        DT_Node *node = static_cast<DT_Node*>(index.internalPointer());
        beginInsertRows(index.parent(), index.row()+1, index.row()+1);
        DT_Node *newNode = new DT_Node;
        newNode->parent = node->parent;
        node->parent->childList.insert(index.row()+1, newNode);
        newNode->title = tr("Untitled");
        endInsertRows();
        isModified = true;
        emit editItemTitle(this->index(index.row()+1, 0, index.parent()));
    }
}

void DiagnosticTemplateModel::onNewItemBefore(const QModelIndex &index)
{
    if (index.isValid()) {
        DT_Node *node = static_cast<DT_Node*>(index.internalPointer());
        beginInsertRows(index.parent(), index.row(), index.row());
        DT_Node *newNode = new DT_Node;
        newNode->parent = node->parent;
        node->parent->childList.insert(index.row(), newNode);
        newNode->title = tr("Untitled");
        endInsertRows();
        isModified = true;
        emit editItemTitle(this->index(index.row(), 0, index.parent()));
    }
}

void DiagnosticTemplateModel::onNewSubItem(const QModelIndex &index)
{
    if (index.isValid()) {
        DT_Node *node = static_cast<DT_Node*>(index.internalPointer());
        int row = node->childList.size();
        beginInsertRows(index, row, row);
        DT_Node *newNode = new DT_Node(node);
        newNode->title = tr("Untitled");
        endInsertRows();
        isModified = true;
        emit editItemTitle(this->index(row, 0, index));
    }
}

void DiagnosticTemplateModel::onEditItem(const QModelIndex &index)
{
    if (index.isValid()) {
        DT_Node *node = static_cast<DT_Node*>(index.internalPointer());
        emit editItem(index, node->title, node->findings, node->conclusion);
    }
}

void DiagnosticTemplateModel::onApplyEdit(const QModelIndex &index, const QString &t, const QString &f, const QString &c)
{
    if (index.isValid()) {
        DT_Node *node = static_cast<DT_Node*>(index.internalPointer());
        node->title = t;
        node->findings = f;
        node->conclusion = c;
        isModified = true;
        emit dataChanged(index, index, QVector<int>()<<Qt::DisplayRole);
    }
}

void DiagnosticTemplateModel::onRemoveItem(const QModelIndex &index)
{
    if (index.isValid()) {
        beginRemoveRows(index.parent(), index.row(), index.row());
        DT_Node *node = static_cast<DT_Node*>(index.internalPointer());
        node->parent->childList.removeAt(index.row());
        delete node;
        endRemoveRows();
        isModified = true;
    }
}
