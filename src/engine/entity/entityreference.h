#ifndef ACAENGINE_ENTITYREFERENCE_H
#define ACAENGINE_ENTITYREFERENCE_H

#include <vector>
#include <typeindex>

namespace entity {
    struct EntityReference {
    public:
        [[nodiscard]] bool isExpired() const {
            return _isExpired;
        }

    protected:
        explicit EntityReference(int referenceID) : _referenceID(referenceID) {};

        void updateReferenceID(int refID) {
            this->_referenceID = refID;
        }

        [[nodiscard]] const int &getReferenceID() const {
            return _referenceID;
        }

        void setExpired() {
            _isExpired = true;
        }

        friend
        class EntityRegistry;

    private:
        int _referenceID;
        bool _isExpired = false;
    };
}

#endif //ACAENGINE_ENTITYREFERENCE_H
