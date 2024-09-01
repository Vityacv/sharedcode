#include "pch.h"
#include "shared/dynobject.h"

void DObject::addObject(DObject *parent) {  // add object to list
    m_nodes.insert(parent);
}

void DObject::removeObject(DObject *parent)  // remove object from list
{
    const auto &it = m_nodes.find(parent);
    if (m_nodes.end() != it) m_nodes.erase(it);
    delete parent;
}

DObject::DObject(DObject *parent) {
    if (parent) parent->addObject(this);
}

DObject::~DObject() {
    for (auto it = m_nodes.begin(); it != m_nodes.end();) {
        auto key = *it;
        it = m_nodes.erase(it);
        delete key;
    }
    //    while (m_nodes.count()) {
    //        DynObjectNode *node = m_nodes.begin();
    //        m_nodes.free(node);
    //        delete node->m_item;
    //    }
}
