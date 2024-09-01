#ifndef _DYNOBJECT_H
#define _DYNOBJECT_H

class DObject;
#include <set>

class DObject {
    std::set<DObject *> m_nodes;

   public:
    void addObject(DObject *parent);
    void setParent(DObject *parent);
    void removeObject(DObject *parent);
    DObject(DObject *parent = nullptr);
    virtual ~DObject();
};
#endif
