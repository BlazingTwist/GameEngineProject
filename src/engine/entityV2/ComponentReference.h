#ifndef ACAENGINE_COMPONENTREFERENCE_v2_H
#define ACAENGINE_COMPONENTREFERENCE_v2_H

namespace entityV2 {
    struct ComponentReference {
    public:
        [[nodiscard]] bool isExpired() const {
            return _isExpired;
        }
        
        [[nodiscard]] bool isPresent() const {
            return !_isExpired && _componentPtr != nullptr;
        }

        template<typename T_component>
        T_component *getComponent() {
            if (_isExpired) {
                return nullptr;
            }
            return static_cast<T_component *>(_componentPtr);
        }

        [[nodiscard]] unsigned int getEntityId() const {
            return _entityID;
        }

        [[nodiscard]] unsigned int getComponentId() const {
            return _componentID;
        }

    private:
        explicit ComponentReference(unsigned int entityID, unsigned int componentID, void *componentPtr) :
                _entityID(entityID),
                _componentID(componentID),
                _componentPtr(componentPtr) {}
        
        unsigned int _entityID;
        unsigned int _componentID;
        bool _isExpired = false;
        void *_componentPtr = nullptr;

        friend class ComponentRegistry;
        friend class EntityRegistry;
    };
}

#endif //ACAENGINE_COMPONENTREFERENCE_v2_H