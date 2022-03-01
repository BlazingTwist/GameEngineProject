#ifndef ACAENGINE_ENTITYREFERENCE_H
#define ACAENGINE_ENTITYREFERENCE_H

namespace entity {
    struct EntityReference {
    public:
        /**
         * @return true, if the Entity referenced by this has been deleted | false otherwise.
         */
        [[nodiscard]] bool isExpired() const {
            return _isExpired;
        }

        /**
         * @return internal ID of this Entity, should only be used for debugging!
         */
        [[nodiscard]] int getReferenceID() const {
            return _referenceID;
        }

        bool operator==(const EntityReference &rhs) const {
            if(_isExpired || rhs._isExpired){
                return false;
            }
            return _referenceID == rhs._referenceID;
        }

        bool operator!=(const EntityReference &rhs) const {
            if(_isExpired || rhs._isExpired){
                return true;
            }
            return _referenceID != rhs._referenceID;
        }

    protected:
        explicit EntityReference(int referenceID) : _referenceID(referenceID) {};

        void updateReferenceID(int refID) {
            this->_referenceID = refID;
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
